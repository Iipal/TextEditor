#include "editor.h"

void	raw_mode_enable(void)
{
	if (-1 == tcgetattr(STDIN_FILENO, &g_editor.raw_termios))
		die("tcgetattr");
	atexit(raw_mode_disable);

	struct termios	termios = g_editor.raw_termios;

	termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	termios.c_oflag &= ~(OPOST);
	termios.c_cflag |= (CS8);
	termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	termios.c_cc[VMIN] = 0;
	termios.c_cc[VTIME] = 1;
	if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios))
		die("tcsetattr");
}

void	raw_mode_disable(void)
{
	if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_editor.raw_termios))
		die("tcsetattr");
}
