#include "editor.h"

inline void	op_insert_char(int c)
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

inline void	op_find(void)
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

static char	*op_full_path(char *path, size_t *path_len)
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

struct	s_matches {
	char	*filename;
	bool	match;
};

struct s_matches	*g_matches = NULL;
size_t				g_matches_len = 0UL;

size_t				g_matches_founded = 0UL;
size_t				g_at_match = 0UL;

char				*g_last_to_match = NULL;
char				*g_last_to_match_dir = NULL;

static void	*op_ofc_free_matches(void)
{
	if (g_matches) {
		for (size_t i = 0UL; g_matches_len > i; i++)
			free(g_matches[i].filename);
		free(g_matches);
	}
	return (void*)(g_matches_len = 0UL);
}

static int op_ofc_match_cmp(void const *a, void const *b)
{
	return ((struct s_matches const*)b)->match
		- ((struct s_matches const*)a)->match;
}

static void	op_open_file_callback(char *path, size_t *path_len, int key)
{
	if ('\r' == key || '\x1b' == key) {
		g_matches = op_ofc_free_matches();
		if (g_last_to_match)
			free(g_last_to_match);
		if (g_last_to_match_dir)
			free(g_last_to_match_dir);
		return ;
	}
	path = op_full_path(path, path_len);
	if (!path)
		return ;

	DIR				*dr = NULL;
	struct dirent	*de = NULL;
	char			*to_match = NULL;
	char			*to_match_dir = NULL;
	size_t			to_match_len = 0UL;

	to_match = strrchr(path, '/');
	if (!to_match) {
		to_match = path;
		to_match_dir = ".";
	} else {
		++to_match;
	}
	to_match_len = strlen(to_match);
	if (!to_match_dir && to_match - path)
		to_match_dir = strndup(path, to_match - path);
	else
		to_match_dir = ".";
	if (!g_last_to_match_dir || (g_last_to_match_dir
		&& strcmp(to_match_dir, g_last_to_match_dir))) {
		g_at_match = 0UL;
		g_matches = op_ofc_free_matches();
		dr = opendir(to_match_dir);
		if (!dr)
			return ;
		while ((de = readdir(dr))) {
			if (!g_matches)
				g_matches = calloc(g_matches_len + 1,
					sizeof(struct s_matches));
			else
				g_matches = realloc(g_matches,
					sizeof(struct s_matches) * (g_matches_len + 1));
			g_matches[g_matches_len++].filename = strdup(de->d_name);
		}
		if (g_last_to_match_dir) {
			free(g_last_to_match_dir);
			g_last_to_match_dir = NULL;
		}
		g_last_to_match_dir = strdup(to_match_dir);
		closedir(dr);
	}
	if (g_matches_len && '\t' == key) {
		if (!g_last_to_match_dir
		|| (g_last_to_match_dir && strcmp(to_match, g_last_to_match_dir))) {
			g_matches_founded = 0UL;
			for (size_t i = 0UL; g_matches_len > i; i++) {
				size_t	match = 0UL;

				g_matches[i].match = false;
				if (strlen(g_matches[i].filename) < to_match_len)
					continue ;
				while (to_match[match]
					&& to_match[match] == g_matches[i].filename[match])
					++match;
				if (to_match_len && match == to_match_len) {
					++g_matches_founded;
					g_matches[i].match = true;
				}
			}
			qsort(g_matches, g_matches_len,
				sizeof(struct s_matches), op_ofc_match_cmp);
			g_at_match = 0UL;
			if (g_last_to_match) {
				free(g_last_to_match);
				g_last_to_match = NULL;
			}
			g_last_to_match = strdup(to_match);
		}
		if (g_matches_founded) {
			if (g_at_match == g_matches_founded)
				g_at_match = 0UL;
			strcpy(to_match, g_matches[g_at_match].filename);
			*path_len += strlen(g_matches[g_at_match].filename) - to_match_len;
			++g_at_match;
		}
	}
	/* if (10 < 2 && g_matches_len && CTRL_KEY('\t') == key) {
		char	*all_matches = NULL;
		size_t	all_matches_len = 0UL;
		size_t	matches_to_print = 0UL;

		for (matches_to_print = 0UL;
			g_matches_len > matches_to_print;
			matches_to_print++) {
			all_matches_len += strlen(g_matches[matches_to_print].filename);
			if (all_matches_len >= (size_t)g_editor.screen_cols - 4)
				break ;
		}
		all_matches = calloc(all_matches_len, sizeof(char));
		for (size_t i = 0UL; matches_to_print + 1 > i; i++) {
			strcpy(all_matches + strlen(all_matches), g_matches[i].filename);
			all_matches[strlen(all_matches)] = ' ';
		}
		strcpy(all_matches + strlen(all_matches), "...");
		set_status_msg(all_matches);
		draw_refresh_screen();
		sleep(1);
	} */
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
		path = op_full_path(path, NULL);
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
