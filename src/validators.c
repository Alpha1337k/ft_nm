#include <ft_nm.h>

int validate_header(t_elf_header header, char *filename)
{
	VALIDATE_ASSERTION(header.magic_number == 1179403647, "1file format not recognized");
	VALIDATE_ASSERTION(header.format_bits == 2 || header.format_bits == 1, "2file format not recognized");
	VALIDATE_ASSERTION(header.elf_version == 1 && header.elf_version2 == 1, "4file format not recognized");
	VALIDATE_ASSERTION(header.architecture == 0x03 || header.architecture == 0x3E , "5file format not recognized");



	return 0;
}