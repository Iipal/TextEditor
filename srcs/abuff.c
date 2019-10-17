#include "abuff.h"

inline void	ab_append(struct s_abuff *ab, const char *s, int len)
{
	char	*new = realloc(ab->b, ab->len + len);

	if (!new)
		return ;
	memcpy(&new[ab->len], s, len);
	ab->b = new;
	ab->len += len;
}

inline void	ab_free(struct s_abuff *ab)
{
	free(ab->b);
}
