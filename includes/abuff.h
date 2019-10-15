#ifndef ABUFF_H
# define ABUFF_H

# include <stdlib.h>
# include <string.h>

struct s_abuff {
	char	*b;
	int		len;
};

# define S_ABUFF_INIT {NULL, 0}

extern void	ab_append(struct s_abuff *ab, const char *s, int len)
				__nonnull((1,2));
extern void	ab_free(struct s_abuff *ab) __nonnull((1));

#endif /*ABUFF_H*/
