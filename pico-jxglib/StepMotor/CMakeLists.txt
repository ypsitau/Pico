add_library(pico_jxglib_StepMotor INTERFACE)

pico_generate_pio_header(pico_jxglib_StepMotor
	${CMAKE_CURRENT_LIST_DIR}/src/RotateOut4Bits.pio)

target_include_directories(pico_jxglib_StepMotor INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(pico_jxglib_StepMotor INTERFACE
	hardware_pio)

target_sources(pico_jxglib_StepMotor INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/src/StepMotor.cpp)
