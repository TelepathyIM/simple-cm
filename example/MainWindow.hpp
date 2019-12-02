#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace SimpleCM {

class Service;
class Message;

} // SimpleCM

class SimpleProtocol;

class CContactsModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_registerButton_clicked(bool checked);

    void on_addContactButton_clicked();
    void on_sendMessageButton_clicked();

    void addMessageFromSelfContact(const SimpleCM::Message &message);
    void addMessage(QString sender, QString text);

private:
    Ui::MainWindow *ui;
    SimpleCM::Service *m_service = nullptr;
    CContactsModel *m_contactsModel = nullptr;
};

#endif // MAINWINDOW_H
