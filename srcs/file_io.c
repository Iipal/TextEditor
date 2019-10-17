#include "editor.h"

void		io_open(char *filename)
{
	free(g_editor.filename);
	g_editor.filename = strdup(filename);

	syntax_select_hl();

	FILE	*fp = fopen(filename, "r");

	if (!fp)
		err(1, "fopen [\'%s\']", filename);

	char	*line = NULL;
	size_t	line_cap = 0UL;
	ssize_t line_len;

	while (-1 != (line_len = getline(&line, &line_cap, fp))) {
		while (0 < line_len
		&& (line[line_len - 1] == '\n' || line[line_len - 1] == '\r'))
			--line_len;
		row_insert(g_editor.num_rows, line, line_len);
	}
	free(line);
	fclose(fp);
	g_editor.dirty = 0;
}

static int	io_row_len(int j)
{
	if (IS_BIT(g_flags.b_mask, F_B_TEW)) {
		char const *const	s = g_editor.row[j].chars;
		int					end = g_editor.row[j].size;

		while (end && s[end] && (isblank(s[end]) || isspace(s[end])))
			--end;
		return end;
	}
	return g_editor.row[j].size;
}

static char	*io_rows_to_string(int *buff_len)
{
	u_int8_t const	eof_bytes = IS_BIT(g_flags.b_mask, F_B_CRLF) ? 2 : 1;
	size_t			total_len = 0;

	for (int j = 0; g_editor.num_rows > j; j++)
		total_len += io_row_len(j) + eof_bytes;
	*buff_len = total_len;

	char	*buff = malloc(total_len);
	char	*ptr = buff;

	for (int j = 0; g_editor.num_rows > j; j++) {
		int const	row_len = io_row_len(j);

		memcpy(ptr, g_editor.row[j].chars, row_len);
		ptr += row_len;
		if (IS_BIT(g_flags.b_mask, F_B_CRLF)) {
			*ptr++ = '\r';
			*ptr++ = '\n';
		} else {
			*ptr++ = '\n';
		}
	}
	return buff;
}

void		io_save(void)
{
	if (!g_editor.filename) {
		g_editor.filename =
			input_prompt("Save as: %s (ESC to cancel)", NULL);
		if (!g_editor.filename) {
			set_status_msg("Save cancled");
			return ;
		}
		syntax_select_hl();
	}

	int		len = 0;
	char	*buff = io_rows_to_string(&len);
	int		fd = open(g_editor.filename, O_RDWR | O_CREAT, 0644);

	if (-1 != fd) {
		if (-1 != ftruncate(fd, len))
			if (len == write(fd, buff, len)) {
				close(fd);
				free(buff);
				g_editor.dirty = 0;
				set_status_msg("%d bytes written to disk", len);
				return ;
			}
		close(fd);
	}
	free(buff);
	set_status_msg("Can't save! I/O error: %s", strerror(errno));
}
