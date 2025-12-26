NAME		=	webserv

SRCS		=	main.cpp                         \
			source/ConfigFile.cpp            \
			source/EpollHandler.cpp          \
			source/EpollInstance.cpp         \
			source/HttpRequest.cpp           \
			source/HttpResponse.cpp          \
			source/LocationBlock.cpp         \
			source/RunTime.cpp               \
			source/ServerBlock.cpp           \
			source/ServerListen.cpp          \
			source/Client.cpp                \
			source/Utils.cpp                 \
			source/CookieHandler.cpp         \
			source/LogHandler.cpp            \
			source/CompositeLogHandler.cpp   \
			source/StdLogHandler.cpp         \
			source/FileLogHandler.cpp        \
			source/CgiHandler.cpp            \
			#source/Logger.cpp

OBJDIR		=	objects

OBJS		=	$(SRCS:%.cpp=$(OBJDIR)/%.o)

CXX			=	c++

CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98

INCLUDES	=	-Iincludes

RM			=	rm -rf

TOTAL_SRCS		=	$(words $(SRCS))
COMPILED_SRCS	=	0

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "\033[K\033[1;32m✅ Webserv Is Ready! ✅\033[0m"

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	$(eval COMPILED_SRCS=$(shell echo $$(($(COMPILED_SRCS)+1))))
	$(eval COLOR_VALUE=$(shell echo $$((255*$(COMPILED_SRCS)/$(TOTAL_SRCS)))))
	@echo -n "\033[38;2;0;$(COLOR_VALUE);0m  Compiling: $<\033[0m\033[K\r"
	@sleep 0.02

clean:
	@$(RM) $(OBJDIR)
	@echo "\033[38;2;255;165;0m🗑️  Objects Are Cleaned! 🗑️\033[0m"

fclean: clean
	@$(RM) $(NAME)
	@echo "\033[31m🗑️  Webserv Is Cleaned! 🗑️\033[0m"

re: fclean all

workflow: $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

#valgrind: fclean all
#	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME)

.PHONY: all clean fclean re workflow #valgrind