CMAKE_MINIMUM_REQUIRED (VERSION 3.0.0)
PROJECT(synthetic)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../CMake")

include(cotire)

aux_source_directory(. SRC_LIST)

include_directories(AFTER ./)
include_directories(AFTER ./../include)


file(GLOB_RECURSE HEADERS "./*.h")
source_group("Header Files" FILES ${HEADERS})

add_executable (${PROJECT_NAME} ${SRC_LIST} ${HEADERS}
   main.cpp   
)

set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.62 COMPONENTS program_options serialization REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "./stdafx.h")

cotire(${PROJECT_NAME})