#include "GUI.hpp"
#include "Tintin_reporter.hpp"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QtGui/QKeyEvent>

MattDaemonGUI::MattDaemonGUI(QWidget *parent)
    : QMainWindow(parent), isConnected(false) {
    setWindowTitle("MattDaemon Client");
    setMinimumSize(800, 600);
    
    // Initialize TCP socket
    tcpSocket = new QTcpSocket(this);
    
    // Setup UI
    setupUI();
    
    // Connect signals and slots
    connect(connectButton, &QPushButton::clicked, this, &MattDaemonGUI::connectToServer);
    connect(disconnectButton, &QPushButton::clicked, this, &MattDaemonGUI::disconnectFromServer);
    connect(sendButton, &QPushButton::clicked, this, &MattDaemonGUI::sendCommand);
    connect(commandInput, &QLineEdit::returnPressed, this, &MattDaemonGUI::sendCommand);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MattDaemonGUI::readResponse);
    connect(tcpSocket, &QAbstractSocket::errorOccurred,
            this, &MattDaemonGUI::displayError);
    connect(tcpSocket, &QTcpSocket::connected, this, &MattDaemonGUI::connectionEstablished);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MattDaemonGUI::connectionClosed);
    
    // Set initial state
    setConnectedState(false);
    logMessage("MattDaemon Client started. Click 'Connect' to connect to the server.");
}

MattDaemonGUI::~MattDaemonGUI() {
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
        tcpSocket->disconnectFromHost();
}

void MattDaemonGUI::setupUI() {
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create log display
    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);
    logDisplay->setFont(QFont("Monospace", 10));
    logDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    
    // Create input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("Enter command...");
    commandInput->setFont(QFont("Monospace", 10));
    
    // Create buttons
    connectButton = new QPushButton("Connect", this);
    disconnectButton = new QPushButton("Disconnect", this);
    sendButton = new QPushButton("Send", this);
    
    // Style buttons
    QString buttonStyle = "QPushButton { padding: 5px 15px; min-width: 80px; } ";
    connectButton->setStyleSheet(buttonStyle + "background-color: #4CAF50; color: white;");
    disconnectButton->setStyleSheet(buttonStyle + "background-color: #f44336; color: white;");
    sendButton->setStyleSheet(buttonStyle + "background-color: #2196F3; color: white;");
    
    // Add widgets to input layout
    inputLayout->addWidget(commandInput);
    inputLayout->addWidget(sendButton);
    
    // Create button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);
    buttonLayout->addStretch();
    
    // Add widgets to main layout
    mainLayout->addWidget(logDisplay);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(inputLayout);
    
    // Set central widget
    setCentralWidget(centralWidget);
    
    // Create status bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Not connected");
}

void MattDaemonGUI::connectToServer() {
    if (isConnected) {
        logMessage("Already connected to the server.");
        return;
    }
    
    logMessage("Connecting to MattDaemon server on localhost:4242...");
    tcpSocket->connectToHost("localhost", 4242);
}

void MattDaemonGUI::disconnectFromServer() {
    if (!isConnected) {
        logMessage("Not connected to any server.");
        return;
    }
    
    logMessage("Disconnecting from server...");
    tcpSocket->disconnectFromHost();
}

void MattDaemonGUI::sendCommand() {
    if (!isConnected) {
        logMessage("Not connected to any server.");
        return;
    }
    
    QString command = commandInput->text().trimmed();
    if (command.isEmpty())
        return;
    
    // Log the command
    logMessage(">> " + command);
    
    // Send the command to the server
    QByteArray data = (command + "\n").toUtf8();
    tcpSocket->write(data);
    
    // Clear the input field
    commandInput->clear();
}

void MattDaemonGUI::readResponse() {
    while (tcpSocket->canReadLine()) {
        QByteArray response = tcpSocket->readLine().trimmed();
        if (!response.isEmpty()) {
            logMessage("<< " + QString::fromUtf8(response));
        }
    }
}

void MattDaemonGUI::displayError(QAbstractSocket::SocketError socketError) {
    QString errorMessage;
    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            errorMessage = "Connection was refused. Make sure the server is running.";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            errorMessage = "Server closed the connection.";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMessage = "The host was not found. Check the host name and port settings.";
            break;
        default:
            errorMessage = "Error: " + tcpSocket->errorString();
    }
    
    logMessage("Error: " + errorMessage);
    setConnectedState(false);
}

void MattDaemonGUI::connectionEstablished() {
    logMessage("Connected to MattDaemon server.");
    setConnectedState(true);
}

void MattDaemonGUI::connectionClosed() {
    logMessage("Disconnected from server.");
    setConnectedState(false);
}

void MattDaemonGUI::logMessage(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    
    // Display in the GUI
    logDisplay->append(logEntry);
    
    // Log using Tintin_reporter to match daemon logging
    Tintin_reporter::log(LOG, logEntry.toStdString());
    
    // Auto-scroll to bottom
    QScrollBar *sb = logDisplay->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MattDaemonGUI::setConnectedState(bool connected) {
    isConnected = connected;
    connectButton->setEnabled(!connected);
    disconnectButton->setEnabled(connected);
    sendButton->setEnabled(connected);
    commandInput->setEnabled(connected);
    
    if (connected) {
        statusBar->showMessage("Connected to MattDaemon server");
        commandInput->setFocus();
    } else {
        statusBar->showMessage("Not connected");
    }
}