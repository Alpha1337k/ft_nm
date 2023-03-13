#include <ft_nm.h>

u_int8_t resolve_type(u_int8_t type)
{
    u_int8_t rv;

    switch (type & 0x0E)
    {
    case 0x00:
        rv = 'U';
        break;
    case 0x02:
        rv = 'A';
        break;
    case 0x0A:
        rv = 'D';
        break;
    case 0x0C:
        rv = 'N';
        break;
    case 0x0E:
        rv = 'T';
        break;
    default:
        return '?';
    }

    if ((type & 0x11) == 17)
        rv += 32;
    return rv;
}

int handle_symbol_table()
{
    t_mach_o_symbol_table s_table = LOAD_STRUCTURE(t_mach_o_symbol_table);

    size_t old_offset = get_offset();

    for (size_t i = 0; i < s_table.symbols_count; i++)
    {
        move_to_offset(s_table.symbols_offset + (sizeof(t_mach_o_symbol_entry) * i));
        t_mach_o_symbol_entry entry = LOAD_STRUCTURE(t_mach_o_symbol_entry);

        char type = resolve_type(entry.symbol_type);

        if (type == 'U')
            printf("%16s %c ", "", type);
        else
            printf("%.16llx %c ", entry.symbol_address, type);

        move_to_offset(s_table.string_table_offset + entry.name_offset);
        char c = 1;
        while (c)
        {
            c = LOAD_STRUCTURE(char);
            printf("%c", c);
        }
        printf("\n");
        
    }

    move_to_offset(old_offset);
    
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;
    open_file(argv[1]);

    t_mach_o_header header = LOAD_STRUCTURE(t_mach_o_header);

    printf("%x\n", header.magic_number);
    for (size_t i = 0; i < header.number_of_load_commands; i++)
    {
        t_mach_o_load loaded = LOAD_STRUCTURE(t_mach_o_load);

        printf("%.2x %.2x\n", loaded.type, loaded.size);

        switch (loaded.type)
        {
        case 0x02:
            handle_symbol_table();
            break;
        
        default:
            break;
        }
        if (!read_bytes(loaded.size - sizeof(t_mach_o_load))) break;
    }
    
}