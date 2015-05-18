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

    void on_sendMessageButton_clicked();

    void whenMessage(QString sender, QString message, bool fromSelfcontact = true);

    void on_addContactButton_clicked();

private:
    Ui::MainWindow *ui;
    SimpleProtocol *m_protocol;

    CContactsModel *m_contactsModel;
};

#endif // MAINWINDOW_H
