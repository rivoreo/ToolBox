//#include <bits/types.h>
#include <stdint.h>

// Little-end
struct portable_executable {
	uint32_t pe_signature;
	uint16_t machine;
	uint16_t sections_count;
	uint32_t time_stamp;
	uint32_t symbol_table_pointer;
	uint32_t symbols_count;
	uint16_t optional_header_size;
	uint16_t characteristics;

	// Optional Header
	uint16_t magic;
	uint8_t major_linker_version;
	uint8_t minor_linker_version;
	uint32_t code_size;
	uint32_t initialized_data_size;
	uint32_t uninitialized_data_size;
	uint32_t entry_point_address;
	uint32_t code_base;
	uint32_t data_base;
	uint32_t image_base;
	uint32_t section_alignment;
	uint32_t file_alignment;
	uint16_t major_os_version;
	uint16_t minor_os_version;
	uint16_t major_image_version;
	uint16_t minor_image_version;
	uint16_t major_subsystem_version;
	uint16_t minor_subsystem_version;
	uint32_t may_be_a_version;
	uint32_t image_size;
	uint32_t headers_size;
	uint32_t check_sum;
	uint16_t subsystem;
	uint16_t dll_characteristics;
	uint32_t stack_reserve_size;
	uint32_t stack_commit_size;
	uint32_t heap_reserve_size;
	uint32_t heap_commit_size;
	uint32_t loader_flags;
} __attribute__((packed));
