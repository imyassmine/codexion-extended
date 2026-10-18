# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ LAYOUT ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

DEFAULT			= \033[0m
GREEN			= \033[1;32m
CYAN			= \033[1;36m


define ANIMATION_FRAME
				@for frame in ⠋ ⠙ ⠹ ⠸ ⠼ ⠴ ⠦ ⠧ ⠇ ⠏; do \
				printf "$$frame $(1)...\r"; \
				sleep 0.02; \
				done; \
				printf "\n"
endef


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ VARIABLES ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

NAME =			codexion
PROCESSES =		codexion_processes
CC =			cc
CFLAGS =		-Wall -Wextra -Werror -pthread -I coders/include -I coders_processes/include $(EXTRA_FLAGS)

SRC_DIR =		coders/src/
OBJ_DIR =		coders/obj/
SRC_PROC_DIR =	coders_processes/src/
OBJ_PROC_DIR =	coders_processes/obj/


SRC =			$(wildcard $(SRC_DIR)*.c)
OBJ =			$(patsubst $(SRC_DIR)%.c,$(OBJ_DIR)%.o,$(SRC))
SRC_PROC =		$(wildcard $(SRC_PROC_DIR)*.c)
OBJ_PROC =		$(patsubst $(SRC_PROC_DIR)%.c,$(OBJ_PROC_DIR)%.o,$(SRC_PROC))


# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ RULES ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

all:			$(NAME) $(PROCESSES)

$(OBJ_DIR)%.o:	$(SRC_DIR)%.c
				@mkdir -p $(dir $@)
				@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PROC_DIR)%.o:	$(SRC_PROC_DIR)%.c
				@mkdir -p $(dir $@)
				@$(CC) $(CFLAGS) -c $< -o $@

$(NAME):		$(OBJ)
				$(call ANIMATION_FRAME,Building $(NAME) executable)
				$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
				@printf "$(GREEN)✓$(DEFAULT) Built successfully ($(words $?)/$(words $(SRC)) files recompiled)\n"

$(PROCESSES):	$(OBJ_PROC)
				$(call ANIMATION_FRAME,Building $(PROCESSES) executable)
				$(CC) $(CFLAGS) $(OBJ_PROC) -o $(PROCESSES)
				@printf "$(GREEN)✓$(DEFAULT) Built successfully ($(words $?)/$(words $(SRC_PROC)) files recompiled)\n"

clean:
				$(call ANIMATION_FRAME,Cleaning objects)
				@rm -rf $(OBJ_DIR) $(OBJ_PROC_DIR)

fclean:			clean
				$(call ANIMATION_FRAME,Removing executable)
				@rm -f $(NAME) $(PROCESSES)

re:				fclean all

debug:
				@$(MAKE) --no-print-directory fclean
				@$(MAKE) all --no-print-directory EXTRA_FLAGS="-g3 -fsanitize=address,undefined -fno-omit-frame-pointer"

test:			all
				@bash tests/smoke.sh

help:
				@printf "Usage: make [target]\n"
				@printf "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
				@printf "$(CYAN)all$(DEFAULT)		- Build the executable $(NAME)\n"
				@printf "$(CYAN)clean$(DEFAULT)		- Clean up object files\n"
				@printf "$(CYAN)fclean$(DEFAULT)		- Clean up all object files and executable\n"
				@printf "$(CYAN)re$(DEFAULT)		- Rebuild the entire library\n"
				@printf "$(CYAN)debug$(DEFAULT)		- Build both binaries with -g3 and sanitizers\n"
				@printf "$(CYAN)test$(DEFAULT)		- Build and run the test suite\n"
				@printf "$(CYAN)help$(DEFAULT)		- Show this help message\n"

.PHONY:			all clean fclean re debug test help
