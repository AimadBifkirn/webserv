# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Executable name
NAME = webserv

# Source files
SRCS = main.cpp \
       config/config.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(NAME)

# Link object files to create executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJS)

# Clean everything
fclean: clean
	rm -f $(NAME)

# Rebuild
re: fclean all

# Run the program with default config
run: $(NAME)
	./$(NAME)

# Run with custom config
test: $(NAME)
	./$(NAME) config/default.conf

.PHONY: all clean fclean re run test
