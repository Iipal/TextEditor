#ifndef FLAGS_H
# define FLAGS_H

struct	s_flags {
	int	b_mask;
	int	valid_flags;
};

struct s_flags	g_flags;

# define BIT_TO_N(n) (1 << (n))

# define BIT_SET(mask, bit) ((mask) |= (bit))
# define BIT_TOGGLE(mask, bit) ((mask) ^= (bit))
# define BIT_UNSET(mask, bit) ((mask) &= ~(bit))

# define IS_BIT(mask, bit) ((mask) & (bit))

/* TEW - Trim End Whitespaces. Delete all whitespaces from EoF. */
# define F_B_TEW (1 << 0)
/* Saving EoF symbol as \r\n instead of \n. */
# define F_B_CRLF (1 << 1)

# define F_TEW  "tew"
# define F_CRLF "crlf"

#endif /* FLAGS_H */
