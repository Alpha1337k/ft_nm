#include <ft_nm.h>

int normal_sort(char *a, char *b)
{
	return strcmp(a, b) > 0;
}

int reverse_sort(char *a, char *b)
{
	return strcmp(b, a) > 0;
}

int no_sort(char *a, char *b)
{
	(void)a;
	(void)b;
	return 0;
}

int normal_filter(char type)
{
	(void)type;
	return 1;
}

int undefined_filter(char type)
{
	char finds[] = "Uw";
	return strchr(finds, type) != 0;
}

int external_filter(char type)
{
	return (isupper(type) && type != 'N') || type == 'w';
}

int combined_filters(char type)
{
	return undefined_filter(type) && external_filter(type);
}

t_elf_symbol_wrap *elf_sort_symbol_table(t_elf_symbol_wrap *entries, size_t len, int (*f)(char *, char *))
{
    int sorted = 0;
    while (!sorted)
    {
        sorted = 1;
        for (size_t i = 0; i < len - 1; i++)
        {
            if (f(entries[i].name, entries[i + 1].name) > 0)
            {
                t_elf_symbol_wrap tmp = entries[i];
                entries[i] = entries[i + 1];
                entries[i + 1] = tmp;
                sorted = 0;
            }
        }
    }
    return entries;    
}