cmake_minimum_required(VERSION 2.8)

project(AllocatorPoolTest)

set(${PROJECT_NAME}_SOURCES
  main.cpp
  )

set(${PROJECT_NAME}_HEADERS
  AllocatorPool.h
  )

set(${PROJECT_NAME}_DEPS
  )

set(all_files
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
  )

add_executable(${PROJECT_NAME} ${all_files})

target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_DEPS})
