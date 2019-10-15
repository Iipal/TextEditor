#ifndef DATA_H
# define DATA_H

# include <termios.h>
# include <time.h>

# include "syntax.h"

# define E_TAB_STOP 8
# define E_QUIT_TIMES 3

# define CTRL_KEY(key) ((key) & 0x1f)

enum	e_editor_key {
	BACKSPACE = 127,
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};

typedef struct	s_e_row {
	int				idx;
	int				size;
	int				r_size;
	char			*chars;
	char			*render;
	unsigned char	*hl;
	int				hl_open_comment;
}				e_row;

struct	s_editor_config {
	int				cx;
	int				cy;
	int				rx;
	int				row_off;
	int				col_off;
	int				screen_rows;
	int				screen_cols;
	int				num_rows;
	e_row			*row;
	int				dirty;
	char			*filename;
	char			status_msg[80];
	time_t			status_msg_time;
	struct s_syntax	*syntax;
	struct termios	raw_termios;
};

struct s_editor_config	g_editor;

#endif /*DATA_H*/
