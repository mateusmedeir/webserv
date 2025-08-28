NAME = Webserv

COMPILER = c++

FLAGS = -Wall -Werror -Wextra

CPP_FLAGS = -std=c++98

SRC = main.cpp

OBJ = $(SRC:.cpp=.o)

.cpp.o :
	@$(COMPILER) $(FLAGS) $(CPP_FLAGS) -c $< -o $(<:.cpp=.o)

$(NAME) : $(OBJ) progress
	@$(COMPILER) $(FLAGS) $(CPP_FLAGS) $(OBJ) -o $(NAME)

all : $(NAME)
	@sleep 0.2
	@printf "\033[0;32m ALL READY TO GO!\033[0m\n"; \

clean :
	@rm -rf $(OBJ)
	@sleep 0.1
	@printf "\033[0;32m OBJECTS CLEANED!\033[0m\n"; \

fclean : clean
	@rm -rf $(NAME)
	@sleep 0.1
	@printf "\033[0;32m ALL CLEANED!\033[0m\n"; \

re : fclean all

progress :
	@$(MAKE) --no-print-directory _progress

_progress :
	@tput civis
	@width=50; \
	progress=0; \
	while [ $$progress -le $$width ]; do \
		printf "\r\033[0;32m Progress: [ "; \
		for i in $$(seq 1 $$progress); do \
			printf "="; \
		done; \
		for i in $$(seq $$progress $$width); do \
			printf " "; \
		done; \
		percent=$$((progress * 2)); \
		printf "]%d%%\033[0m" $$percent; \
		progress=$$((progress + 5)); \
		sleep 0.1; \
	done; \
	printf "\n\033[0;32m COMPILATION COMPLETE!\033[0m\n"; \
	tput cnorm

workflow : $(OBJ)
	@$(COMPILER) $(FLAGS) $(CPP_FLAGS) $(OBJ) -o $(NAME)

.PHONY: all clean fclean re