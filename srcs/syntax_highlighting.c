#include "editor.h"

struct s_syntax	g_HLDB[] = {
	{ "c",
		(char*[]){ ".c", ".h", ".cpp", NULL },
		(char*[]){ "switch", "if", "while", "for", "break", "continue",
			"return", "else", "struct", "union", "typedef", "static",
			"enum", "class", "case", "inline", "#include", "const",
			"int|", "long|", "double|", "float|", "char|",
			"unsigned|", "signed|", "void|", "size_t|", NULL },
		"//", "/*", "*/",
		HL_NUMBERS | HL_STRINGS
	}
};

static inline bool	is_separator(int c)
{
	return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c);
}

void				syntax_update(e_row *row)
{
	row->hl = realloc(row->hl, row->r_size);
	memset(row->hl, HL_NORMAL, row->r_size);
	if (!g_editor.syntax)
		return ;

	char	**kw = g_editor.syntax->keywords;
	char	*scs = g_editor.syntax->singleline_comment_start;
	char	*mcs = g_editor.syntax->ml_comment_start;
	char	*mce = g_editor.syntax->ml_comment_end;
	int		scs_len = scs ? strlen(scs) : 0;
	int		mcs_len = mcs ? strlen(mcs) : 0;
	int		mce_len = mce ? strlen(mce) : 0;

	int	in_string = 0;
	int	in_comment = (0 < row->idx
		&& g_editor.row[row->idx - 1].hl_open_comment);
	int	prev_sep = 1;
	int	i = 0;

	while (row->r_size > i) {
		char			c = row->render[i];
		unsigned char	prev_hl = (0 < i) ? row->hl[i - 1] : HL_NORMAL;

		if (scs_len && !in_string && !in_comment)
			if (!strncmp(&row->render[i], scs, scs_len)) {
				memset(&row->hl[i], HL_COMMENT, row->r_size - i);
				break ;
			}
		if (mcs_len && mce_len && !in_string) {
			if (in_comment) {
				row->hl[i] = HL_MLCOMMENT;
				if (!strncmp(&row->render[i], mce, mce_len)) {
					memset(&row->hl[i], HL_MLCOMMENT, mce_len);
					i += mce_len;
					in_comment = 0;
					prev_sep = 1;
					continue ;
				} else {
					++i;
					continue ;
				}
			} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
				memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
				i += mcs_len;
				in_comment = 1;
				continue ;
			}
		}
		if (g_editor.syntax->flags  & HL_STRINGS) {
			if (in_string) {
				row->hl[i] = HL_STRING;
				if ('\\' == c && row->r_size > i + 1) {
					row->hl[i + 1] = HL_STRING;
					i += 2;
					continue ;
				}
				if (in_string == c)
					in_string = 0;
				++i;
				prev_sep = 1;
				continue ;
			} else {
				if ('"' == c || '\'' == c) {
					in_string = c;
					row->hl[i++] = HL_STRING;
					continue ;
				}
			}
		}
		if (g_editor.syntax->flags & HL_NUMBERS) {
			if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER))
			|| (c == '.' && prev_hl == HL_NUMBER)) {
				row->hl[i++] = HL_NUMBER;
				prev_sep = 0;
				continue ;
			}
		}
		if (prev_sep) {
			int j;
			for (j = 0; kw[j]; j++) {
				int		kw_len = strlen(kw[j]);
				bool	is_kw2 = kw[j][kw_len - 1] == '|';

				if (is_kw2)
					--kw_len;
				if (!strncmp(&row->render[i], kw[j], kw_len)
				&& is_separator(row->render[i + kw_len])) {
					memset(&row->hl[i],
						is_kw2 ? HL_KEYWORD2 : HL_KEYWORD1, kw_len);
					i += kw_len;
					break ;
				}
			}
			if (kw[j]) {
				prev_sep = 0;
				continue ;
			}
		}

		prev_sep = is_separator(c);
		++i;
	}

	int	changed = (row->hl_open_comment != in_comment);

	row->hl_open_comment = in_comment;
	if (changed && g_editor.num_rows > row->idx + 1)
		syntax_update(&g_editor.row[row->idx + 1]);
}

inline int			syntax_to_clr(int hl)
{
	switch(hl) {
		case HL_COMMENT:
		case HL_MLCOMMENT: return 36;
		case HL_KEYWORD1: return 33;
		case HL_KEYWORD2: return 32;
		case HL_STRING: return 35;
		case HL_NUMBER: return 31;
		case HL_MATCH: return 34;
		default: return 37;
	}
}

void				syntax_select_hl(void)
{
	g_editor.syntax = NULL;
	if (!g_editor.filename)
		return ;

	for (unsigned int j = 0U; G_HLDB_ENTIRES > j; j++) {
		struct s_syntax	*s = &g_HLDB[j];

		unsigned int	i = 0U;

		while (s->filematch[i]) {
			char	*p = strstr(g_editor.filename, s->filematch[i]);

			if (p) {
				int pat_len = strlen(s->filematch[i]);

				if ('.' != s->filematch[i][0] || '\0' == p[pat_len]) {
					g_editor.syntax = s;
					for (int f_row = 0; g_editor.num_rows > f_row; f_row++)
						syntax_update(&g_editor.row[f_row]);
					return ;
				}
			}
			++i;
		}
	}
}
