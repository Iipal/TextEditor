#include "editor.h"

void		io_open(char *filename)
{
	free(g_editor.filename);
	g_editor.filename = strdup(filename);

	syntax_select_hl();

	FILE	*fp = fopen(filename, "r");

	if (!fp)
		die("fopen");

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

static char	*io_rows_to_string(int *buff_len)
{
	int	total_len = 0;
	int	j;

	for (j = 0; g_editor.num_rows > j; j++)
		total_len += g_editor.row[j].size + 1;
	*buff_len = total_len;

	char	*buff = malloc(total_len);
	char	*ptr = buff;

	for (j = 0; g_editor.num_rows > j; j++) {
		memcpy(ptr, g_editor.row[j].chars, g_editor.row[j].size);
		ptr += g_editor.row[j].size;
		*ptr++ = '\n';
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

	int		len;
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
