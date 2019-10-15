#include "editor.h"

static inline void	draw_scroll(void)
{
	g_editor.rx = 0;
	if (g_editor.num_rows > g_editor.cy)
		g_editor.rx = row_cx_to_rx(&g_editor.row[g_editor.cy], g_editor.cx);

	if (g_editor.row_off > g_editor.cy)
		g_editor.row_off = g_editor.cy;
	if (g_editor.row_off + g_editor.screen_rows <= g_editor.cy)
		g_editor.row_off = g_editor.cy - g_editor.screen_rows + 1;
	if (g_editor.col_off > g_editor.rx)
		g_editor.col_off = g_editor.rx;
	if (g_editor.col_off + g_editor.screen_cols <= g_editor.rx)
		g_editor.col_off = g_editor.rx - g_editor.screen_cols + 1;
}

static void			draw_rows(struct s_abuff *ab)
{
	for (int y = 0; g_editor.screen_rows > y; y++) {
		int	file_row = y + g_editor.row_off;
		if (g_editor.num_rows <= file_row) {
			if (!g_editor.num_rows && g_editor.screen_rows / 3 == y) {
				char	welcome[80];
				int		welcome_len = snprintf(welcome, sizeof(welcome),
								"my editor -- version %s", E_VERSION);

				if (g_editor.screen_cols < welcome_len)
					welcome_len = g_editor.screen_cols;

				int padding = (g_editor.screen_cols - welcome_len) / 2;

				if (padding) {
					ab_append(ab, "~", 1);
					--padding;
				}
				while (padding--)
					ab_append(ab, " ", 1);
				ab_append(ab, welcome, welcome_len);
			} else {
				ab_append(ab, "~", 1);
			}
		} else {
			int	len = g_editor.row[file_row].r_size - g_editor.col_off;

			if (0 > len)
				len = 0;
			if (len > g_editor.screen_cols)
				len = g_editor.screen_cols;

			char *c = &g_editor.row[file_row].render[g_editor.col_off];
			unsigned char *hl = &g_editor.row[file_row].hl[g_editor.col_off];
			int curr_clr = -1;

			for (int j = 0; len > j; j++) {
				if (iscntrl(c[j])) {
					char	sym = (c[j] <= 26) ? '@' + c[j] : '?';
					ab_append(ab, "\x1b[7m", 4);
					ab_append(ab, &sym, 1);
					ab_append(ab, "\x1b[m", 3);
					if (-1 != curr_clr) {
						char	buff[16];
						int		c_len = snprintf(buff, sizeof(buff),
											"\x1b[%dm", curr_clr);

						ab_append(ab, buff, c_len);
					}
				} else if (hl[j] == HL_NORMAL) {
					if (-1 != curr_clr) {
						ab_append(ab, "\x1b[39m", 5);
						curr_clr = -1;
					}
					ab_append(ab, &c[j], 1);
				} else {
					int	color = syntax_to_clr(hl[j]);
					if (color != curr_clr) {
						curr_clr = color;
						char	buff[16];
						int		c_len = snprintf(buff, sizeof(buff),
										"\x1b[%dm", color);
						ab_append(ab, buff, c_len);
					}
					ab_append(ab, &c[j], 1);
				}
			}
			ab_append(ab, "\x1b[39m", 5);
		}
		ab_append(ab, "\x1b[K", 3);
		ab_append(ab, "\r\n", 2);
	}
}

static void			draw_status_bar(struct s_abuff *ab)
{
	ab_append(ab, "\x1b[7m", 4);

	char	status[80];
	char	r_status[80];
	int		len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
					g_editor.filename ? g_editor.filename : "[no name]",
					g_editor.num_rows, g_editor.dirty ? "(modified)" : "");
	int		r_len = snprintf(r_status, sizeof(r_status), "%s %d/%d",
					g_editor.syntax ? g_editor.syntax->filetype : "no ft",
					g_editor.cy + 1, g_editor.num_rows);

	if (g_editor.screen_cols < len)
		len = g_editor.screen_cols;
	ab_append(ab, status, len);
	while (g_editor.screen_cols > len)
		if (r_len == g_editor.screen_cols - len) {
			ab_append(ab, r_status, r_len);
			break ;
		} else {
			ab_append(ab, " ", 1);
			++len;
		}
	ab_append(ab, "\x1b[m", 3);
	ab_append(ab, "\r\n", 2);
}

static inline void	draw_msg_bar(struct s_abuff *ab)
{
	int	msg_len = strlen(g_editor.status_msg);

	ab_append(ab, "\x1b[K", 3);
	if (g_editor.screen_cols < msg_len)
		msg_len = g_editor.screen_cols;
	if (msg_len && 5 > g_editor.status_msg_time - time(NULL))
		ab_append(ab, g_editor.status_msg, msg_len);
}

void				draw_refresh_screen(void)
{
	draw_scroll();

	struct s_abuff	ab = S_ABUFF_INIT;

	ab_append(&ab, "\x1b[?25l", 6);
	ab_append(&ab, "\x1b[H", 3);
	draw_rows(&ab);
	draw_status_bar(&ab);
	draw_msg_bar(&ab);

	char	buff[32];

	snprintf(buff, sizeof(buff), "\x1b[%d;%dH",
		(g_editor.cy - g_editor.row_off) + 1,
		(g_editor.rx - g_editor.col_off) + 1);
	ab_append(&ab, buff, strlen(buff));

	ab_append(&ab, "\x1b[?25h", 6);
	write(STDOUT_FILENO, ab.b, ab.len);
	ab_free(&ab);
}
