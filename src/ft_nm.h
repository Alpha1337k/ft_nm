#ifndef FT_NM_H
#define FT_NM_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define LOAD_STRUCTURE(type) *(type *)read_bytes(sizeof(type));

typedef struct s_mach_o_header
{
    u_int32_t	magic_number;
    u_int32_t	cpu_type;
    u_int32_t	cpu_subtype;
    u_int32_t	file_type;
    u_int32_t	number_of_load_commands;
    u_int32_t	size_of_load_commands;
    u_int32_t	flags;
    u_int32_t	reserved;
} t_mach_o_header;

typedef struct s_mach_o_load
{
    u_int32_t type;
    u_int32_t size;
} t_mach_o_load;

typedef struct s_mach_o_symbol_table
{
    u_int32_t   symbols_offset;
    u_int32_t   symbols_count;
    u_int32_t   string_table_offset;
    u_int32_t   string_table_size;
} t_mach_o_symbol_table;

typedef struct s_mach_o_symbol_entry
{
    u_int32_t   name_offset;
    u_int8_t    symbol_type;
    u_int8_t    section_number;
    u_int16_t   data_info;
    u_int64_t   symbol_address;
} t_mach_o_symbol_entry ;

typedef struct s_reader
{
    char *ptr;
    size_t cur_offset;
    size_t size;
} t_reader;

int open_file(char *file);
void *read_bytes(size_t amount);
void move_to_offset(size_t offset);
size_t get_offset();


#endif