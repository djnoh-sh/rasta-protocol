cmake_minimum_required(VERSION 3.12)

project(rsrx_rasta_reimplementation
	LANGUAGES C)

set(CMAKE_C_STANDARD 11)

option(RSRX_ENABLE_STRICT_WARNING_HARDENING
	"Enable stricter compiler warning profile for evaluation builds"
	OFF)

option(RSRX_ENABLE_STACK_MEMORY_EVIDENCE
	"Generate host stack-usage files and a representative linker map"
	OFF)

set(RSRX_WARNING_FLAGS
	-Wall
	-Wextra
	-Werror)

if(RSRX_ENABLE_STRICT_WARNING_HARDENING)
	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		list(APPEND RSRX_WARNING_FLAGS
			-Wconversion
			-Wsign-conversion)
	endif()
endif()

add_compile_options(${RSRX_WARNING_FLAGS})

if(RSRX_ENABLE_STACK_MEMORY_EVIDENCE)
	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		add_compile_options(-fstack-usage)
	endif()
endif()

include_directories(include)

function(rsrx_configure_representative_memory_map target_name)
	if(RSRX_ENABLE_STACK_MEMORY_EVIDENCE)
		if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
			set_target_properties(${target_name}
				PROPERTIES
					LINK_FLAGS "-Wl,-Map=${CMAKE_BINARY_DIR}/${target_name}.map")
		endif()
	endif()
endfunction()

add_library(rsrx_state_machine STATIC
	src/rsrx_state_machine.c)

target_include_directories(rsrx_state_machine PUBLIC include)

add_library(rsrx_orchestrator STATIC
	src/rsrx_orchestrator.c)

target_include_directories(rsrx_orchestrator PUBLIC include)
target_link_libraries(rsrx_orchestrator PUBLIC rsrx_state_machine)

add_library(rsrx_codec STATIC
	src/rsrx_codec.c)

target_include_directories(rsrx_codec PUBLIC include)

add_library(rsrx_protocol_context STATIC
	src/rsrx_protocol_context.c)

target_include_directories(rsrx_protocol_context PUBLIC include)
target_link_libraries(rsrx_protocol_context PUBLIC rsrx_codec)

add_library(rsrx_channel_manager STATIC
	src/rsrx_channel_manager.c)

target_include_directories(rsrx_channel_manager PUBLIC include)

add_library(rsrx_platform_adapters STATIC
	src/rsrx_platform_adapters.c)

target_include_directories(rsrx_platform_adapters PUBLIC include)
target_link_libraries(rsrx_platform_adapters PUBLIC rsrx_channel_manager rsrx_orchestrator rsrx_codec rsrx_protocol_context)

add_library(rsrx_config_validator STATIC
	src/rsrx_config_validator.c)

target_include_directories(rsrx_config_validator PUBLIC include)
target_link_libraries(rsrx_config_validator PUBLIC rsrx_channel_manager)

add_library(rsrx_api STATIC
	src/rsrx_api.c)

target_include_directories(rsrx_api PUBLIC include)
target_link_libraries(rsrx_api PUBLIC rsrx_channel_manager rsrx_platform_adapters rsrx_config_validator)

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

add_executable(rsrx_channel_manager_test
	tests/unit/test_rsrx_channel_manager.c)

target_link_libraries(rsrx_channel_manager_test rsrx_channel_manager)

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

add_executable(rsrx_protocol_context_test
	tests/unit/test_rsrx_protocol_context.c)

target_link_libraries(rsrx_protocol_context_test rsrx_protocol_context)

add_executable(rsrx_transport_supervisor_test
	tests/unit/test_rsrx_transport_supervisor.c)

target_link_libraries(rsrx_transport_supervisor_test rsrx_transport_supervisor)

add_executable(rsrx_session_supervisor_flow_test
	tests/integration/test_rsrx_session_supervisor_flow.c)

target_link_libraries(rsrx_session_supervisor_flow_test rsrx_transport_supervisor rsrx_api rsrx_codec)
rsrx_configure_representative_memory_map(rsrx_session_supervisor_flow_test)
