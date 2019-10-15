#include "editor.h"

static inline void	editor_init(void)
{
	struct termios	tmp = g_editor.raw_termios;

	bzero(&g_editor, sizeof(struct s_editor_config));
	g_editor.raw_termios = tmp;
	if (!terminal_get_win_size(&g_editor.screen_rows,
								&g_editor.screen_cols))
		die("terminal_get_win_size");
	g_editor.screen_rows -= 2;
}

int					main(int argc, char *argv[])
{
	--argc; ++argv;

	raw_mode_enable();
	editor_init();
	if (argc)
		io_open(*argv);

	set_status_msg(E_DEF_STATUS_MSG);

	while (1) {
		draw_refresh_screen();
		input_key_process();
	}
}
