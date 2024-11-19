#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Create the main window
    MainWindow window;
    window.show();

    // Start the application loop
    return app.exec();
}

