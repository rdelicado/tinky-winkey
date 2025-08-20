# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/08 13:00:27 by vzurera-          #+#    #+#              #
#    Updated: 2025/05/14 16:11:15 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ───────────────────────────────────────────────────────────── #
# ─────────────────────── CONFIGURATION ─────────────────────── #
# ───────────────────────────────────────────────────────────── #

# /wd4255:		Falta de prototipos de función: func() vs func(void)
# /wd4668:		Macros no definidas en condiciones

NAME			= svc
SRC				= src
OBJ				= build\obj
BIN				= bin

CC				= cl /nologo
FLAGS			= /Wall /WX /wd4255 /wd4668
LIBS			= advapi32.lib

OBJS			= $(OBJ)\main.obj				\
				  $(OBJ)\service_control.obj	\
				  $(OBJ)\service_manager.obj	\
				  $(OBJ)\impersonation.obj		\
				  $(OBJ)\resource.res

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── RULES ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

.SUFFIXES: .c .obj

all: $(BIN)\$(NAME).exe
svc: $(BIN)\$(NAME).exe

$(BIN)\$(NAME).exe: $(OBJS)
	@if not exist $(BIN) mkdir $(BIN)
	@$(CC) -Iinc $(FLAGS) /Fe$(BIN)\$(NAME).exe $(OBJS) $(LIBS)

$(OBJ)\main.obj: $(SRC)\main.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\service_control.obj: $(SRC)\service_control.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\service_manager.obj: $(SRC)\service_manager.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\impersonation.obj: $(SRC)\impersonation.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\resource.res: res\resource.rc
	@if not exist $(OBJ) mkdir $(OBJ)
	@rc /nologo /fo$@ res\resource.rc

clean:
	@if exist $(OBJ) rmdir /s /q $(OBJ)
	@if exist build rmdir /s /q build
	@echo $(NAME)'s objects deleted

fclean: clean
	@if exist $(BIN) rmdir /s /q $(BIN)
	@echo $(NAME)'s binary deleted

re: fclean all

.PHONY: all svc clean fclean re
