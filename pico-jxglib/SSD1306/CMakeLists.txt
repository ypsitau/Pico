add_library(pico_jxglib_SSD1306 INTERFACE)

target_include_directories(pico_jxglib_SSD1306 INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(pico_jxglib_SSD1306 INTERFACE
	hardware_i2c
	pico_jxglib_Common
	pico_jxglib_Font)

target_sources(pico_jxglib_SSD1306 INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/src/SSD1306.cpp)
