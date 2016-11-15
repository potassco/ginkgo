set(target ginkgo-analyze)

file(GLOB core_sources "*.cpp")
file(GLOB core_headers "*.h")

set(sources
	${core_sources}
	${core_headers}
)

add_executable(${target} ${sources})
target_link_libraries(${target} ginkgo)
