// #include "mainwindow.h"
// #include "./ui_mainwindow.h"

// MainWindow::MainWindow(QWidget *parent)
//     : QMainWindow(parent)
//     , ui(new Ui::MainWindow)
// {
//     ui->setupUi(this);
// }

// MainWindow::~MainWindow()
// {
//     delete ui;
// }

// #include "mainwindow.h"
// #include "ui_mainwindow.h"
// #include <QMessageBox>
// #include <QtSql/QSqlError>

// MainWindow::MainWindow(QWidget *parent)
//     : QMainWindow(parent), ui(new Ui::MainWindow) {
//     ui->setupUi(this);

//     // Set up the SQLite database
//     db = QSqlDatabase::addDatabase("QSQLITE");
//     db.setDatabaseName("user_auth.db");

//     if (!db.open()) {
//         QMessageBox::critical(this, "Database Error", db.lastError().text());
//         return;
//     }

//     createUserTable();
// }

// MainWindow::~MainWindow() {
//     delete ui;
// }

// void MainWindow::createUserTable() {
//     QSqlQuery query;
//     query.exec("CREATE TABLE IF NOT EXISTS users ("
//                "user_id TEXT PRIMARY KEY, "
//                "first_name TEXT, "
//                "last_name TEXT, "
//                "registration_number TEXT)");
// }

// QString MainWindow::generatePrimaryKey(const QString &firstName, const QString &lastName, const QString &regNumber) {
//     QString initials = firstName.left(1).toUpper() + lastName.left(1).toUpper();
//     return initials + regNumber;
// }

// void MainWindow::see_signInButton_clicked() {
//     // Handle sign-in logic here
//     QMessageBox::information(this, "Sign In", "Sign-in functionality not yet implemented.");
// }

// void MainWindow::see_signUpButton_clicked() {
//     QString firstName = ui->firstNameLineEdit->text();
//     QString lastName = ui->lastNameLineEdit->text();
//     QString regNumber = ui->regNumberLineEdit->text();

//     if (firstName.isEmpty() || lastName.isEmpty() || regNumber.isEmpty()) {
//         QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
//         return;
//     }

//     QString userID = generatePrimaryKey(firstName, lastName, regNumber);

//     QSqlQuery query;
//     query.prepare("INSERT INTO users (user_id, first_name, last_name, registration_number) "
//                   "VALUES (?, ?, ?, ?)");
//     query.addBindValue(userID);
//     query.addBindValue(firstName);
//     query.addBindValue(lastName);
//     query.addBindValue(regNumber);

//     if (!query.exec()) {
//         QMessageBox::critical(this, "Database Error", query.lastError().text());
//     } else {
//         QMessageBox::information(this, "Success", "User signed up successfully!");
//     }
// }




#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Set up the SQLite database
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user_auth.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }

    createUserTable();

    // Explicitly connect the buttons to their respective slots
    connect(ui->signInButton, &QPushButton::clicked, this, &MainWindow::signIn);
    connect(ui->signUpButton, &QPushButton::clicked, this, &MainWindow::signUp);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::createUserTable() {
    // Create the table for storing user information
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "user_id TEXT PRIMARY KEY, "
               "first_name TEXT, "
               "last_name TEXT, "
               "registration_number TEXT)");
}

QString MainWindow::generatePrimaryKey(const QString &firstName, const QString &lastName, const QString &regNumber) {
    QString initials = firstName.left(1).toUpper() + lastName.left(1).toUpper();
    return initials + regNumber;
}

// void MainWindow::signIn() {
//     // For now, we will just display a message
//     QMessageBox::information(this, "Sign In", "Sign-in functionality not yet implemented.");
// }

void MainWindow::signIn() {
    QString firstname = ui->firstNameLineEdit->text();
    QString lastname = ui->lastNameLineEdit->text();
    QString regnumber = ui->regNumberLineEdit->text();

    if (firstname.isEmpty() || lastname.isEmpty() || regnumber.isEmpty()) {
        QMessageBox::warning(this, "Incomplete", "Please fill all fields.");
        return;
    }

    QString userID = generatePrimaryKey(firstname, lastname, regnumber);

    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE user_id = :user_id"); // Corrected column name
    query.bindValue(":user_id", userID); // Match the placeholder with the column name

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", query.lastError().text());
        return;
    }

    if (query.next()) {
        QString firstName = query.value("first_name").toString(); // Match the column name
        QString lastName = query.value("last_name").toString(); // Match the column name
        QString regNum = query.value("registration_number").toString(); // Match the column name

        QString details = QString("Welcome back %1 %2!\nRegistration Number: %3")
                              .arg(firstName, lastName, regNum);

        QMessageBox::information(this, "Sign In Success", details);
    } else {
        QMessageBox::warning(this, "Error", "User not found.");
    }
}


void MainWindow::signUp() {
    QString firstName = ui->firstNameLineEdit->text();
    QString lastName = ui->lastNameLineEdit->text();
    QString regNumber = ui->regNumberLineEdit->text();

    if (firstName.isEmpty() || lastName.isEmpty() || regNumber.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    QString userID = generatePrimaryKey(firstName, lastName, regNumber);

    // Check if the user already exists
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT user_id FROM users WHERE user_id = ?");
    checkQuery.addBindValue(userID);
    checkQuery.exec();

    if (checkQuery.next()) {
        QMessageBox::warning(this, "Duplicate Error", "User already exists.");
        return;
    }

    // Insert the new user into the database
    QSqlQuery query;
    query.prepare("INSERT INTO users (user_id, first_name, last_name, registration_number) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(userID);
    query.addBindValue(firstName);
    query.addBindValue(lastName);
    query.addBindValue(regNumber);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", query.lastError().text());
    } else {
        QMessageBox::information(this, "Success", "User signed up successfully!");
    }
}





