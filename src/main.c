#include <ft_nm.h>

char *resolve_type(u_int8_t type, u_int8_t section_type)
{
    char *rv;

    if (type & 0x20)
        return 0;

    switch (type & 0x0E)
    {
    case 0x00:
        rv = "U";
        break;
    case 0x02:
        rv = "A";
        break;
    case 0x0A:
        rv = "D";
        break;
    case 0x0C:
        rv = "N";
        break;
    case 0x0E:
        switch (section_type)
        {
        case 0x0A:
        case 0x0C:
            rv = "S";
            break;
        case 0x0B:
            rv = "D";
            break;
        
        default:
            rv = "T";
            break;
        }
        break;
    default:
        return "?";
    }

    if (rv[2] == 0 && (type & 0x11) == 17)
        rv += 32;
    return rv;
}

void sort_symbol_table(t_mach_o_symbol_wrap *entries, size_t len)
{
    int sorted = 0;
    while (!sorted)
    {
        sorted = 1;
        for (size_t i = 0; i < len - 1; i++)
        {
            if (strcmp(entries[i].name, entries[i + 1].name) > 0)
            {
                t_mach_o_symbol_wrap tmp = entries[i];
                entries[i] = entries[i + 1];
                entries[i + 1] = tmp;
                sorted = 0;
            }
        }
    }    
}

void print_symbols(t_mach_o_symbol_wrap *entries, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        // printf("%x %x ", entries[i].entry.symbol_type, entries[i].entry.section_number);
        char *type = resolve_type(entries[i].entry.symbol_type, entries[i].entry.section_number);
        if (!type)
            continue;
        if (*type != 'U')
            printf("%.16llx %s %s\n", entries[i].entry.symbol_address, type, entries[i].name);
        else
            printf("%16s %s %s\n", "", type, entries[i].name);
    }
    
}

int handle_symbol_table()
{
    t_mach_o_symbol_table s_table = LOAD_STRUCTURE(t_mach_o_symbol_table);

    size_t old_offset = get_offset();
    t_mach_o_symbol_wrap *entries = malloc(s_table.symbols_count * sizeof(t_mach_o_symbol_wrap)); 

    for (size_t i = 0; i < s_table.symbols_count; i++)
    {
        move_to_offset(s_table.symbols_offset + (sizeof(t_mach_o_symbol_entry) * i));
        entries[i].entry = LOAD_STRUCTURE(t_mach_o_symbol_entry);
        move_to_offset(s_table.string_table_offset + entries[i].entry.name_offset);
        entries[i].name = read_bytes(0);

        // t_mach_o_symbol_entry entry = 

        // char *type = resolve_type(entry.symbol_type);

        // if (*type == 'U')
        //     printf("%16s %s ", "", type);
        // else
        //     printf("%.16llx %s ", entry.symbol_address, type);

        // move_to_offset(s_table.string_table_offset + entry.name_offset);
        // char c = 1;
        // while (c)
        // {
        //     c = LOAD_STRUCTURE(char);
        //     printf("%c", c);
        // }
        // printf("\n");
    }

    sort_symbol_table(entries, s_table.symbols_count);
    print_symbols(entries, s_table.symbols_count);


    free(entries);

    move_to_offset(old_offset);
    
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    open_file(argv[1]);

    t_mach_o_header_multi multi;
    t_mach_o_header header = LOAD_STRUCTURE(t_mach_o_header);
    if (header.magic_number != HEADER_MULTI && header.magic_number != HEADER_64BITS)
        abort();
    if (header.magic_number == HEADER_MULTI)
    {
        move_to_offset(0);
        // multi is always b. endian
        multi = LOAD_STRUCTURE(t_mach_o_header_multi);

        // printf("%x\n", SWAP_ENDIAN(multi.file_offset));
        set_base_offset(SWAP_ENDIAN(multi.file_offset));
        header = LOAD_STRUCTURE(t_mach_o_header);
    }

    for (size_t i = 0; i < header.number_of_load_commands; i++)
    {
        t_mach_o_load loaded = LOAD_STRUCTURE(t_mach_o_load);

        // printf("%.2x %.2x\n", loaded.type, loaded.size);

        switch (loaded.type)
        {
        case 0x02:
            handle_symbol_table();
            break;
        
        default:
            break;
        }
        if (loaded.size == 0)
            break;
        if (!read_bytes(loaded.size - sizeof(t_mach_o_load))) break;
    }
    
}