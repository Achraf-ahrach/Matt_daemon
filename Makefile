CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11
INCLUDES = -I./include

NAME = MattDaemon
CLIENT_TEST = client_test

# Source files
SRC_DIR = src
SRCS = $(SRC_DIR)/main.cpp

# Object files
OBJ_DIR = obj
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

# Create object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "MattDaemon compiled successfully"

client: $(CLIENT_TEST)

$(CLIENT_TEST): client_test.cpp
	$(CXX) $(CXXFLAGS) client_test.cpp -o $(CLIENT_TEST)
	@echo "Client test compiled successfully"

clean:
	rm -rf $(OBJ_DIR)
	@echo "Object files cleaned"

fclean: clean
	rm -f $(NAME) $(CLIENT_TEST)
	@echo "Executables cleaned"

re: fclean all

.PHONY: all clean fclean re client

