#include "editor.h"

void	free_g_editor(void)
{
	for (int i = 0; g_editor.num_rows > i; i++)
		row_free(&g_editor.row[i]);
	g_editor.row = realloc(g_editor.row, sizeof(e_row));
	g_editor.cx = g_editor.cy = g_editor.rx
		= g_editor.row_off = g_editor.col_off
		= g_editor.dirty = g_editor.num_rows = 0;
	set_status_msg("");
	g_editor.syntax = NULL;
	draw_refresh_screen();
}
