set(target tests)

file(GLOB core_sources "*.cpp")

find_package(GTest)

INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${JSONCPP_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/include
)

set(libraries
	${Boost_LIBRARIES}
	${GTEST_BOTH_LIBRARIES}
	ginkgo
)

add_executable(${target} ${core_sources})
target_link_libraries(${target} ${libraries})
GTEST_ADD_TESTS(tests "" ${core_sources})
