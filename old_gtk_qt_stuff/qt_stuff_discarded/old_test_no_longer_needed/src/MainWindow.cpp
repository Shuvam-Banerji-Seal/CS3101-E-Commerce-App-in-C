#include "MainWindow.h"
#include <QMenuBar>
#include <QMessageBox>

MainWindow::MainWindow() {
    // Set window title
    setWindowTitle("Library Management System");

    // Set window dimensions
    resize(800, 600);

    // Create menu
    QMenuBar *menuBar = new QMenuBar(this);

    // Add "File" menu
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *exitAction = fileMenu->addAction("Exit");

    // Connect exit action
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Set menu bar
    setMenuBar(menuBar);
}

MainWindow::~MainWindow() {
    // Destructor
}

