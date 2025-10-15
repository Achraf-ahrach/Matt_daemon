CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11
INCLUDES = -I./include

NAME = MattDaemon
CLIENT_TEST = client_test

all: $(NAME)

$(NAME): src/main.cpp include/Tintin_reporter.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) src/main.cpp -o $(NAME)
	@echo "MattDaemon compiled successfully"

client: $(CLIENT_TEST)

$(CLIENT_TEST): client_test.cpp
	$(CXX) $(CXXFLAGS) client_test.cpp -o $(CLIENT_TEST)
	@echo "Client test compiled successfully"

clean:
	@echo "Nothing to clean (no object files)"

fclean: clean
	rm -f $(NAME) $(CLIENT_TEST)
	@echo "Executables cleaned"

re: fclean all

.PHONY: all clean fclean re
