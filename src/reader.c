#include <ft_nm.h>

t_reader mapped;
char    *filename;

void move_to_offset(size_t offset)
{
    mapped.cur_offset = offset;
}

void set_base_offset(size_t offset)
{
    mapped.base_offset = offset;
    mapped.cur_offset = 0;
}

size_t get_offset()
{
    return mapped.cur_offset;
}

void *read_bytes(size_t amount, int *error)
{
    char *rv = mapped.ptr + mapped.cur_offset + mapped.base_offset;

    mapped.cur_offset += amount;

    if (mapped.cur_offset > mapped.size) {
        printf("bfd plugin: %s: file too short\n", filename);
        dprintf(2, "ft_nm: error: out of mapped size\n");
        *error = 1;
        return 0;
    }

    return rv;
}

int open_file(char *file)
{
    int fd = open(file, O_RDONLY);
    filename = file;

    struct stat s;

    if (fstat(fd, &s) == -1) {
        dprintf(2, "ft_nm: '%s': No such file\n", file);
        return -1;
    }
    if (s.st_size < 0x40) {
        dprintf(2, "ft_nm: '%s': Invalid header\n", file);
        return -1;
    }

    mapped.ptr = mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    mapped.cur_offset = 0;
    mapped.base_offset = 0;
    mapped.size = s.st_size;

    if (mapped.ptr == (void *)-1)
        return -1;
    return 0;
}

void    print_reader_error()
{
    dprintf(2, "ft_nm: error: request went past the end of file\n");
}

void close_file()
{
    munmap(mapped.ptr, mapped.size);
}

