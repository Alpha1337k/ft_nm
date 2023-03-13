#ifndef FT_NM_H
#define FT_NM_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define HEADER_64BITS 4277009103
#define HEADER_MULTI 3199925962
#define SWAP_ENDIAN(num) (((num>>24)&0xff) |   \
                    ((num<<8)&0xff0000) |  \
                    ((num>>8)&0xff00) | \
                    ((num<<24)&0xff000000))

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

typedef struct s_mach_o_header_multi
{
    u_int32_t	magic_number;
    u_int32_t	binaries_count;
    u_int32_t	cpu_type;
    u_int32_t	cpu_subtype;
    u_int32_t	file_offset;
    u_int32_t	size;
    u_int32_t	section_alignment;
} t_mach_o_header_multi;

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

typedef struct s_mach_o_symbol_wrap
{
    t_mach_o_symbol_entry entry;
    char *name;
} t_mach_o_symbol_wrap;


typedef struct s_reader
{
    char *ptr;
    size_t cur_offset;
    size_t size;
    size_t base_offset;
} t_reader;

int open_file(char *file);
void *read_bytes(size_t amount);
void move_to_offset(size_t offset);
void set_base_offset(size_t offset);
size_t get_offset();


#endif