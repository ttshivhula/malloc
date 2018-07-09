ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

SRCS		= malloc.c

PATH_OBJ	= obj
PATH_SRC	= .
PATH_INC	= inc

NAME		= libft_malloc_$(HOSTTYPE).so
CFLAGS		= #-Wall -Wextra -Werror
DLFLAGS		= -shared -fPIC
OBJECTS		= $(patsubst %.c, $(PATH_OBJ)/%.o, $(SRCS))
DEBUG		= -g -O0

.PHONY: all

all: $(NAME)

$(NAME): $(OBJECTS)
	@gcc $(DLFLAGS) -o $@ $(OBJECTS)
	@rm -f libft_malloc.so
	@ln -s $(NAME) libft_malloc.so
	@echo libft_malloc.so now link to $(NAME)

$(PATH_OBJ)/%.o: $(addprefix $(PATH_SRC)/,%.c)
	@mkdir -p $(PATH_OBJ)
	$(CC) -c -o $@ $(CFLAGS) $^ -O0 -g  -I $(PATH_INC)/

clean:
	@rm -f $(OBJECTS)
	@echo Delete $(words $(OBJECTS)) object file

fclean: clean
	rm -f $(NAME)
	rm -f libft_malloc.so

re: fclean $(NAME)
