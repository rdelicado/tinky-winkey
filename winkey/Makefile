# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/08 13:00:27 by vzurera-          #+#    #+#              #
#    Updated: 2025/05/21 16:17:55 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ───────────────────────────────────────────────────────────── #
# ─────────────────────── CONFIGURATION ─────────────────────── #
# ───────────────────────────────────────────────────────────── #

# /wd5045:		Avisos sobre funciones de inserción para mitigación especulativa
# /wd4255:		Falta de prototipos de función: func() vs func(void)
# /wd4668:		Macros no definidas en condiciones

NAME			= winkey
SRC				= src
OBJ				= build\obj
BIN				= bin

CC				= cl /nologo
FLAGS			= /Wall /WX /wd5045 /wd4255 /wd4668
LIBS			= advapi32.lib user32.lib Shell32.lib

OBJS			= $(OBJ)\main.obj			\
				  $(OBJ)\hook.obj			\
				  $(OBJ)\key.obj			\
				  $(OBJ)\clipboard.obj		\
				  $(OBJ)\log.obj			\
				  $(OBJ)\service.obj		\
				  $(OBJ)\keyboardstate.obj	\
				  $(OBJ)\resource.res

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── RULES ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

.SUFFIXES: .cpp .obj

all: $(BIN)\$(NAME).exe
winkey: $(BIN)\$(NAME).exe

$(BIN)\$(NAME).exe: $(OBJS)
	@if not exist $(BIN) mkdir $(BIN)
	@$(CC) -Iinc $(FLAGS) /Fe$(BIN)\$(NAME).exe $(OBJS) $(LIBS)

$(OBJ)\main.obj: $(SRC)\main.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\hook.obj: $(SRC)\hook.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\key.obj: $(SRC)\key.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\clipboard.obj: $(SRC)\clipboard.c
    @if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\log.obj: $(SRC)\log.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\service.obj: $(SRC)\service.c
	@if not exist $(OBJ) mkdir $(OBJ)
	@$(CC) -Iinc $(FLAGS) /c /Fo$@ $**

$(OBJ)\keyboardstate.obj: $(SRC)\keyboardstate.c
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

.PHONY: all winkey clean fclean re
