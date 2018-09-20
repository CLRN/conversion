set(PROJECT_NAME lib_conversion)

cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE SOURCES "include/*.hpp" "src/*.cpp")

set(BOOST_COMPONENTS locale date_time)
find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
set_target_properties(${PROJECT_NAME}
                      PROPERTIES FOLDER "common"
                      LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

if (WITH_TESTS)
    add_subdirectory(tests)
endif()
