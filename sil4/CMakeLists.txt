cmake_minimum_required(VERSION 3.12)

project(sil4_rasta_reimplementation
	LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

add_library(sil4_state_machine STATIC
	src/sil4_state_machine.c)

target_include_directories(sil4_state_machine PUBLIC include)

add_executable(sil4_state_machine_test
	tests/unit/test_state_machine.c)

target_link_libraries(sil4_state_machine_test sil4_state_machine)

