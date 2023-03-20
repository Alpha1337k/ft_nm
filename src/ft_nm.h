#ifndef FT_NM_H
#define FT_NM_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>

#define HEADER_64BITS 4277009103
#define HEADER_MULTI 3199925962
#define SWAP_ENDIAN(num) (((num>>24)&0xff) |   \
                    ((num<<8)&0xff0000) |  \
                    ((num>>8)&0xff00) | \
                    ((num<<24)&0xff000000))

#define LOAD_STRUCTURE(type) *(type *)read_bytes(sizeof(type));

#define PRINT_MACRO(type, target) printf("%30s: ", #target); type(target);
#define PRINT_MACRO_S(type, target, len) printf("%30s: ", #target); type(target, len);

#define ELF64_ST_BIND(i)   ((i)>>4)
#define ELF64_ST_TYPE(i)   ((i)&0xf)
#define ELF64_ST_INFO(b,t) (((b)<<4)+((t)&0xf))
#define ELF64_ST_VISIBILITY(o) ((o)&0x3)

typedef struct s_elf_header
{
    u_int32_t   magic_number;
    u_int8_t    format_bits;
    u_int8_t    endianness;
    u_int8_t    elf_version;
    u_int8_t    target_os;
    u_int8_t    abi_version;
    u_int8_t    padding[7];
    u_int16_t   file_type;
    u_int16_t   architecture;
    u_int32_t   elf_version2;    
} t_elf_header;

typedef struct s_elf_header_extended_64
{
    u_int64_t   entry_point_program;
    u_int64_t   entry_point_pheader;
    u_int64_t   entry_point_sheader;
} t_elf_header_extended_64;

typedef struct s_elf_header_extended_32
{
    u_int32_t   entry_point_program;
    u_int32_t   entry_point_pheader;
    u_int32_t   entry_point_sheader;
} t_elf_header_extended_32;

typedef struct s_elf_header_final
{
    u_int32_t   flags;
    u_int16_t   header_size;
    u_int16_t   pheader_size;
    u_int16_t   pheader_len;
    u_int16_t   sheader_size;
    u_int16_t   sheader_len;
    u_int16_t   sheader_name_index;    
} t_elf_header_final;

typedef struct s_elf_sheader_64
{
    u_int32_t   sheader_name;
    u_int32_t   section_type;
    u_int64_t   flags;
    u_int64_t   file_addr;
    u_int64_t   file_offset;
    u_int64_t   section_size;
    u_int32_t   section_link_idx;
    u_int32_t   section_info;
    u_int64_t   section_alignment;
    u_int64_t   section_total_size;
} t_elf_sheader_64;

typedef struct s_elf_symbol_64 {
	u_int32_t	name;
	u_int8_t	info;
	u_int8_t	other;
	u_int16_t	shndx;
	u_int64_t	value;
	u_int64_t	size;
} t_elf_symbol_64;

typedef struct s_elf_symbol_query {
    t_elf_sheader_64 name;
    t_elf_sheader_64 symbol;
} t_elf_symbol_query;

typedef struct s_elf_symbol_wrap
{
    t_elf_symbol_64 entry;
    t_elf_sheader_64 *sheader;
    char *name;
} t_elf_symbol_wrap;


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

typedef struct s_ft_nm
{
    u_int8_t sorttype; // 0 def 1 rev (-r) 2 no (-p)
    u_int8_t filter; // 0 def 1 undefined (-u) 2 external (-g)
    u_int8_t debug_syms; // 0 false 1 true (-a) 
} t_ft_nm;

int open_file(char *file);
void close_file();
void *read_bytes(size_t amount);
void move_to_offset(size_t offset);
void set_base_offset(size_t offset);
size_t get_offset();

void print_char(u_int8_t target);
void print_short(short t);
void print_int(int t);
void print_long(long t);
void print_string(u_int8_t *s, size_t len);
void print_address(long addr);

int no_sort(char *a, char *b);
int reverse_sort(char *a, char *b);
int normal_sort(char *a, char *b);

int normal_filter(char type);
int undefined_filter(char type);
int external_filter(char type);

#endif