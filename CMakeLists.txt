cmake_minimum_required(VERSION 3.16)

project(sfSnakePro)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# set(BUILD_SHARED_LIBS OFF)
# add_subdirectory(thirdparty/SFML)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(main ${SOURCES})
target_include_directories(main PUBLIC src D:/soft/SFML-2.6.1/include)
target_link_libraries(main PRIVATE D:/soft/SFML-2.6.1/lib sfml-system sfml-window sfml-graphics sfml-audio)
set_property(TARGET main PROPERTY
  CXX_STANDARD          17
  CXX_STANDARD_REQUIRED ON)

file(COPY "assets" DESTINATION "${CMAKE_BINARY_DIR}/${NAME}")

