#ifndef EDITOR_H
# define EDITOR_H

# define _DEFAULT_SOURCE
# define _BSD_SOURCE
# define _GNU_SOURCE

# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <stdbool.h>
# include <ctype.h>
# include <err.h>
# include <errno.h>
# include <sys/ioctl.h>
# include <sys/types.h>
# include <stddef.h>
# include <dirent.h>
# include <pwd.h>

# define E_VERSION "0.0.1"

# define E_DEF_STATUS_MSG "HELP: Ctrl-S = save | Ctrl-Q = quit " \
	"| Ctrl-F = find | Ctrl-O = open"

# include "data.h"

/* abuff.c */
# include "abuff.h"

/* raw_mode.c */
void	raw_mode_enable(void);
void	raw_mode_disable(void);

/* operations.c */
void	op_insert_char(int c);
void	op_insert_new_line(void);
void	op_del_char(void);
void	op_find(void);
void	op_open_file(void);

/* input.c */
char	*input_prompt(char *prompt, void (*callback)(char*, size_t*, int))
			__nonnull((1));
int		input_key_read(void);
void	input_key_process(void);

/* terminal_win_state.c */
bool	terminal_get_win_size(int *rows, int *cols) __nonnull((1,2));

/* drawing.c */
void	draw_refresh_screen(void);

/* drawing_status_bar.c */
void	set_status_msg(const char *fmt, ...) __nonnull((1));
void	draw_status_bar(struct s_abuff *ab) __nonnull((1));
void	draw_msg_bar(struct s_abuff *ab) __nonnull((1));

/* row.c */
int		row_cx_to_rx(e_row *row, int cx) __nonnull((1));
int		row_rx_to_cx(e_row *row, int rx) __nonnull((1));

void	row_update(e_row *row) __nonnull((1));
void	row_insert(int at, char *s, size_t len) __nonnull((2));
void	row_append_string(e_row *row, char *s, size_t len)
			__nonnull((1,2));
void	row_ch_insert(e_row *row, int at, int c) __nonnull((1));
void	row_ch_del(e_row *row, int at) __nonnull((1));
void	row_free(e_row *row) __nonnull((1));
void	row_del(int at);

/* file_io.c */
void	io_open(char *filename) __nonnull((1));
void	io_save(void);

/* syntax_highlighting.c */
# include "syntax.h"

void	syntax_update(e_row *row) __nonnull((1));
int		syntax_to_clr(int hl);
void	syntax_select_hl(void);

/* flags.c */
# include "flags.h"

void	parse_flags(int argc, char *argv[]);

/* free.c */
void	free_g_editor(void);

#endif /* EDITOR_H */
