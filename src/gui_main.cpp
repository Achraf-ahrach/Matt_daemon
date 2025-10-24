#include "GUI.hpp"
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application style
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    // Set application info
    QApplication::setApplicationName("MattDaemon Client");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("MattDaemon");
    
    // Create and show the main window
    MattDaemonGUI window;
    window.show();
    
    return app.exec();
}
