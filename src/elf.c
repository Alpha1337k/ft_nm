#include <ft_nm.h>

int (*sorters[3])(char *, char *) = {
    normal_sort,
    reverse_sort,
    no_sort
};

int (*filters[4])(char) = {
    normal_filter,
    undefined_filter,
    external_filter,
    combined_filters,
};

int error = 0;

t_elf_sheader get_sheader(u_int16_t index, u_int64_t start_offset, char is_64_bit)
{
    u_int64_t cur_offset = get_offset();
    t_elf_sheader rv;
    
    if (is_64_bit) {
        move_to_offset(start_offset + (sizeof(t_elf_sheader_64) * index));
        MIGRATE_SHEADER(t_elf_sheader_64, rv);
    } else {
        move_to_offset(start_offset + (sizeof(t_elf_sheader_32) * index));
        MIGRATE_SHEADER(t_elf_sheader_32, rv);
    }
    if (error) {
        exit(1);
    }
    move_to_offset(cur_offset);
    return rv;
}

t_elf_symbol_query get_tables(size_t count, u_int64_t offset, char is_64bit) {
    t_elf_symbol_query rv;
    ft_memset(&rv, 0, sizeof(rv));
    u_int64_t cur_offset = get_offset();
    move_to_offset(offset);
    for (size_t i = 0; i < count; i++)
    {
        t_elf_sheader sh;
        if (is_64bit) {
            MIGRATE_SHEADER(t_elf_sheader_64, sh);
        } else {
            MIGRATE_SHEADER(t_elf_sheader_32, sh);
        }
		if (error)
			return rv;
        if (sh.section_type == 2) {
            rv.symbol = sh;
            rv.name = get_sheader(sh.section_link_idx, offset, is_64bit);
            break;
        }
    }
    move_to_offset(cur_offset);
    return rv;
}

t_elf_symbol_wrap *load_symbols(t_elf_symbol_query tables, u_int64_t count, u_int64_t sheader_offset, t_elf_sheader name_header, t_ft_nm options)
{
    u_int64_t cur_offset = get_offset();
    u_int64_t iteration_offset;
    move_to_offset(tables.symbol.file_offset + (options.is_64_bit ? sizeof(t_elf_symbol_64) : sizeof(t_elf_symbol_32)));

    t_elf_symbol_wrap *items = malloc(sizeof(t_elf_symbol_wrap) * count);
    if (!items)
        return 0;

    for (size_t i = 0; i < count; i++)
    {
        if (options.is_64_bit) {
            MIGRATE_SYMBOL(t_elf_symbol_64, items[i].entry);
        } else {
            MIGRATE_SYMBOL(t_elf_symbol_32, items[i].entry);
        }
		if (error) {
			free(items);
			return 0;
		}

        iteration_offset = get_offset();
        move_to_offset(tables.name.file_offset + items[i].entry.name);
        items[i].name = LOAD_STRUCTURE(char);
		if (error) {
			free(items);
			return 0;
		}
        if (items[i].entry.shndx != 65521) {
            items[i].has_sheader = 1;
            items[i].sheader = get_sheader(items[i].entry.shndx, sheader_offset, options.is_64_bit);
			if (error) {
				free(items);
				return 0;
			}
        }
        else
            items[i].has_sheader = 0;
        if (!*items[i].name && items[i].has_sheader && (items[i].sheader.flags != 0 || items[i].entry.info != 0) && options.debug_syms == 1) {
            u_int64_t nested_offset = get_offset();
            move_to_offset(name_header.file_offset + items[i].sheader.sheader_name);
            items[i].name = LOAD_STRUCTURE(char);
			if (error) {
				free(items);
				return 0;
			}
            move_to_offset(nested_offset);
        }
        move_to_offset(iteration_offset);
    }
    move_to_offset(cur_offset);
    
    return items;
}

char elf_resolve_type(t_elf_symbol_wrap item, t_elf_sheader name_header, t_ft_nm options)
{
    u_int8_t type = ELF64_ST_TYPE(item.entry.info);
    u_int8_t bind = ELF64_ST_BIND(item.entry.info);

    if (!item.has_sheader && options.debug_syms == 0) {
        return 0;
    }

    if (!item.has_sheader && options.debug_syms && type == 4)
        return 'a';
    else if (!item.has_sheader && options.debug_syms){
        return 0;
    }
    char rv = '?';

    u_int64_t cur_offset = get_offset();
    move_to_offset(name_header.file_offset + item.sheader.sheader_name);
    char *ss = LOAD_STRUCTURE(char);
	if (error) {
		return -1;
	}
    move_to_offset(cur_offset);

    if (bind == 2) {
        if (type == 1)
            rv = 'V';
        else
            rv = 'W';
        if (item.entry.shndx == 0)
            rv += 32;
        return rv;
    }
    else if (item.entry.shndx == 0) {
        rv = 'U';
    }
    else if (item.entry.shndx == 0xfff2) {
        rv = 'C';
    } else if (item.entry.shndx == 0xfff1) {
        rv = 'A';
    }
    else if (item.sheader.section_type == 8) {
        rv = 'B';
    }
    else if (1) {
        if ((item.sheader.flags & 0x3) == 0x3)
            rv = 'D';
        else if ((item.sheader.flags & 0x6) == 0x6)
            rv = 'T';
        else if ((item.sheader.flags & 0x1) == 0 && item.sheader.flags & 0x2)
            rv = 'R';
        else
            rv = 'N';
    }


    if ((bind & 0x1) == 0 && (rv != 'N' || ft_strncmp(ss, ".debug", 6) != 0))
		rv += 32;
    if ((!*item.name) && options.debug_syms == 0 && item.entry.value != 2325)
        return 0;
    return rv;

}

void print_elf(t_elf_symbol_wrap *items, u_int64_t count, t_elf_sheader name_table, t_ft_nm options)
{
    int padding = options.is_64_bit ? 16 : 8;
    for (size_t i = 0; i < count; i++)
    {
        char type = elf_resolve_type(items[i], name_table, options);

		if (type == -1) {
			print_reader_error();
			return;
		}

        if (type == 0 || filters[options.filter](type) == 0)
            continue;
        if (type != 'U' && type != 'u' && type != 'w')
            print_result_addr(padding, items[i].entry.value, type, items[i].name);
        else
            print_result_empt(padding, type, items[i].name);
    }
}

void handle_elf(t_ft_nm options, char *filename)
{
    u_int64_t sheader_offset = 0;
    if (options.is_64_bit)
        sheader_offset = (*LOAD_STRUCTURE(t_elf_header_extended_64)).entry_point_sheader;
    else {
        sheader_offset = (*LOAD_STRUCTURE(t_elf_header_extended_32)).entry_point_sheader;
	}

	if (error) return print_reader_error();  
    t_elf_header_final fin = *LOAD_STRUCTURE(t_elf_header_final);
	if (error) return print_reader_error();
    t_elf_symbol_query query = get_tables(fin.sheader_len, sheader_offset, options.is_64_bit);

    if (query.name.sheader_name == 0 || query.symbol.sheader_name == 0 || error) {
		if (!error) {
            ft_puts(2, "ft_nm: ");
            ft_puts(2, filename);
            ft_puts(2, ": no symbols\n");
        }
		else print_reader_error();
        return;
    }

    u_int64_t count = (query.symbol.section_size / query.symbol.section_total_size) - 1;
    t_elf_sheader name_table = get_sheader(fin.sheader_name_index, sheader_offset, options.is_64_bit);

	if (error) {
		return print_reader_error(); 
	}
    t_elf_symbol_wrap *items = load_symbols(query, count, sheader_offset, name_table, options);
    if (!items)
    {
        ft_puts(2, "ft_nm: malloc failed\n");
        return;
    }
    items = elf_sort_symbol_table(items, count, sorters[options.sorttype]);
    print_elf(items, count, name_table, options);
    free(items);
}