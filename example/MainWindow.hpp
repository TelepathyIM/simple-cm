#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AccountHelper.hpp"
#include "ManagerPreset.hpp"

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

    void on_contactListAddContact_clicked();
    void on_sendMessageButton_clicked();

    void addMessageFromSelfContact(const SimpleCM::Message &message);
    void addMessage(QString sender, QString text);

    void on_addAccount_clicked();
    void on_removeAccount_clicked();
    void on_connectAccount_clicked();
    void on_managerPresetsCombo_currentIndexChanged(int index);
    void on_accountsView_doubleClicked(const QModelIndex &index);

    void onCurrentAccountIdChanged();
    void onCurrentAccountStatusChanged();

private:
    void startService(const QString &cmName, const QString &protocolName);
    void stopService();

    void setupPresets();
    void updateTabsState();
    void updateAccountControls();
    QString getSelectedAccount() const;
    QString getAccountId(const QModelIndex &accountIndex) const;

    static QString accountStatusToString(AccountHelper::AccountStatus status);

    Ui::MainWindow *ui;

    QList<ManagerPreset> m_presets;
    SimpleCM::Service *m_service = nullptr;
    CContactsModel *m_contactsModel = nullptr;
    AccountHelper *m_accountHelper = nullptr;
};

#endif // MAINWINDOW_H
