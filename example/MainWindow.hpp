#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

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

    void addMessageFromSelfContact(QString target, QString message);
    void addMessage(QString sender, QString message);

private:
    Ui::MainWindow *ui;
    SimpleProtocol *m_protocol;

    CContactsModel *m_contactsModel;
};

#endif // MAINWINDOW_H
