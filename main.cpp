#include <iostream>
#include <ctime>
#include <stdexcept>

//#define ALLOC_POOL_MAX_DELETE_SIZE 1024
//#define ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR ALLOC_POOL_FREE
//#define ALLOC_POOL_EXCEEDING_DELETE_BEHAVIOUR ALLOC_POOL_THROW
#include "AllocatorPool.h"


class Dummy : public std::string
{
  ALLOC_POOL_IMPLEMENT(Dummy)
};


class DummyNoPool : public std::string
{
};


void doBench(void)
{
  std::size_t loopCount = 1024 * 1024;
  clock_t begin_time;

  for (int i = 0; i < loopCount; i++); // simple setting the bench hot as the first loops seems to be slower

  begin_time = std::clock();
  for (int i = 0; i < loopCount; i++)
    {
      Dummy * d;
      d = new Dummy();
      d->append("Test");
      delete d;

      Dummy * d1;
      d1 = new Dummy();
      d1->append("Test");

      Dummy * d2;
      d2 = new Dummy();
      d2->append("Test");
      delete d2;

      delete d1;
    }
  double pool_time = double(std::clock() - begin_time) / CLOCKS_PER_SEC;

  begin_time = std::clock();
  for (int i = 0; i < loopCount; i++)
    {
      DummyNoPool * d;
      d = new DummyNoPool();
      d->append("Test");
      delete d;

      DummyNoPool * d1;
      d1 = new DummyNoPool();
      d1->append("Test");

      DummyNoPool * d2;
      d2 = new DummyNoPool();
      d2->append("Test");
      delete d2;

      delete d1;
    }
  double no_pool_time = double(std::clock() - begin_time) / CLOCKS_PER_SEC;

  std::cout << "Pool : " << pool_time << std::endl
            << "No pool : " << no_pool_time << std::endl;
}

void testExceedingDeletes(void)
{
  try
    {
      Dummy * tab[ALLOC_POOL_MAX_DELETE_SIZE + 1];
      for (int i = 0; i < ALLOC_POOL_MAX_DELETE_SIZE + 1; i++)
        {
          tab[i] = new Dummy();
        }

      for (int i = 0; i < ALLOC_POOL_MAX_DELETE_SIZE + 1; i++)
        {
          delete tab[i];
        }
    }
  catch (std::length_error le)
    {
      std::cout << "Succesfully throwed exception std::length_error : " << le.what() << std::endl;
    }
}

int main(void)
{
  Dummy d; // not interaction with the pool. May need to test this.

  doBench();
  doBench();
  doBench();

  testExceedingDeletes();

  return 0;
}
