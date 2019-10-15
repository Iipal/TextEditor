#ifndef SYNTAX_H
# define SYNTAX_H

enum	e_editor_highlight {
	HL_NORMAL = 0,
	HL_COMMENT,
	HL_MLCOMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH
};

# define HL_NUMBERS (1 << 0)
# define HL_STRINGS (1 << 1)

struct	s_syntax {
	char	*filetype;
	char	**filematch;
	char	**keywords;
	char	*singleline_comment_start;
	char	*ml_comment_start;
	char	*ml_comment_end;
	int		flags;
};

extern struct s_syntax	g_HLDB[];

# define G_HLDB_ENTIRES (sizeof(g_HLDB) / sizeof(*g_HLDB))

#endif /*SYNTAX_H*/
