#include "editor.h"

int			row_cx_to_rx(e_row *row, int cx)
{
	int	rx = 0;

	for (int j = 0; cx > j; j++) {
		if ('\t' == row->chars[j])
			rx += (E_TAB_STOP - 1) - (rx % E_TAB_STOP);
		rx++;
	}
	return rx;
}

int			row_rx_to_cx(e_row *row, int rx)
{
	int	curr_rx = 0;
	int	cx;

	for (cx = 0; row->size > cx; cx++) {
		if ('\t' == row->chars[cx])
			curr_rx += (E_TAB_STOP - 1) - (curr_rx % E_TAB_STOP);
		curr_rx++;
		if (rx < curr_rx)
			return cx;
	}
	return cx;
}

void		row_update(e_row *row)
{
	int	tabs = 0;

	for (int j = 0; row->size > j; j++)
		if ('\t' == row->chars[j])
			++tabs;
	free(row->render);
	row->render = malloc(row->size + tabs * (E_TAB_STOP - 1) + 1);

	int	i = 0;

	for (int j = 0; row->size > j; j++)
		if ('\t' == row->chars[j]) {
			row->render[i++] = ' ';
			while (i % E_TAB_STOP)
				row->render[i++] = ' ';
		} else {
			row->render[i++] = row->chars[j];
		}
	row->render[i] = '\0';
	row->r_size = i;
	syntax_update(row);
}

void		row_insert(int at, char *s, size_t len)
{
	if (0 > at || g_editor.num_rows < at)
		return ;
	g_editor.row = realloc(g_editor.row,
					sizeof(e_row) * (g_editor.num_rows + 1));
	memmove(&g_editor.row[at + 1], &g_editor.row[at],
		sizeof(e_row) * (g_editor.num_rows - at));
	for (int j = at + 1; g_editor.num_rows >= j; j++)
		++g_editor.row[j].idx;
	g_editor.row[at] = (e_row){at, len, 0, malloc(len + 1), NULL, NULL, 0};
	memcpy(g_editor.row[at].chars, s, len);
	g_editor.row[at].chars[len] = '\0';
	row_update(&g_editor.row[at]);
	++g_editor.num_rows;
	++g_editor.dirty;
}

void		row_free(e_row *row)
{
	free(row->render);
	free(row->chars);
	free(row->hl);
}

void		row_del(int at)
{
	if (0 > at || g_editor.num_rows <= at)
		return;
	row_free(&g_editor.row[at]);
	memmove(&g_editor.row[at], &g_editor.row[at + 1],
		sizeof(e_row) * (g_editor.num_rows - at - 1));
	for (int j = at; g_editor.num_rows - 1 > j; j++)
		--g_editor.row[j].idx;
	--g_editor.num_rows;
	++g_editor.dirty;
}

void		row_ch_insert(e_row *row, int at, int c)
{
	if (0 > at || row->size < at)
		at = row->size;
	row->chars = realloc(row->chars, row->size + 2);
	memmove(&row->chars[at + 1], &row->chars[at], row->size++ - at + 1);
	row->chars[at] = c;
	row_update(row);
	++g_editor.dirty;
}

void		row_ch_del(e_row *row, int at)
{
	if (0 > at || row->size <= at)
		return ;
	memmove(&row->chars[at], &row->chars[at + 1], row->size-- - at);
	row_update(row);
	++g_editor.dirty;
}

void		row_append_string(e_row *row, char *s, size_t len)
{
	row->chars = realloc(row->chars, row->size + len + 1);
	memcpy(&row->chars[row->size], s, len);
	row->size += len;
	row->chars[row->size] = '\0';
	row_update(row);
	++g_editor.dirty;
}
