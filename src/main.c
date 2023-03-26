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
    move_to_offset(cur_offset);
    return rv;
}

t_elf_symbol_wrap *load_symbols(t_elf_symbol_query tables, u_int64_t count, u_int64_t sheader_offset, t_elf_sheader name_header, t_ft_nm options, char is_64_bit)
{
    u_int64_t cur_offset = get_offset();
    u_int64_t iteration_offset;
    move_to_offset(tables.symbol.file_offset);

    t_elf_symbol_wrap *items = malloc(sizeof(t_elf_symbol_wrap) * count);
    assert(items != 0);

    // first one is always 0
    if (is_64_bit) {
        (void)LOAD_STRUCTURE(t_elf_symbol_64);
    }
    else {
        (void)LOAD_STRUCTURE(t_elf_symbol_32);
    }
    for (size_t i = 0; i < count; i++)
    {
        if (is_64_bit) {
            MIGRATE_SYMBOL(t_elf_symbol_64, items[i].entry);
        } else {
            MIGRATE_SYMBOL(t_elf_symbol_32, items[i].entry);
        }
        iteration_offset = get_offset();
        move_to_offset(tables.name.file_offset + items[i].entry.name);
        items[i].name = &LOAD_STRUCTURE(char);
        if (items[i].entry.shndx != 65521) {
            items[i].has_sheader = 1;
            items[i].sheader = get_sheader(items[i].entry.shndx, sheader_offset, is_64_bit);
        }
        else
            items[i].has_sheader = 0;
        if (!*items[i].name && items[i].has_sheader && options.debug_syms == 1) {
            u_int64_t nested_offset = get_offset();
            move_to_offset(name_header.file_offset + items[i].sheader.sheader_name);
            items[i].name = &LOAD_STRUCTURE(char);
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
    u_int8_t visb = ELF64_ST_VISIBILITY(item.entry.other);



    if (item.has_sheader) {
        dprintf(2, "%d %ld %d ", item.sheader.section_type, item.sheader.flags, item.sheader.section_info);
    }
    dprintf(2, "%d %d %d %d ", item.entry.info, type, bind, visb);
    if (!item.has_sheader && options.debug_syms == 0) {
        dprintf(2, "NONE\n");
        return 0;
    }

    if (!item.has_sheader && options.debug_syms && type == 4)
        return 'a';
    else if (!item.has_sheader && options.debug_syms){
        dprintf(2, "NONE\n");
        return 0;
    }
    char rv = '?';

    u_int64_t cur_offset = get_offset();
    move_to_offset(name_header.file_offset + item.sheader.sheader_name);
    char *ss = &LOAD_STRUCTURE(char);
    move_to_offset(cur_offset);

    dprintf(2, "'%s' '%s'\n", ss, item.name);

    if (bind == 2)
        rv = 'W';
    else if (strncmp(ss, ".bss", 4) == 0)
        rv = 'B';
    else if (strncmp(ss, ".data", 5) == 0 || (item.sheader.flags & 0x3) == 0x3)
        rv = 'D';
    else if (strncmp(ss, ".text", 5) == 0)
        rv = 'T';
    else if (strcmp(ss, "") == 0) {
        if (bind == 2)
            rv = 'W';
        else
            rv = 'U';
    } else {
        if ((item.sheader.flags & 0x1) == 0 && type != 2)
            rv = 'R';
        else
            rv = 'T';
    }

    if (bind != 1 && (bind == 2 && type == 0 && item.sheader.section_type != 0) == 0)
        rv += 32;

    return rv;

}

t_elf_symbol_query get_tables(size_t count, u_int64_t offset, char is_64bit) {
    t_elf_symbol_query rv;
    bzero(&rv, sizeof(rv));
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
        if (sh.section_type == 2) {
            rv.symbol = sh;
            rv.name = get_sheader(sh.section_link_idx, offset, is_64bit);
            break;
        }
    }
    move_to_offset(cur_offset);
    return rv;
}

void print_elf(t_elf_header h, t_ft_nm options)
{
    u_int64_t sheader_offset = 0;
    if (h.format_bits == 2) {
        t_elf_header_extended_64 ext = LOAD_STRUCTURE(t_elf_header_extended_64);
        sheader_offset = ext.entry_point_sheader;
    }
    else {
        t_elf_header_extended_32 ext = LOAD_STRUCTURE(t_elf_header_extended_32);    
        sheader_offset = ext.entry_point_sheader;
    }
    t_elf_header_final fin = LOAD_STRUCTURE(t_elf_header_final);
    t_elf_symbol_query q =  get_tables(fin.sheader_len, sheader_offset, h.format_bits == 2);

    if (q.name.sheader_name == 0 || q.symbol.sheader_name == 0) {
        printf("ft_nm: no symbols\n");
        return;
    }


    u_int64_t count = (q.symbol.section_size / q.symbol.section_total_size) - 1;


    t_elf_sheader name_table = get_sheader(fin.sheader_name_index, sheader_offset, h.format_bits == 2);
    t_elf_symbol_wrap *items = load_symbols(q, count, sheader_offset, name_table, options, h.format_bits == 2);

    items = elf_sort_symbol_table(items, count, sorters[options.sorttype]);

    int padding = h.format_bits == 2 ? 16 : 8;

    for (size_t i = 0; i < count; i++)
    {
        if (!*items[i].name && options.debug_syms == 0)
            continue;
        dprintf(2, "''%30s''\t", items[i].name);
        char type = elf_resolve_type(items[i], name_table, options);

        if (type == 0 || filters[options.filter](type) == 0)
            continue;
        if (type != 'U' && type != 'u' && type != 'w')
            printf("%.*lx %c %s\n", padding, items[i].entry.value, type, items[i].name);
        else
            printf("%*s %c %s\n", padding, "", type, items[i].name);
    }
    free(items);
    

}

int main(int argc, char **argv)
{
    char *fallback[1] = {"a.out"};
    char **targets;
    size_t target_len = 1;
    t_ft_nm options;

    if (argc < 2)
        targets = fallback;
    else {
        targets = &argv[1];
        target_len = argc - 1;
    }

    for (size_t i = 0; i < target_len; i++)
    {
        if (targets[i][0] == '-') {
            for (size_t x = 1; targets[i][x]; x++)
            {
                switch (targets[i][x])
                {
                case 'r':
                    options.sorttype = 1;
                    break;
                case 'p':
                    options.sorttype = 2;
                    break;
                case 'u':
                    options.filter |= 0x1;
                    break;
                case 'g':
                    options.filter |= 0x2;
                    break;
                case 'a':
                    options.debug_syms = 1;
                    break;
                
                default:
                    printf("ft_nm: '%c' is an invalid paramter\n", targets[i][x]);
                    exit(1);
                }
            }
            
        }
    }

    for (size_t i = 0; i < target_len; i++)
    {
        if (targets[i][0] == '-')
            continue;
        open_file(targets[i]);
        t_elf_header h = LOAD_STRUCTURE(t_elf_header);
        if (h.magic_number != 1179403647) {
            printf("ft_nm: %s: file format not recognized\n", targets[i]);
            close_file();
            continue;
        }
        print_elf(h, options);
        close_file();
    }

    return 0;    
}
