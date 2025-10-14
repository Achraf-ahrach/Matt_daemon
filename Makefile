CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pthread
INCLUDES = -I./include

SRCDIR = src
OBJDIR = obj
INCDIR = include

# Target executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = $(SRCDIR)/server.cpp
CLIENT_SRC = $(SRCDIR)/client.cpp

# Object files
SERVER_OBJ = $(OBJDIR)/server.o
CLIENT_OBJ = $(OBJDIR)/client.o

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_OBJ) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(SERVER_OBJ) -o $(SERVER)
	@echo "✅ Server compiled"

$(CLIENT): $(CLIENT_OBJ) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJ) -o $(CLIENT)
	@echo "✅ Client compiled"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
	rm -f $(SERVER) $(CLIENT) server.log
	@echo "🧹 Cleaned"

fclean: clean

re: fclean all

test: $(SERVER) $(CLIENT)
	@echo "🧪 Testing simple server..."
	@echo "1. Start server: ./$(SERVER)"
	@echo "2. Test client: ./$(CLIENT) \"Hello\""
	@echo "3. Check logs: cat server.log"
	@echo "4. Stop server: pkill $(SERVER)"

.PHONY: all clean fclean re test
