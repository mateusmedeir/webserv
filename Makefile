NAME	=	Parser

SRCS	=	main.cpp         \
		ParserConfigFile.cpp  \
		ServerBlock.cpp        \

OBJDIR	=	Objects

OBJS	=	$(SRCS:%.cpp=$(OBJDIR)/%.o)

CXX	=	c++

CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98

RM	=	rm -rf

TOTAL_SRCS	=	$(words $(SRCS))

COMPILED_SRCS	=	0

all: $(NAME)

$(NAME): $(OBJS)
		@$(CXX) -o $(NAME) $(OBJS)
		@echo "\033[1;97m🔔 WebServer Is Ready! 🔔\033[0m"

$(OBJDIR)/%.o: %.cpp
		@mkdir -p $(dir $@)
		@$(CXX) $(CXXFLAGS) -c $< -o $@
		$(eval COMPILED_SRCS=$(shell echo $$(($(COMPILED_SRCS)+1))))
		$(eval COLOR_VALUE=$(shell echo $$((255*$(COMPILED_SRCS)/$(TOTAL_SRCS)))))
		@echo -n "\033[38;2;$(COLOR_VALUE);$(COLOR_VALUE);$(COLOR_VALUE)m  WebServer Is Ready!\033[0m\r"
		@sleep 0.01

supp: re
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./WebServer config.conf

clean:
		@$(RM) $(OBJDIR)
		@echo "\033[38;2;255;165;0m🗑️  Objects Are Cleaned! 🗑️\033[0m"

fclean: clean
		@$(RM) $(NAME)
		@echo "\033[31m🗑️  WebServer Is Cleaned! 🗑️\033[0m"

re: fclean all

.PHONY: all clean fclean re
