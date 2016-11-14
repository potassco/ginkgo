set(target ginkgo-evaluate)

file(GLOB core_sources "*.cpp")
file(GLOB core_headers "*.h")

include_directories(
	${Boost_INCLUDE_DIRS}
	${Clingo_INCLUDE_DIRS}
	${GTKmm_INCLUDE_DIRS}
	${JSONCPP_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/include
)

set(sources
	${core_sources}
	${core_headers}
)

set(libraries
	${Boost_LIBRARIES}
	${GTKmm_LIBRARIES}
	${JSONCPP_LIBRARY}
    ginkgo
)

add_executable(${target} ${sources})
target_link_libraries(${target} ${libraries})
