cmake_minimum_required(VERSION 3.12)

project(rsrx_rasta_reimplementation
	LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

add_library(rsrx_state_machine STATIC
	src/rsrx_state_machine.c)

target_include_directories(rsrx_state_machine PUBLIC include)

add_library(rsrx_orchestrator STATIC
	src/rsrx_orchestrator.c)

target_include_directories(rsrx_orchestrator PUBLIC include)
target_link_libraries(rsrx_orchestrator PUBLIC rsrx_state_machine)

add_library(rsrx_platform_adapters STATIC
	src/rsrx_platform_adapters.c)

target_include_directories(rsrx_platform_adapters PUBLIC include)
target_link_libraries(rsrx_platform_adapters PUBLIC rsrx_orchestrator rsrx_codec)

add_library(rsrx_codec STATIC
	src/rsrx_codec.c)

target_include_directories(rsrx_codec PUBLIC include)

add_library(rsrx_config_validator STATIC
	src/rsrx_config_validator.c)

target_include_directories(rsrx_config_validator PUBLIC include)

add_library(rsrx_api STATIC
	src/rsrx_api.c)

target_include_directories(rsrx_api PUBLIC include)
target_link_libraries(rsrx_api PUBLIC rsrx_platform_adapters rsrx_config_validator)

add_library(rsrx_transport_supervisor STATIC
	src/rsrx_transport_supervisor.c)

target_include_directories(rsrx_transport_supervisor PUBLIC include)
target_link_libraries(rsrx_transport_supervisor PUBLIC rsrx_api rsrx_codec)

add_executable(rsrx_state_machine_test
	tests/unit/test_rsrx_state_machine.c)

target_link_libraries(rsrx_state_machine_test rsrx_state_machine)

add_executable(rsrx_orchestrator_test
	tests/unit/test_rsrx_orchestrator.c)

target_link_libraries(rsrx_orchestrator_test rsrx_orchestrator)

add_executable(rsrx_platform_contract_test
	tests/unit/test_rsrx_platform_contract.c)

add_executable(rsrx_platform_adapters_test
	tests/unit/test_rsrx_platform_adapters.c)

target_link_libraries(rsrx_platform_adapters_test rsrx_platform_adapters)

add_executable(rsrx_transport_contract_test
	tests/unit/test_rsrx_transport_contract.c)

add_executable(rsrx_api_test
	tests/unit/test_rsrx_api.c)

target_link_libraries(rsrx_api_test rsrx_api)

add_executable(rsrx_config_validator_test
	tests/unit/test_rsrx_config_validator.c)

target_link_libraries(rsrx_config_validator_test rsrx_config_validator rsrx_codec)

add_executable(rsrx_codec_contract_test
	tests/unit/test_rsrx_codec_contract.c)

add_executable(rsrx_codec_test
	tests/unit/test_rsrx_codec.c)

target_link_libraries(rsrx_codec_test rsrx_codec)

add_executable(rsrx_transport_supervisor_test
	tests/unit/test_rsrx_transport_supervisor.c)

target_link_libraries(rsrx_transport_supervisor_test rsrx_transport_supervisor)
