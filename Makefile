NAME := TextEditor
NPWD := $(CURDIR)/$(NAME)

CC_BASE := gcc -march=native -mtune=native

CC := $(CC_BASE) -Ofast -pipe -flto
CC_DEBUG := $(CC_BASE) -g3 -D DEBUG

CFLAGS := -Wall -Wextra -Werror -Wunused
IFLAGS := -I $(CURDIR)/includes

SRCS := $(abspath $(wildcard srcs/*.c))
OBJ := $(SRCS:.c=.o)

DEL := rm -rf

WHITE := \033[0m
BGREEN := \033[42m
GREEN := \033[32m
RED := \033[31m
INVERT := \033[7m

SUCCESS := [$(GREEN)✓$(WHITE)]
SUCCESS2 := [$(INVERT)$(GREEN)✓$(WHITE)]

all: $(NAME)

$(OBJ): %.o: %.c
	@echo -n ' $@: '
	@$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@
	@echo "$(SUCCESS)"

$(NAME): $(OBJ)
	@echo -n ' <q.p> | $(NPWD): '
	@$(CC) $(OBJ) -o $(NAME)
	@echo "$(SUCCESS2)"

del:
	@$(DEL) $(OBJ)
	@$(DEL) $(NAME)

pre: del all
	@echo "$(INVERT)$(GREEN)Successed re-build.$(WHITE)"

set_cc_debug:
	@$(eval CC=$(CC_DEBUG))
debug_all: set_cc_debug pre
	@echo "$(INVERT)$(NAME) $(GREEN)ready for debug.$(WHITE)"
debug: set_cc_debug all
	@echo "$(INVERT)$(NAME) $(GREEN)ready for debug.$(WHITE)"

clean:
	@$(DEL) $(OBJ)

fclean: clean
	@$(DEL) $(NAME)

re: fclean all

.PHONY: re fclean clean all norme_all norme del pre debug debug_all
