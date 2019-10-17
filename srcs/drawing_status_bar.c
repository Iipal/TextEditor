#include "editor.h"

void	set_status_msg(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_editor.status_msg, sizeof(g_editor.status_msg), fmt, ap);
	va_end(ap);
	g_editor.status_msg_time = time(NULL);
}

void	draw_status_bar(struct s_abuff *ab)
{
	ab_append(ab, "\x1b[7m", 4);

	char	l_status[80];
	char	r_status[80];
	int		l_len = snprintf(l_status, sizeof(l_status), "%.20s - %d lines %s",
					g_editor.filename ? g_editor.filename : "[no name]",
					g_editor.num_rows, g_editor.dirty ? "(modified)" : "");
	int		r_len = snprintf(r_status, sizeof(r_status), "%s %d/%d %s",
					g_editor.syntax ? g_editor.syntax->filetype : "no ft",
					g_editor.cy + 1, g_editor.num_rows,
					IS_BIT(g_flags.b_mask, F_B_CRLF) ? "crlf" : "lf");

	if (g_editor.screen_cols < l_len)
		l_len = g_editor.screen_cols;
	ab_append(ab, l_status, l_len);
	while (g_editor.screen_cols > l_len)
		if (r_len == g_editor.screen_cols - l_len) {
			ab_append(ab, r_status, r_len);
			break ;
		} else {
			ab_append(ab, " ", 1);
			++l_len;
		}
	ab_append(ab, "\x1b[m", 3);
	ab_append(ab, "\r\n", 2);
}

void	draw_msg_bar(struct s_abuff *ab)
{
	int	msg_len = strlen(g_editor.status_msg);

	ab_append(ab, "\x1b[K", 3);
	if (g_editor.screen_cols < msg_len)
		msg_len = g_editor.screen_cols;
	if (msg_len && 5 > g_editor.status_msg_time - time(NULL))
		ab_append(ab, g_editor.status_msg, msg_len);
}
