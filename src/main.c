#include <ft_nm.h>

void print_char(int fd)
{
    char c = 0;

    read(fd, &c, sizeof(char));
    printf("\t%.2x\n", c);
}

void print_short(int fd)
{
    short t = 0;

    read(fd, &t, sizeof(short));
    printf("%10u\t", t);
    for (size_t i = 0; i < 2; i++)
    {
        int target = (t & 0xFF00) >> 8;
        printf("%.2x ", target);
        t = t << 8;
    }
    printf("\n");
}

void print_int(int fd)
{
    unsigned t = 0;

    read(fd, &t, sizeof(unsigned));
    printf("%10u\t", t);
    for (size_t i = 0; i < 4; i++)
    {
        int target = (t & 0xFF000000) >> 24;
        printf("%.2x ", target);
        t = t << 8;
    }
    printf("\n");
}

void print_long(int fd)
{
    size_t t = 0;

    read(fd, &t, sizeof(size_t));
    printf("%10zu\t", t);
    for (size_t i = 0; i < 8; i++)
    {
        int target = (t & 0xFF00000000000000) >> 56;
        printf("%.2x ", target);
        t = t << 8;
    }
    printf("\n");
}

void print_string(int fd, size_t len)
{
    char str[4096];

    read(fd, str, len);
    printf("%s\t", str);
    for (size_t i = 0; i < len; i++)
    {
        printf("%.2x ", str[i]);
    }
    printf("\n");

}

void print_address(int fd)
{
    size_t addr;

    read(fd, &addr, sizeof(size_t));

    printf("%p\n", (void *)addr);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    int fd = open(argv[1], O_RDONLY);
    if (!fd)
        return 2;
    
    printf("\n== new header ==\n\n`");

    for (size_t i = 0; i < 8; i++)
    {
        print_int(fd);
    }

    printf("\n== new segment ==\n\n`");


    for (size_t i = 0; i < 1; i++)
    {
        print_int(fd);
        print_int(fd);
        print_string(fd, 16);
        print_address(fd);
        print_long(fd);
        print_long(fd);
        print_long(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
    }

    for (size_t i = 0; i < 4; i++)
    {
        printf("\n == new section ==\n\n");
        print_string(fd, 16);
        print_string(fd, 16);
        print_address(fd);
        print_long(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);
        print_int(fd);        
    }

    printf("\n == new OS Def ==\n\n");

    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);

    printf("\n == new symbol_table ==\n\n");

    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);

    printf("\n == new symbol_table_info ==\n\n");

    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);
    print_int(fd);

    int fd2 = open("main.o", O_RDONLY);

    char skip[4096];
    read(fd2, skip, 872);


    for (size_t i = 0; i < 3; i++)
    {
        printf("\n == new symbol_table_load ==\n\n");
        print_int(fd2);
        print_char(fd2);
        print_char(fd2);
        print_short(fd2);
        print_long(fd2);
    }
    




    // read(fd2, skip, 920);

    // int ret = 1;
    // while (ret)
    // {
    //     char s = 0;
    //     ret = read(fd2, &s, 1);
    //     if (ret && s)
    //         printf("%c", s);
    // }
    // printf("\n");
    

    close(fd);

}