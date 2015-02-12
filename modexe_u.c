/*	PE header item edit tool
	toolbox
	Copyright 2007-2015 PC GO Ld.
	Copyright 2015 libdll.so

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "pe-struct.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#define PE_ITEM_RW(ITEM)								\
	do {										\
		if(readonly) {								\
			*out = pe->ITEM;						\
			return 0;							\
		}									\
		pe->ITEM = in;								\
		if(save_file(file, (const char *)&pe->ITEM,				\
		offset + ((char *)&pe->ITEM - (char *)pe), sizeof pe->ITEM)) {		\
			return 0;							\
		}									\
		return 6;								\
	} while(0)

static int save_file(const char *filename, const char *buffer, off_t offset, size_t len) {
	int fd = open(filename, O_WRONLY);
	if(fd == -1) {
		perror(filename);
		return 0;
	}
	//if(!len) len = BUF_SIZE;
	if(offset) lseek(fd, offset, SEEK_SET);
	return write(fd, buffer, len) != -1;
}

static int pe_file_rw(struct portable_executable *pe, const char *item, char readonly, ...) {
	va_list ap;
	uint32_t *out;
	uint32_t in;
	const char *file;
	off_t offset;
	va_start(ap, readonly);
	if(readonly) {
		out = va_arg(ap, uint32_t *);
	} else {
		in = va_arg(ap, uint32_t);
		file = va_arg(ap, const char *);
		offset = va_arg(ap, off_t);
	}
	va_end(ap);
	if(strcmp(item, "machine") == 0) PE_ITEM_RW(machine);
	else if(strcmp(item, "major_linker_version") == 0) PE_ITEM_RW(major_linker_version);
	else if(strcmp(item, "minor_linker_version") == 0) PE_ITEM_RW(minor_linker_version);
	else if(strcmp(item, "code_size") == 0) PE_ITEM_RW(code_size);
	else if(strcmp(item, "initialized_data_size") == 0) PE_ITEM_RW(initialized_data_size);
	else if(strcmp(item, "uninitialized_data_size") == 0) PE_ITEM_RW(uninitialized_data_size);
	else if(strcmp(item, "entry_point_address") == 0) PE_ITEM_RW(entry_point_address);
	else if(strcmp(item, "code_base") == 0) PE_ITEM_RW(code_base);
	else if(strcmp(item, "data_base") == 0) PE_ITEM_RW(data_base);
	else if(strcmp(item, "image_base") == 0) PE_ITEM_RW(image_base);
	else if(strcmp(item, "major_os_version") == 0) PE_ITEM_RW(major_os_version);
	else if(strcmp(item, "minor_os_version") == 0) PE_ITEM_RW(minor_os_version);
	else if(strcmp(item, "major_image_version") == 0) PE_ITEM_RW(major_image_version);
	else if(strcmp(item, "minor_image_version") == 0) PE_ITEM_RW(minor_image_version);
	else if(strcmp(item, "major_subsystem_version") == 0) PE_ITEM_RW(major_subsystem_version);
	else if(strcmp(item, "minor_subsystem_version") == 0) PE_ITEM_RW(minor_subsystem_version);
	else if(strcmp(item, "subsystem") == 0) PE_ITEM_RW(subsystem);
	else if(strcmp(item, "dll_characteristics") == 0) PE_ITEM_RW(dll_characteristics);
	else if(strcmp(item, "stack_reserve_size") == 0) PE_ITEM_RW(stack_reserve_size);
	else if(strcmp(item, "stack_commit_size") == 0) PE_ITEM_RW(stack_commit_size);
	else if(strcmp(item, "heap_reserve_size") == 0) PE_ITEM_RW(heap_reserve_size);
	else if(strcmp(item, "heap_commit_size") == 0) PE_ITEM_RW(heap_commit_size);
	else if(strcmp(item, "loader_flags") == 0) PE_ITEM_RW(loader_flags);
	return 1;
}

int modexe_main(int argc, char **argv) {
	if(argc < 3) {
		fprintf(stderr, "Usage: %s <file> <item> [<new value>]\n", argv[0]);
		return -1;
	}

	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		perror(argv[1]);
		return 2;
	}

	lseek(fd, 0x3c, SEEK_SET);
	unsigned char pe_offset;
	if(read(fd, &pe_offset, 1) < 1) {
		perror(argv[1]);
		return 5;
	}
	if(!pe_offset) {
		fprintf(stderr, "Unexcepted PE offset, %s may not a valid PE file\n", argv[1]);
		return 4;
	}
	lseek(fd, pe_offset, SEEK_SET);

	int s = 0;
	struct portable_executable pe;
	do {
		int r = read(fd, ((char *)&pe) + s, 1);
		if(r < 1) break;
		s += r;
	} while(s < sizeof pe);
	close(fd);

	if(pe.pe_signature != 0x4550) {
		fprintf(stderr, "The PE signature not found, %s may not a valid PE file\n", argv[1]);
		return 3;
	}

	int r;
	uint32_t value;
	if(argc == 3) {
		r = pe_file_rw(&pe, argv[2], 1, &value);
		if(r == 0) {
			printf("%s = %u\n", argv[2], value);
			return 0;
		}
	} else {
		r = pe_file_rw(&pe, argv[2], 0, atoi(argv[3]), argv[1], pe_offset);
	}

	if(r == 1) fprintf(stderr, "Item %s unknown or unsupported\n", argv[2]);
	else if(r > 5) perror(argv[1]);

	return r;
}
