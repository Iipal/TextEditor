#include "editor.h"

char	*input_prompt(char *prompt, void (*callback)(char*, int))
{
	size_t	buff_size = 128UL;
	char	*buff = malloc(buff_size);
	size_t	buff_len = 0;

	buff[0] = '\0';
	while (1) {
		set_status_msg(prompt, buff);
		draw_refresh_screen();

		int c = input_key_read();
		if (DEL_KEY == c || CTRL_KEY('h') == c || BACKSPACE == c) {
			if (buff_len)
				buff[--buff_len] = '\0';
		} else if ('\x1b' == c) {
			set_status_msg("");
			if (callback)
				callback(buff, c);
			free(buff);
			return NULL;
		} else if ('\r' == c) {
			if (buff_len) {
				set_status_msg("");
				if (callback)
					callback(buff, c);
				return buff;
			}
		} else if (!iscntrl(c) && 128 > c) {
			if (buff_len == buff_size - 1) {
				buff_size *= 2;
				buff = realloc(buff, buff_size);
			}
			buff[buff_len++] = c;
			buff[buff_len] = '\0';
		}
		if (callback)
			callback(buff, c);
	}
}

int		input_key_read(void)
{
	int		nread;
	char	c;

	while (1 != (nread = read(STDIN_FILENO, &c, 1)))
		if (-1 == nread && EAGAIN != errno)
			err(1, "read");
	if ('\x1b' == c) {
		char	seq[3];

		if (1 != read(STDIN_FILENO, &seq[0], 1))
			return '\x1b';
		if (1 != read(STDIN_FILENO, &seq[1], 1))
			return '\x1b';
		if ('[' == seq[0]) {
			if ('0' <= seq[1] && '9' >= seq[1]) {
				if (1 != read(STDIN_FILENO, &seq[2], 1))
					return '\x1b';
				if ('~' == seq[2])
					switch(seq[1]) {
						case '1': return HOME_KEY;
						case '3': return DEL_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
			} else {
				switch(seq[1]) {
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				}
			}
		} else if ('O' == seq[0]) {
			switch (seq[1]) {
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}
		return '\x1b';
	}
	return c;
}

static inline void	input_move_cursor(int key)
{
	e_row	*r = (g_editor.num_rows <= g_editor.cy)
				? NULL : &g_editor.row[g_editor.cy];

	switch(key) {
		case ARROW_LEFT:
			if (g_editor.cx)
				--g_editor.cx;
			else if (0 < g_editor.cy)
				g_editor.cx = g_editor.row[--g_editor.cy].size;
			break;
		case ARROW_RIGHT:
			if (r && r->size > g_editor.cx) {
				++g_editor.cx;
			} else if (r && r->size == g_editor.cx) {
				++g_editor.cy;
				g_editor.cx = 0;
			}
			break;
		case ARROW_UP:
			if (g_editor.cy)
				--g_editor.cy;
			break;
		case ARROW_DOWN:
			if (g_editor.num_rows > g_editor.cy)
				++g_editor.cy;
			break;
	}
	r = (g_editor.num_rows <= g_editor.cy)
		? NULL : &g_editor.row[g_editor.cy];

	int r_len = r ? r->size : 0;

	if (r_len < g_editor.cx)
		g_editor.cx = r_len;
}

void	input_key_process(void)
{
	static int	quit_times = E_QUIT_TIMES;
	int			c = input_key_read();

	switch(c) {
		case '\r': editor_operations_insert_new_line(); break;

		case CTRL_KEY('q'):
			if (g_editor.dirty && 0 < quit_times) {
				set_status_msg("WARNING!!! File has unsaved changes. "
					"Press Ctrl-Q %d more times to quit.", quit_times--);
				return;
			}
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;

		case CTRL_KEY('s'): io_save(); break;

		case HOME_KEY: g_editor.cx = 0; break;
		case END_KEY:
			if (g_editor.num_rows > g_editor.cy)
				g_editor.cx = g_editor.row[g_editor.cy].size;
			break;

		case CTRL_KEY('f'): editor_operations_find(); break ;

		case BACKSPACE:
		case CTRL_KEY('h'):
		case DEL_KEY:
			if (DEL_KEY == c)
				input_move_cursor(ARROW_RIGHT);
			editor_operations_del_char();
			break;

		case PAGE_UP:
		case PAGE_DOWN: {
			if (PAGE_UP == c) {
				g_editor.cy = g_editor.row_off;
			} else if (PAGE_DOWN == c) {
				g_editor.cy = g_editor.row_off + g_editor.screen_rows - 1;
				if (g_editor.num_rows < g_editor.cy)
					g_editor.cy = g_editor.num_rows;
			}
			int	times = g_editor.screen_rows;
			while (times--)
				input_move_cursor((PAGE_UP == c) ? ARROW_UP : ARROW_DOWN);
		} break;

		case ARROW_UP:
		case ARROW_LEFT:
		case ARROW_DOWN:
		case ARROW_RIGHT: input_move_cursor(c); break;

		case CTRL_KEY('l'):
		case '\x1b':
			break ;

		default: editor_operations_insert_char(c); break;
	}
	quit_times = E_QUIT_TIMES;
}
