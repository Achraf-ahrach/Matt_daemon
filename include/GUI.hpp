#ifndef GUI_HPP
#define GUI_HPP

#include <QtWidgets/QMainWindow>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMessageBox>
#include <QtCore/QDateTime>
#include <QtWidgets/QScrollBar>

class MattDaemonGUI : public QMainWindow {
    Q_OBJECT

public:
    MattDaemonGUI(QWidget *parent = nullptr);
    ~MattDaemonGUI();

private slots:
    void connectToServer();
    void disconnectFromServer();
    void sendCommand();
    void readResponse();
    void displayError(QAbstractSocket::SocketError socketError);
    void connectionEstablished();
    void connectionClosed();

private:
    void setupUI();
    void logMessage(const QString &message);
    void setConnectedState(bool connected);
    void handleAuthentication(const QString &response);

    QTcpSocket *tcpSocket;
    QTextEdit *logDisplay;
    QLineEdit *commandInput;
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *sendButton;
    QStatusBar *statusBar;
    bool isConnected;
    bool isAuthenticated;
    bool waitingForPassword;
    QString pendingResponse;
};

#endif // GUI_HPP
