#include <ft_nm.h>

t_ft_nm parse_options(size_t target_count, char **targets)
{
    t_ft_nm options;

    for (size_t i = 0; i < target_count; i++)
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
                    ft_puts(2, "ft_nm: '");
                    write(2, &targets[i][x], 1);
                    ft_puts(2, "' is an invalid paramter\n");
                    exit(1);
                }
            }
            
        }
    }
    return options;
}

int main(int argc, char **argv)
{
    char *fallback[1] = {"a.out"};
    char **targets;
    size_t target_count = 1;

    if (argc < 2)
        targets = fallback;
    else {
        targets = &argv[1];
        target_count = argc - 1;
    }
    t_ft_nm options = parse_options(target_count, targets);

    for (size_t i = 0; i < target_count; i++)
    {
        if (targets[i][0] == '-')
            continue;
        if (open_file(targets[i]) == -1) {
            continue;
        }
        int error = 0;
        t_elf_header h = *LOAD_STRUCTURE(t_elf_header);
        if (error) {
            print_reader_error();
            continue;
        }
        options.is_64_bit = h.format_bits == 2;
        if (validate_header(h, targets[i]) == 0)
            handle_elf(options, targets[i]);
        close_file();
    }

    return 0;    
}
