#include "editor.h"

void		op_insert_char(int c)
{
	if (g_editor.num_rows == g_editor.cy)
		row_insert(g_editor.num_rows, "", 0);
	row_ch_insert(&g_editor.row[g_editor.cy], g_editor.cx++, c);
}

void		op_insert_new_line(void)
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

void		op_del_char(void)
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

/*
**
** Find string in file functions:
**
*/
static void	op_find_callback(char *query, size_t *query_len, int key)
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
	(void)query_len;
}

void		op_find(void)
{
	int const	saved_cx = g_editor.cx;
	int const	saved_cy = g_editor.cy;
	int const	saved_col_off = g_editor.col_off;
	int const	saved_row_off = g_editor.row_off;

	char	*query = input_prompt("Search: %s (Use ESC/Arrows/Enter)",
						op_find_callback);

	if (query) {
		free(query);
	} else {
		g_editor.cx = saved_cx;
		g_editor.cy = saved_cy;
		g_editor.col_off = saved_col_off;
		g_editor.row_off = saved_row_off;
	}
}

/*
**
** Open file functions:
**
*/
typedef struct	s_matches_helper {
	char	*filename;
	bool	match;
}				t_mh;

static char	*op_of_callback_full_path(char *path, size_t *path_len)
{
	if (path) {
		if ('~' != *path)
			return path;

		char			*path_dup = strdup(path);
		struct passwd	*pw = getpwuid(getuid());

		if (!pw)
			err(1, "getpwuid(getuid())");
		if (path_len)
			*path_len += strlen(pw->pw_dir);
		strcpy(path, pw->pw_dir);
		path[strlen(path)] = '/';
		strcpy(path + strlen(path), path_dup + 1);
		free(path_dup);
	}
	return path;
}

static void	*op_of_callback_free_matches(t_mh *matches,
							size_t *const matches_len)
{
	if (matches) {
		for (size_t i = 0UL; *matches_len > i; i++)
			free(matches[i].filename);
		free(matches);
	}
	return (void*)(*matches_len = 0UL);
}

static int op_of_callback_match_cmp(void const *_a, void const *_b)
{
	t_mh const	*a = _a;
	t_mh const	*b = _b;

	return b->match - a->match;
}

static void	op_open_file_callback(char *path, size_t *path_len, int key)
{
	static char		*last_to_match;
	static char		*last_dir_path;
	static t_mh		*matches;
	static size_t	matches_len;
	static size_t	at_match;
	static size_t	matches_founded;

	if ('\r' == key || '\x1b' == key) {
		matches = op_of_callback_free_matches(matches, &matches_len);
		if (last_dir_path)
			free(last_dir_path);
		return ;
	}
	if (!path || (path && !*path))
		return ;
	if ('~' == *path)
		path = op_of_callback_full_path(path, path_len);

	DIR				*dr = NULL;
	struct dirent	*de = NULL;
	char			*dir_path = NULL;
	char			*to_match = NULL;
	size_t			to_match_len = 0UL;

	if (!to_match) {
		to_match = strrchr(path, '/');
		if (!to_match) {
			matches = op_of_callback_free_matches(matches, &matches_len);
			last_to_match = NULL;
			return ;
		} else if (!*(++to_match)) {
			last_to_match = NULL;
			return ;
		}
	}
	to_match_len = strlen(to_match);
	dir_path = strndup(path, to_match - path);
	if (!to_match_len || !dir_path || (dir_path && !*dir_path))
		return ;
	if (!last_dir_path || (last_dir_path && strcmp(dir_path, last_dir_path))) {
		at_match = 0UL;
		matches = op_of_callback_free_matches(matches, &matches_len);
		dr = opendir(dir_path);
		if (!dr)
			return ;
		while ((de = readdir(dr))) {
			if (!matches)
				matches = calloc(matches_len + 1, sizeof(t_mh));
			else
				matches = realloc(matches, sizeof(t_mh) * (matches_len + 1));
			matches[matches_len].filename = strdup(de->d_name);
			matches[matches_len++].match = false;
		}
		closedir(dr);
	}
	if (matches_len && '\t' == key) {
		if (!last_to_match
		|| (last_to_match && strcmp(to_match, last_to_match))) {
			for (size_t i = 0UL; matches_len > i; i++) {
				size_t	match = 0UL;

				matches[i].match = false;
				if (strlen(matches[i].filename) < to_match_len)
					continue ;
				while (to_match[match] && matches[i].filename[match]
					&& to_match[match] == matches[i].filename[match])
					++match;
				if (match == to_match_len) {
					++matches_founded;
					matches[i].match = true;
				}
			}
			qsort(matches, matches_len, sizeof(t_mh), op_of_callback_match_cmp);
			at_match = 0UL;
			last_to_match = to_match;
		}
		if (matches_founded) {
			if (at_match == matches_founded || !matches[at_match].match)
				at_match = 0UL;
			strcpy(to_match, matches[at_match].filename);
			*path_len += strlen(matches[at_match++].filename) - to_match_len;
		}
	}
	last_dir_path = dir_path;
}

void		op_open_file(void)
{
	if (g_editor.dirty) {
		bool const	is_save = input_ask_question((char*[]){"yes", "y", NULL},
		NULL, "Do you want to save current \'%s\' file ?(Y/n)",
		g_editor.filename);

		if (is_save) {
			io_save();
			draw_refresh_screen();
			sleep(1);
		}
	}

	char	*path = input_prompt("Open file: %s", op_open_file_callback);

	if (path) {
		path = op_of_callback_full_path(path, NULL);
		if (g_editor.filename && !strcmp(g_editor.filename, path)) {
			set_status_msg("File \'%s\' already open.", path);
		} else {
			free_g_editor();
			io_open(path);
		}
		free(path);
	} else {
		set_status_msg("Open file canceled.");
	}
}
