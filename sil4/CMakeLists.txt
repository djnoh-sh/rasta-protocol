cmake_minimum_required(VERSION 3.12)

project(rsrx_rasta_reimplementation
	LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

add_library(rsrx_state_machine STATIC
	src/rsrx_state_machine.c)

target_include_directories(rsrx_state_machine PUBLIC include)

add_executable(rsrx_state_machine_test
	tests/unit/test_rsrx_state_machine.c)

target_link_libraries(rsrx_state_machine_test rsrx_state_machine)

