NAME = MattDaemon
CLIENT_NAME = MattDaemonClient

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11 -Iinclude -fPIC
MOC = /opt/homebrew/opt/qt@5/bin/moc

# Default paths for Qt5 on macOS with Homebrew (Apple Silicon)
QT5_PATH = /opt/homebrew/opt/qt@5
QT_HEADERS = -I$(QT5_PATH)/include
QT_FRAMEWORKS = -F$(QT5_PATH)/lib
QT_INCLUDES = $(QT_HEADERS) \
              -I$(QT5_PATH)/lib/QtWidgets.framework/Headers \
              -I$(QT5_PATH)/lib/QtCore.framework/Headers \
              -I$(QT5_PATH)/lib/QtGui.framework/Headers \
              -I$(QT5_PATH)/lib/QtNetwork.framework/Headers \
              $(QT_FRAMEWORKS)

QT_LINK = $(QT_FRAMEWORKS) \
          -framework QtWidgets \
          -framework QtCore \
          -framework QtGui \
          -framework QtNetwork

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

DAEMON_SRCS = $(SRC_DIR)/main.cpp \
              $(SRC_DIR)/Daemon.cpp \
              $(SRC_DIR)/Server.cpp \
              $(SRC_DIR)/Client.cpp \
              $(SRC_DIR)/Utils.cpp \
              $(SRC_DIR)/Tintin_reporter.cpp \
              $(SRC_DIR)/ShellCommands.cpp

GUI_SRCS = $(SRC_DIR)/gui_main.cpp \
           $(SRC_DIR)/GUI.cpp \
           $(OBJ_DIR)/moc_GUI.cpp

DAEMON_OBJS = $(DAEMON_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
GUI_OBJS = $(GUI_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

$(NAME): $(DAEMON_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(DAEMON_OBJS) -o $(BIN_DIR)/$@

$(CLIENT_NAME): $(GUI_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(GUI_OBJS) -o $(BIN_DIR)/$@ $(QT_LINK)

# Rule for moc files
$(OBJ_DIR)/moc_%.cpp: $(INC_DIR)/%.hpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(MOC) $(QT_HEADERS) $< -o $@

# Rule for C++ files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(QT_HEADERS) -c $< -o $@

# Rule for moc-generated C++ files
$(OBJ_DIR)/moc_GUI.o: $(OBJ_DIR)/moc_GUI.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(QT_INCLUDES) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -rf $(BIN_DIR)

client: $(CLIENT_NAME)

gui: client

re: fclean all

.PHONY: all clean fclean re client gui