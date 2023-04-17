#include "ft_nm.h"

int		ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t			x;
	unsigned char	*s;
	unsigned char	*d;

	s = (unsigned char *)s1;
	d = (unsigned char *)s2;
	if (n == 0)
		return (0);
	if (s[0] == '\0' && d[0] == '\0')
		return (0);
	x = 0;
	while (x < n)
	{
		if (s[x] != 0 && d[x] == 0)
			return (1);
		if (s[x] == 0 && d[x] != 0)
			return (-1);
		if (s[x] != d[x])
			return ((int)(s[x] - d[x]));
		if (s[x] == 0 && d[x] == 0)
			return (0);
		x++;
	}
	return (0);
}

int		ft_strcmp(const char *s1, const char *s2)
{
	size_t			x;
	unsigned char	*s;
	unsigned char	*d;

	s = (unsigned char *)s1;
	d = (unsigned char *)s2;
	if (s[0] == '\0' && d[0] == '\0')
		return (0);
	x = 0;
	while (1)
	{
		if (s[x] != 0 && d[x] == 0)
			return (1);
		if (s[x] == 0 && d[x] != 0)
			return (-1);
		if (s[x] != d[x])
			return ((int)(s[x] - d[x]));
		if (s[x] == 0 && d[x] == 0)
			return (0);
		x++;
	}
	return (0);
}

void ft_puts(int fd, char *str)
{
	while (str && *str)
	{
		write(fd, str, 1);
		str++;
	}
	
}

void	print_result_addr(u_int8_t padding, u_int64_t value, char type, char *name)
{
	char *val = malloc(17);
	if (!val) return;

	for (size_t i = 0; i < 16; i++)
	{
		val[i] = '0';
	}
	val[16] = 0;
	
	const char *based = "0123456789abcdefgh";
	int idx = 15;
	while (value && idx >= 0)
	{
		val[idx] = based[value % 16];
		value /= 16;
		idx--;
	}
	if (padding == 16)
		ft_puts(1, val);
	else
		ft_puts(1, &val[8]);
	write(1, " ", 1);
	write(1, &type, 1);
	write(1, " ", 1);
	ft_puts(1, name);
	write(1, "\n", 1);

	free(val);
}

void	print_result_empt(u_int8_t padding, char type, char *name)
{
	for (u_int8_t i = 0; i < padding + 1; i++)
	{
		write(1, " ", 1);
	}
	write(1, &type, 1);
	write(1, " ", 1);
	ft_puts(1, name);
	write(1, "\n", 1);
}


void	*ft_memset(void *b, int c, size_t n)
{
	size_t			x;
	unsigned char	*ed;
	unsigned char	ch;

	ch = (unsigned char)c;
	ed = (unsigned char *)b;
	x = 0;
	while (x < n)
	{
		ed[x] = ch;
		x++;
	}
	return (b);
}
