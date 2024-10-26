add_library(pico_jxglib_Button INTERFACE)

target_include_directories(pico_jxglib_Button INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(pico_jxglib_Button INTERFACE
	pico_jxglib_Common)

target_sources(pico_jxglib_Button INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/src/Button.cpp)
