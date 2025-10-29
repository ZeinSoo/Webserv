# Variables
NAME    = webserv
CXX     = c++
CFLAGS  = -Wall -Wextra -Werror -std=c++98 -g3
HEADER  = inc/
INCLUDE = inc/webServ.hpp inc/Request.hpp inc/Response.hpp
SRCDIR  = src/
OBJDIR  = .obj/

# Source files and object files
SRCS    = CGI.cpp\
			handleClientRequest.cpp\
			sendClientResponse.cpp\
			main.cpp parsing_conf.cpp\
			Request.cpp\
			webServ.cpp\
			parse_uri_path.cpp\
			Response.cpp\
	
OBJS    = $(addprefix $(OBJDIR), $(SRCS:.cpp=.o))

# Default rule
all: $(NAME)

# Rule to compile the program
$(NAME): $(OBJS)
	$(CXX) $(CFLAGS) -I$(HEADER) $(OBJS) -o $(NAME)

# Rule to compile source files into object files
$(OBJDIR)%.o: $(SRCDIR)%.cpp Makefile $(INCLUDE)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CFLAGS) -I$(HEADER) -c $< -o $@

# Rule to clean up object files
clean:
	rm -rf $(OBJDIR)

# Rule to clean up object files and the executable
fclean: clean
	rm -f $(NAME)

# Rule to recompile everything
re: fclean all

# Phony targets
.PHONY: all clean fclean re