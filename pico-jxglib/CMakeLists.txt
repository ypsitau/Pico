cmake_minimum_required(VERSION 3.12)

project(pico_jxglib C CXX ASM)

add_subdirectory(Button)
add_subdirectory(Common)
add_subdirectory(Font)
add_subdirectory(SSD1306)
add_subdirectory(StepMotor)

add_library(pico_jxglib INTERFACE)

target_link_libraries(pico_jxglib INTERFACE
	pico_jxglib_Button
	pico_jxglib_Common
	pico_jxglib_Font
	pico_jxglib_SSD1306
	pico_jxglib_StepMotor)
