#include "editor.h"

static char	*g_valid_flags[] = { F_TEW, F_CRLF, NULL };

static bool	check_valid_flag(char *flag)
{
	for (size_t i = 0; g_valid_flags[i]; i++)
		if (!strncmp(flag, g_valid_flags[i], strlen(g_valid_flags[i]))) {
			BIT_SET(g_flags.b_mask, BIT_TO_N(i));
			return true;
		}
	return false;
}

void	parse_flags(int argc, char *argv[])
{
	for (int i = 0; argc > i; i++) {
		if ('-' == argv[i][0]) {
			if (check_valid_flag(argv[i] + 1)) {
				++g_flags.valid_flags;
			} else {
				errx(1, "\'%s\' is an invalid flag.\r", argv[i]);
			}
		}
	}
}
