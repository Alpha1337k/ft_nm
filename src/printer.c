#include <ft_nm.h>

void print_char(u_int8_t target)
{
    char c = target;

    printf("\t%.2x\t %x\n", c, c);
}

void print_short(short t)
{
    printf("%10hu\t", t);
    for (size_t i = 0; i < 2; i++)
    {
        short target = (t & 0xFF00) >> 8;
        printf("%.2x ", target);
        t = t << 8;
    }
    printf("\n");
}

void print_int(int t)
{
    printf("%10u\t", t);
    for (size_t i = 0; i < 4; i++)
    {
        int target = (t & 0xFF000000) >> 24;
        printf("%.2x ", target);
        t = t << 8;
    }
    printf("\n");
}

void print_long(long t)
{
    printf("%10zu\t", t);
    for (size_t i = 0; i < 8; i++)
    {
        int target = (t & 0xFF00000000000000) >> 56;
        printf("%.2x ", target);
        t = t << 8;
    }
    printf("\n");
}

void print_string(u_int8_t *s, size_t len)
{
    char str[len + 1];

    memcpy(str, s, len);
    printf("%s\t", str);
    for (size_t i = 0; i < len; i++)
    {
        printf("%.2x ", str[i]);
    }
    printf("\n");

}

void print_address(long addr)
{
    printf("%p\n", (void *)addr);
}