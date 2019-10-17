#include "editor.h"

void		editor_operations_insert_char(int c)
{
	if (g_editor.num_rows == g_editor.cy)
		row_insert(g_editor.num_rows, "", 0);
	row_ch_insert(&g_editor.row[g_editor.cy], g_editor.cx++, c);
}

void		editor_operations_insert_new_line(void)
{
	if (!g_editor.cx) {
		row_insert(g_editor.cy, "", 0);
	} else {
		e_row	*row = &g_editor.row[g_editor.cy];
		row_insert(g_editor.cy + 1, &row->chars[g_editor.cx],
			row->size - g_editor.cx);
		row = &g_editor.row[g_editor.cy];
		row->size = g_editor.cx;
		row->chars[row->size] = '\0';
		row_update(row);
	}
	++g_editor.cy;
	g_editor.cx = 0;
}

void		editor_operations_del_char(void)
{
	if (g_editor.num_rows == g_editor.cy)
		return ;
	if (!g_editor.cx && !g_editor.cy)
		return ;

	e_row	*row = &g_editor.row[g_editor.cy];

	if (0 < g_editor.cx) {
		row_ch_del(row, g_editor.cx-- - 1);
	} else {
		g_editor.cx = g_editor.row[g_editor.cy - 1].size;
		row_append_string(&g_editor.row[g_editor.cy - 1],
								row->chars, row->size);
		row_del(g_editor.cy--);
	}
}

static void	editor_operations_find_callback(char *query, int key)
{
	static int	last_match = -1;
	static int	direction = 1;
	static int	saved_hl_line;
	static char	*saved_hl = NULL;

	if (saved_hl) {
		memcpy(g_editor.row[saved_hl_line].hl, saved_hl,
			g_editor.row[saved_hl_line].r_size);
		free(saved_hl);
		saved_hl = NULL;
	}
	if ('\r' == key || '\x1b' == key) {
		last_match = -1;
		direction = 1;
		return ;
	} else if (ARROW_RIGHT == key || ARROW_DOWN == key) {
		direction = 1;
	} else if (ARROW_LEFT == key || ARROW_UP == key) {
		direction = -1;
	} else {
		last_match = -1;
		direction = 1;
	}
	if (-1 == last_match)
		direction = 1;

	int	current = last_match;

	for (int i = 0; g_editor.num_rows > i; i++) {
		current += direction;
		if (-1 == current)
			current = g_editor.num_rows - 1;
		else if (g_editor.num_rows == current)
			current = 0;

		e_row	*row = &g_editor.row[current];
		char	*match = strstr(row->render, query);

		if (match) {
			last_match = current;
			g_editor.cy = current;
			g_editor.cx = row_rx_to_cx(row, match - row->render);
			g_editor.row_off = g_editor.num_rows;
			saved_hl_line = current;
			saved_hl = malloc(row->r_size);
			memcpy(saved_hl, row->hl, row->r_size);
			memset(&row->hl[match - row->render], HL_MATCH, strlen(query));
			break ;
		}
	}
}

void		editor_operations_find(void)
{
	int const	saved_cx = g_editor.cx;
	int const	saved_cy = g_editor.cy;
	int const	saved_col_off = g_editor.col_off;
	int const	saved_row_off = g_editor.row_off;

	char	*query = input_prompt("Search: %s (Use ESC/Arrows/Enter)",
						editor_operations_find_callback);

	if (query) {
		free(query);
	} else {
		g_editor.cx = saved_cx;
		g_editor.cy = saved_cy;
		g_editor.col_off = saved_col_off;
		g_editor.row_off = saved_row_off;
	}
}
