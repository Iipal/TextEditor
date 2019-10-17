#include "editor.h"

int	g_flags = 0;

static char	*g_valid_flags[MAX_FLAGS + 1] = {
	F_TRIM_END_WHITESPACES, NULL
};

static bool	check_valid_flag(char *flag)
{
	for (size_t i = 0; g_valid_flags[i]; i++)
		if (!strncmp(flag, g_valid_flags[i], strlen(g_valid_flags[i]))) {
			BIT_SET(g_flags, BIT_TO_N(i));
			return true;
		}
	return false;
}

void	parse_flags_in_args(int argc, char *argv[])
{
	if (0 >= (argc - 1))
		return ;
	for (int i = 0; argc - 1 > i; i++) {
		if ('-' == argv[i][0]) {
			if (!check_valid_flag(argv[i] + 1)) {
				errx(1, "\'%s\' is an invalid flag.\r", argv[i]);
			}
		}
	}
}
