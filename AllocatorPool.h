#ifndef ALLOCATORPOOL_H_
#define ALLOCATORPOOL_H_

#include <stdexcept>
#include <cstdlib>
#include <cstring>

#ifndef STRINGIFY
# define STRINGIFY(x) #x
#endif
#ifndef XSTRINGIFY
# define XSTRINGIFY(x) STRINGIFY(x)
#endif

// ALLOC_POOL_MAX_DELETE_SIZE set the maximum size of the delete stack.
// ie: #define ALLOC_POOL_MAX_DELETE_SIZE 1024
#define __ALLOC_POOL_MAX_DELETE_SIZE_DEFAULT 1024
#ifndef ALLOC_POOL_MAX_DELETE_SIZE
  #define ALLOC_POOL_MAX_DELETE_SIZE __ALLOC_POOL_MAX_DELETE_SIZE_DEFAULT
#endif
#define __ALLOC_POOL_MAX_DELETE_SIZE_STR XSTRINGIFY(ALLOC_POOL_MAX_DELETE_SIZE)


#define ALLOC_POOL_THROW 0
#define ALLOC_POOL_FREE 1
// ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR will tell if the AllocatorPool needs
// to throw when you delete more data than the fixed limit ALLOC_POOL_MAX_DELETE_SIZE.
// ALLOC_POOL_FREE = simply free the data as a classe delete call
// ALLOC_POOL_THROW = throw an std::length_error
// Default behaviour is ALLOC_POOL_FREE.
// ie: #define ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR ALLOC_POOL_FREE


// ALLOC_POOL_IMPLEMENT will implement the allocation pool in the given class.
// Don't forget to put it inside the class declaration, not outside.
// ie:
/*
  class Dummy
  {
    ALLOC_POOL_IMPLEMENT(MyClass)

    public:
      Dummy(void)
      {
        DoSomething();
      }

    ...
  };
*/
#define ALLOC_POOL_IMPLEMENT(classType)                         \
  public:                                                       \
  void * operator new(std::size_t size) throw(std::bad_alloc)   \
  {                                                             \
    return __getPool().New(size);                               \
  }                                                             \
                                                                \
  void operator delete(void * ptr)                              \
  {                                                             \
    __getPool().Delete(ptr);                                    \
  }                                                             \
                                                                \
private:                                                        \
 static AllocatorPool<Dummy> &__getPool(void)                   \
 {                                                              \
   static AllocatorPool<Dummy> pool;                            \
                                                                \
   return pool;                                                 \
 }                                                              \


template <typename T>
class AllocatorPool
{
protected:
  struct AllocWrapper
  {
    AllocWrapper * next;
    AllocWrapper * prev;
    char object[sizeof(T)]; // to avoid calling T's ctor
  };

public:
  AllocatorPool(void)
  {
    _freedCount = 0;
    _linkBegin = NULL;
    _linkLast = NULL;
  }

  ~AllocatorPool(void)
  {
    for (int i = 0; i < _freedCount; i++)
      {
        release(_freed[i]);
      }

    AllocWrapper * node = _linkBegin;
    while (node != NULL)
      {
        AllocWrapper * next = node->next;
        
        std::cout << "freed" << std::endl;
        release(node->object);

        node = next;
      }
  }

  void * New(std::size_t size) throw(std::bad_alloc)
  {
    void * ptr = NULL;

    if (_freedCount > 0)
      {
        _freedCount--;
        ptr = _freed[_freedCount];
      }
    else
      {
        ptr = allocate(size);
      }

    if (ptr != NULL)
      {
        AllocWrapper * wrap = getWrapper(ptr);
        if (_linkBegin == NULL)
          {
            _linkBegin = wrap;
            _linkLast = wrap;
            wrap->next = NULL;
            wrap->prev = NULL;
          }
        else
          {
            _linkLast->next = wrap;

            wrap->next = NULL;
            wrap->prev = _linkLast;
            _linkLast = wrap;
          }
      }

    return ptr;
  }

  void Delete(void * ptr) throw(std::length_error)
  {
#if defined(ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR) && ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR == ALLOC_POOL_THROW
    if (_freedCount + 1 > ALLOC_POOL_MAX_DELETE_SIZE)
      {
        static const std::length_error toofar("Maximum number of Delete() exceeded. ALLOC_POOL_MAX_DELETE_SIZE_STR == "
                                              __ALLOC_POOL_MAX_DELETE_SIZE_STR);
        throw toofar;
      }
#endif
    AllocWrapper * wrap = getWrapper(ptr);
    
    if (wrap->prev != NULL)
      {
        wrap->prev->next = wrap->next;
      }
    if (wrap->next != NULL)
      {
        wrap->next->prev = wrap->prev;
      }

    if (_linkBegin == wrap)
      {
        _linkBegin = NULL;
        _linkLast = NULL;
      }

#if !defined(ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR) || ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR == ALLOC_POOL_FREE
    if (_freedCount + 1 > ALLOC_POOL_MAX_DELETE_SIZE)
      {
        release(ptr);
        return;
      }
#endif

    _freed[_freedCount] = ptr;
    _freedCount++;
  }

protected:
  void * allocate(std::size_t size) throw(std::bad_alloc)
  {
    AllocWrapper * wrap = static_cast<AllocWrapper *>(std::malloc(sizeof(AllocWrapper)));
    if (wrap == NULL)
      {
        static const std::bad_alloc nomem;
        throw nomem;
      }

    return reinterpret_cast<T *>(&(wrap->object));
  }

  void release(void * ptr)
  {
    std::free(getWrapper(ptr));
  }

  AllocWrapper * getWrapper(void * ptr)
  {
    return reinterpret_cast<AllocWrapper * >(static_cast<char *>(ptr) - (sizeof(AllocWrapper) - sizeof(T)));
  }

protected:
  void *_freed[ALLOC_POOL_MAX_DELETE_SIZE];
  std::size_t _freedCount;
  AllocWrapper * _linkBegin;
  AllocWrapper * _linkLast;
};

#endif
