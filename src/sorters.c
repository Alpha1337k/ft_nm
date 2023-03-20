#include <ft_nm.h>

int normal_sort(char *a, char *b)
{
	return strcmp(a, b) > 0;
}

int reverse_sort(char *a, char *b)
{
	return strcmp(a, b) < 0;
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
	return isupper(type) || type == 'w';
}