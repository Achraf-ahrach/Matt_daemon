NAME = MattDaemon

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -pthread
INCLUDES = -I./include

SRCDIR = src
OBJDIR = obj
INCDIR = include

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(NAME)
	@echo "✅ $(NAME) compiled successfully"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)
	@echo "🧹 Object files cleaned"

fclean: clean
	rm -f $(NAME) client_test
	@echo "🧹 Executable cleaned"

re: fclean all

install: $(NAME)
	@echo "📦 Installing $(NAME)..."
	sudo cp $(NAME) /usr/local/bin/
	@echo "✅ $(NAME) installed to /usr/local/bin/"

uninstall:
	@echo "🗑️  Uninstalling $(NAME)..."
	sudo rm -f /usr/local/bin/$(NAME)
	@echo "✅ $(NAME) uninstalled"

client_test: client_test.cpp
	$(CXX) $(CXXFLAGS) client_test.cpp -o client_test
	@echo "✅ Client test program compiled"

test: $(NAME)
	@echo "🧪 Testing $(NAME)..."
	@echo "Note: Run 'sudo ./$(NAME)' to start the daemon"
	@echo "Use 'telnet localhost 4242' to connect and test"
	@echo "Or compile and use: make client_test && ./client_test \"your message\""

.PHONY: all clean fclean re install uninstall test client_test