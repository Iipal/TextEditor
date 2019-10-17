#ifndef FLAGS_H
# define FLAGS_H

extern int	g_flags;

# define BIT_TO_N(n) (1 << (n))

# define BIT_SET(mask, bit) ((mask) |= (bit))
# define BIT_TOGGLE(mask, bit) ((mask) ^= (bit))
# define BIT_UNSET(mask, bit) ((mask) &= ~(bit))

# define IS_BIT(mask, bit) ((mask) & (bit))

# define MAX_FLAGS 1

# define F_BIT_TRIM_END_WHITESPACES (1 << 0)

# define F_TRIM_END_WHITESPACES "tew"

void	parse_flags_in_args(int argc, char *argv[]);

#endif /* FLAGS_H */
