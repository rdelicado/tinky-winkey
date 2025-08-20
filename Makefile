# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/08 13:00:27 by vzurera-          #+#    #+#              #
#    Updated: 2025/05/14 16:11:43 by vzurera-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ───────────────────────────────────────────────────────────── #
# ─────────────────────── CONFIGURATION ─────────────────────── #
# ───────────────────────────────────────────────────────────── #

SVC			= svc
WINKEY		= winkey
BIN			= bin

# ───────────────────────────────────────────────────────────── #
# ─────────────────────────── RULES ─────────────────────────── #
# ───────────────────────────────────────────────────────────── #

all: _$(SVC) _$(WINKEY)
$(SVC): _$(SVC)
$(WINKEY): _$(WINKEY)

_$(SVC):
	@cd $(SVC) && nmake /NOLOGO
	@$(MAKE) /NOLOGO _copy_binaries

_$(WINKEY):
	@cd $(WINKEY) && nmake /NOLOGO
	@$(MAKE) /NOLOGO _copy_binaries

clean:
	@cd $(SVC) && nmake /NOLOGO clean
	@cd $(WINKEY) && nmake /NOLOGO clean

fclean:
	@cd $(SVC) && nmake /NOLOGO fclean
	@cd $(WINKEY) && nmake /NOLOGO fclean
	@if exist $(BIN) rmdir /s /q $(BIN)

re: fclean all

_copy_binaries:
	@if not exist $(BIN) mkdir $(BIN)
	@if exist $(SVC)\bin\$(SVC).exe copy $(SVC)\bin\$(SVC).exe $(BIN) > nul
	@if exist $(WINKEY)\bin\$(WINKEY).exe copy $(WINKEY)\bin\$(WINKEY).exe $(BIN) > nul

.PHONY: all clean fclean re _$(SVC) _$(WINKEY) _copy_binaries
