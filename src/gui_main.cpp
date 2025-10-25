#include "GUI.hpp"
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QApplication::setApplicationName("MattDaemon Client");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("MattDaemon");

    MattDaemonGUI window;
    window.show();
    
    return app.exec();
}
