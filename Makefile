# -- COMPILATION VARIABLES --

NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude -MMD -MP

# -- DIRECTORIES --
SRC_DIR = src
BUILD_DIR = build

# -- FILES --
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# -- TARGETS --

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking object files to create $(NAME)..."
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "$(NAME) is ready! ✅"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(NAME)

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re run

