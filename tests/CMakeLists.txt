set(PROJECT_NAME conversion_test)

find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h" "*.rc" "*.def")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${CLIENT_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "common/tests")
target_link_libraries(${PROJECT_NAME}
	lib_conversion
	${GTEST_BOTH_LIBRARIES}
	${GMOCK_BOTH_LIBRARIES}
)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
