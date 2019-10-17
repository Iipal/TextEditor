#include "editor.h"

static inline void	editor_init(void)
{
	struct termios	tmp = g_editor.raw_termios;

	bzero(&g_editor, sizeof(struct s_editor_config));
	bzero(&g_flags, sizeof(struct s_flags));
	g_editor.raw_termios = tmp;
	if (!terminal_get_win_size(&g_editor.screen_rows,
								&g_editor.screen_cols))
		err(1, "terminal_get_win_size");
	g_editor.screen_rows -= 2;
}

int					main(int argc, char *argv[])
{
	raw_mode_enable();
	editor_init();

	--argc; ++argv;
	if (argc) {
		parse_flags(argc, argv);
		if (argc > g_flags.valid_flags)
			io_open(*(argv + g_flags.valid_flags));
	}

	set_status_msg(E_DEF_STATUS_MSG);

	while (1) {
		draw_refresh_screen();
		input_key_process();
	}
}
