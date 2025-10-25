#include "GUI.hpp"
#include "Tintin_reporter.hpp"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QtGui/QKeyEvent>

MattDaemonGUI::MattDaemonGUI(QWidget *parent)
    : QMainWindow(parent), isConnected(false), isAuthenticated(false), waitingForPassword(false) {
    setWindowTitle("MattDaemon Client");
    setMinimumSize(800, 600);
    
    tcpSocket = new QTcpSocket(this);
    
    setupUI();
    
    connect(connectButton, &QPushButton::clicked, this, &MattDaemonGUI::connectToServer);
    connect(disconnectButton, &QPushButton::clicked, this, &MattDaemonGUI::disconnectFromServer);
    connect(sendButton, &QPushButton::clicked, this, &MattDaemonGUI::sendCommand);
    connect(commandInput, &QLineEdit::returnPressed, this, &MattDaemonGUI::sendCommand);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MattDaemonGUI::readResponse);
    connect(tcpSocket, &QAbstractSocket::errorOccurred,
            this, &MattDaemonGUI::displayError);
    connect(tcpSocket, &QTcpSocket::connected, this, &MattDaemonGUI::connectionEstablished);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MattDaemonGUI::connectionClosed);
    
    setConnectedState(false);
    logMessage("MattDaemon Client started. Click 'Connect' to connect to the server.");
}

MattDaemonGUI::~MattDaemonGUI() {
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
        tcpSocket->disconnectFromHost();
}

void MattDaemonGUI::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    logDisplay = new QTextEdit(this);
    logDisplay->setReadOnly(true);
    QFont monoFont;
    monoFont.setFamily("Monaco");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(10);
    logDisplay->setFont(monoFont);
    logDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    
    QHBoxLayout *inputLayout = new QHBoxLayout();
    commandInput = new QLineEdit(this);
    commandInput->setPlaceholderText("Enter command...");
    commandInput->setFont(monoFont);
    
    connectButton = new QPushButton("Connect", this);
    disconnectButton = new QPushButton("Disconnect", this);
    sendButton = new QPushButton("Send", this);
    
    QString buttonStyle = "QPushButton { padding: 5px 15px; min-width: 80px; } ";
    connectButton->setStyleSheet(buttonStyle + "background-color: #4CAF50; color: white;");
    disconnectButton->setStyleSheet(buttonStyle + "background-color: #f44336; color: white;");
    sendButton->setStyleSheet(buttonStyle + "background-color: #2196F3; color: white;");
    
    inputLayout->addWidget(commandInput);
    inputLayout->addWidget(sendButton);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);
    buttonLayout->addStretch();
    
    mainLayout->addWidget(logDisplay);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(inputLayout);
    
    setCentralWidget(centralWidget);
    
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
    
    logMessage(">> " + command);
    
    QByteArray data = (command + "\n").toUtf8();
    tcpSocket->write(data);
    
    commandInput->clear();
}

void MattDaemonGUI::readResponse() {
    while (tcpSocket->canReadLine()) {
        QByteArray response = tcpSocket->readLine().trimmed();
        if (!response.isEmpty()) {
            QString responseStr = QString::fromUtf8(response);
            logMessage("<< " + responseStr);
            
            if (!isAuthenticated) {
                handleAuthentication(responseStr);
            }
        }
    }
}

void MattDaemonGUI::handleAuthentication(const QString &response) {
    if (response.contains("Username:", Qt::CaseInsensitive)) {
        QByteArray username = "admin\n";
        tcpSocket->write(username);
        logMessage(">> admin");
        return;
    }
    
    if (response.contains("Password:", Qt::CaseInsensitive)) {
        QByteArray password = "admin\n";
        tcpSocket->write(password);
        logMessage(">> ****");
        return;
    }
    
    if (response.contains("Login successful", Qt::CaseInsensitive) || 
        response.contains("✓", Qt::CaseInsensitive)) {
        isAuthenticated = true;
        logMessage("Authentication successful!");
        commandInput->setPlaceholderText("Enter command (type 'help' for available commands)...");
        return;
    }
    
    if (response.contains("Authentication failed", Qt::CaseInsensitive) || 
        response.contains("✗", Qt::CaseInsensitive)) {
        logMessage("Authentication failed. Please check credentials.");
        return;
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
    isAuthenticated = false;
    waitingForPassword = false;
}

void MattDaemonGUI::connectionClosed() {
    logMessage("Disconnected from server.");
    setConnectedState(false);
    isAuthenticated = false;
    waitingForPassword = false;
}

void MattDaemonGUI::logMessage(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp, message);
    
    logDisplay->append(logEntry);
    
    Tintin_reporter::log(LOG, logEntry.toStdString());
    
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