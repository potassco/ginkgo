add_subdirectory(ginkgo-produce)
add_subdirectory(ginkgo-consume)
add_subdirectory(ginkgo-analyze)

if(BUILD_GTK3_GUI_TOOLS)
	add_subdirectory(ginkgo-evaluate)
endif(BUILD_GTK3_GUI_TOOLS)
