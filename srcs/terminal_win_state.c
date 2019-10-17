#include "editor.h"

static bool	terminal_get_cursor_pos(int *rows, int *cols)
{
	char			buff[32];
	unsigned int	i = ~0U;

	if (4 != write(STDOUT_FILENO, "\1xb[6n", 4))
		return false;

	while (sizeof(buff) - 1 > ++i) {
		if (1 != read(STDIN_FILENO, &buff[i], 1))
			break ;
		if (buff[i] == 'R')
			break ;
	}
	buff[i] = '\0';
	if ('\x1b' != buff[0] || '[' != buff[1])
		return false;
	if (2 != sscanf(&buff[2], "%d;%d", rows, cols))
		return false;
	return true;
}

bool	terminal_get_win_size(int *rows, int *cols)
{
	struct winsize	ws;

	if (-1 == ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) || !ws.ws_col) {
		if (12 != write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12))
			return false;
		return terminal_get_cursor_pos(rows, cols);
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
	}
	return true;
}
