#include <ft_nm.h>

t_elf_sheader_64 *get_sheader(u_int16_t index, u_int64_t start_offset)
{
    u_int64_t cur_offset = get_offset();
    move_to_offset(start_offset + (sizeof(t_elf_sheader_64) * index));
    

    t_elf_sheader_64 *rv = &LOAD_STRUCTURE(t_elf_sheader_64);

    move_to_offset(cur_offset);
    return rv;
}

t_elf_symbol_wrap *load_symbols(t_elf_symbol_query tables, u_int64_t count, u_int64_t sheader_offset)
{
    u_int64_t cur_offset = get_offset();
    u_int64_t iteration_offset;
    move_to_offset(tables.symbol.file_offset);

    t_elf_symbol_wrap *items = malloc(sizeof(t_elf_symbol_wrap) * count);
    assert(items != 0);

    for (size_t i = 0; i < count; i++)
    {
        items[i].entry = LOAD_STRUCTURE(t_elf_symbol_64);
        iteration_offset = get_offset();
        move_to_offset(tables.name.file_offset + items[i].entry.name);
        items[i].name = &LOAD_STRUCTURE(char);
        if (items[i].entry.shndx != 65521)
            items[i].sheader = get_sheader(items[i].entry.shndx, sheader_offset);
        else
            items[i].sheader = 0;
        move_to_offset(iteration_offset);
    }
    move_to_offset(cur_offset);
    
    return items;
}

void elf_sort_symbol_table(t_elf_symbol_wrap *entries, size_t len)
{
    int sorted = 0;
    while (!sorted)
    {
        sorted = 1;
        for (size_t i = 0; i < len - 1; i++)
        {
            if (strcmp(entries[i].name, entries[i + 1].name) > 0)
            {
                t_elf_symbol_wrap tmp = entries[i];
                entries[i] = entries[i + 1];
                entries[i + 1] = tmp;
                sorted = 0;
            }
        }
    }    
}

char elf_resolve_type(t_elf_symbol_64 symbol, t_elf_sheader_64 *header, t_elf_sheader_64 *name_header)
{
    u_int8_t type = ELF64_ST_TYPE(symbol.info);
    u_int8_t bind = ELF64_ST_BIND(symbol.info);
    u_int8_t visb = ELF64_ST_VISIBILITY(symbol.other);



    if (header) {
        dprintf(2, "%d %ld %d ", header->section_type, header->flags, header->section_info);
    }
    dprintf(2, "%d %d %d ", type, bind, visb);
    if (!header) {
        dprintf(2, "\n");
        return 0;
    }

    char rv = '?';

    u_int64_t cur_offset = get_offset();
    move_to_offset(name_header->file_offset + header->sheader_name);
    char *ss = &LOAD_STRUCTURE(char);
    dprintf(2, "'%s'\n", ss);
    move_to_offset(cur_offset);

    if (bind == 2)
        rv = 'W';
    else if (strcmp(ss, ".bss") == 0)
        rv = 'B';
    else if (strcmp(ss, ".data") == 0 || (header->flags & 0x3) == 0x3)
        rv = 'D';
    else if (strcmp(ss, ".text") == 0)
        rv = 'T';
    else if (strcmp(ss, "") == 0) {
        if (bind == 2)
            rv = 'W';
        else
            rv = 'U';
    } else {
        if ((header->flags & 0x1) == 0 && type != 2)
            rv = 'R';
        else
            rv = 'T';
    }

    if (bind != 1 && (bind == 2 && type == 0 && header->section_type != 0) == 0)
        rv += 32;

    return rv;

}

t_elf_symbol_query get_tables(size_t count, u_int64_t offset) {
    //
    t_elf_symbol_query rv;
    bzero(&rv, sizeof(rv));
    u_int64_t cur_offset = get_offset();
    move_to_offset(offset);
    for (size_t i = 0; i < count; i++)
    {
        t_elf_sheader_64 sh = LOAD_STRUCTURE(t_elf_sheader_64);
        if (sh.section_type == 2) {
            rv.symbol = sh;
            rv.name = *get_sheader(sh.section_link_idx, offset);
            break;
        }
    }
    move_to_offset(cur_offset);
    return rv;
}

void print_elf(t_elf_header h)
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

    t_elf_symbol_query q =  get_tables(fin.sheader_len, sheader_offset);
    if (q.name.sheader_name == 0 || q.symbol.sheader_name == 0) 
        abort();


    u_int64_t count = q.symbol.section_size / q.symbol.section_total_size;
    t_elf_symbol_wrap *items = load_symbols(q, count, sheader_offset);
    t_elf_sheader_64 *name_table = get_sheader(fin.sheader_name_index, sheader_offset);

    elf_sort_symbol_table(items, count);

    for (size_t i = 0; i < count; i++)
    {
        if (!*items[i].name)
            continue;
        dprintf(2, "%30s\t", items[i].name);
        char type = elf_resolve_type(items[i].entry, items[i].sheader, name_table);
        if (type == 0)
            continue;
        if (items[i].entry.value != 0)
            printf("%.16lx %c %s\n", items[i].entry.value, type, items[i].name);
        else
            printf("%16s %c %s\n", "", type, items[i].name);
    }
    

}

int main(int argc, char **argv)
{
    char *fallback[1] = {"a.out"};
    char **targets;
    size_t target_len = 1;
    if (argc < 2)
        targets = fallback;
    else {
        targets = &argv[1];
        target_len = argc - 1;
    }

    for (size_t i = 0; i < target_len; i++)
    {
        open_file(targets[i]);

        t_elf_header h = LOAD_STRUCTURE(t_elf_header);
        if (h.magic_number != 1179403647)
            abort();
        print_elf(h);
    }

    return 1;
    


    // t_elf_header h = LOAD_STRUCTURE(t_elf_header);

    // PRINT_MACRO(print_int, h.magic_number);
    // PRINT_MACRO(print_char, h.format_bits);
    // PRINT_MACRO(print_char, h.endianness);
    // PRINT_MACRO(print_char, h.elf_version);
    // PRINT_MACRO(print_char, h.target_os);
    // PRINT_MACRO(print_char, h.abi_version);
    // PRINT_MACRO_S(print_string, h.padding, 7);
    // PRINT_MACRO(print_short, h.file_type);
    // PRINT_MACRO(print_char, h.architecture);
    // PRINT_MACRO(print_int, h.elf_version2);

    // printf("\n\n");

    // u_int64_t sheader_offset = 0;
    // if (h.format_bits == 2) {
    //     t_elf_header_extended_64 ext = LOAD_STRUCTURE(t_elf_header_extended_64);
    //     PRINT_MACRO(print_address, ext.entry_point_program);
    //     PRINT_MACRO(print_address, ext.entry_point_pheader);
    //     PRINT_MACRO(print_address, ext.entry_point_sheader);
    //     sheader_offset = ext.entry_point_sheader;
    // }
    // else {
    //     t_elf_header_extended_32 ext = LOAD_STRUCTURE(t_elf_header_extended_32);
    //     PRINT_MACRO(print_address, ext.entry_point_program);
    //     PRINT_MACRO(print_address, ext.entry_point_pheader);
    //     PRINT_MACRO(print_address, ext.entry_point_sheader);       
    //     sheader_offset = ext.entry_point_sheader;
    // }
    // t_elf_header_final fin = LOAD_STRUCTURE(t_elf_header_final);
    // printf("\n\n");

    // PRINT_MACRO(print_int, fin.flags);
    // PRINT_MACRO(print_short, fin.header_size);
    // PRINT_MACRO(print_short, fin.pheader_size);
    // PRINT_MACRO(print_short, fin.pheader_len);
    // PRINT_MACRO(print_short, fin.sheader_size);
    // PRINT_MACRO(print_short, fin.sheader_len);
    // PRINT_MACRO(print_short, fin.sheader_name_index);


    // move_to_offset(sheader_offset);

    // printf("\n\n");

    // for (size_t i = 0; i < fin.sheader_len; i++)
    // {
    //     t_elf_sheader_64 sh = LOAD_STRUCTURE(t_elf_sheader_64);
    
    //     printf("%ld\n", i);
    //     PRINT_MACRO(print_int, sh.sheader_name);
    //     PRINT_MACRO(print_long, sh.section_type);
    //     PRINT_MACRO(print_long, sh.flags);
    //     PRINT_MACRO(print_long, sh.file_addr);
    //     PRINT_MACRO(print_long, sh.file_offset);
    //     PRINT_MACRO(print_long, sh.section_size);
    //     PRINT_MACRO(print_int, sh.section_link_idx);
    //     PRINT_MACRO(print_int, sh.section_info);
    //     PRINT_MACRO(print_long, sh.section_alignment);
    //     PRINT_MACRO(print_long, sh.section_total_size);

    //     size_t cur_offset = get_offset();

    //     move_to_offset(sh.file_offset);

    //     if (sh.section_type == 2) {

    //         for (size_t x = 0; x < sh.section_size / sh.section_total_size; x++)
    //         {
    //             printf("\t%ld\n", x);
    //             t_elf_symbol_64 sym = LOAD_STRUCTURE(t_elf_symbol_64);
    //             PRINT_MACRO(print_int,  sym.name);
    //             PRINT_MACRO(print_char, sym.info);
    //             PRINT_MACRO(print_char, sym.other);
    //             PRINT_MACRO(print_short, sym.shndx);
    //             PRINT_MACRO(print_long, sym.value);
    //             PRINT_MACRO(print_long, sym.size);
    //             size_t cur_offset2 = get_offset();
    //             move_to_offset(sym.name + 18008);
    //             char *b = &LOAD_STRUCTURE(char);

    //             printf("%u %u\n", ELF64_ST_BIND(sym.info), ELF64_ST_TYPE(sym.info));

                
    //             printf("%p %s\n",b, b);
    //             move_to_offset(cur_offset2);

    //         }
            
    //     }

    //     printf("\n\n");

    //     move_to_offset(cur_offset);
    // }

    // t_mach_o_header_multi multi;
    // t_mach_o_header header = LOAD_STRUCTURE(t_mach_o_header);
    // if (header.magic_number != HEADER_MULTI && header.magic_number != HEADER_64BITS)
    //     abort();
    // if (header.magic_number == HEADER_MULTI)
    // {
    //     move_to_offset(0);
    //     // multi is always b. endian
    //     multi = LOAD_STRUCTURE(t_mach_o_header_multi);

    //     // printf("%x\n", SWAP_ENDIAN(multi.file_offset));
    //     set_base_offset(SWAP_ENDIAN(multi.file_offset));
    //     header = LOAD_STRUCTURE(t_mach_o_header);
    // }

    // for (size_t i = 0; i < header.number_of_load_commands; i++)
    // {
    //     t_mach_o_load loaded = LOAD_STRUCTURE(t_mach_o_load);

    //     // printf("%.2x %.2x\n", loaded.type, loaded.size);

    //     switch (loaded.type)
    //     {
    //     case 0x02:
    //         handle_symbol_table();
    //         break;
        
    //     default:
    //         break;
    //     }
    //     if (loaded.size == 0)
    //         break;
    //     if (!read_bytes(loaded.size - sizeof(t_mach_o_load))) break;
    // }
    
}
