#ifndef SIMPLE_ACCOUNT_HELPER_HPP
#define SIMPLE_ACCOUNT_HELPER_HPP

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ServiceTypes>
#include <TelepathyQt/Types>

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)
QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QStandardItem)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class AccountHelper : public QObject
{
    Q_OBJECT
public:
    enum class AccountStatus {
        NoAccount,
        Initialization,
        NeedToEnable,
        NeedToValidate,
        NeedToConnect,
        Connected,
        Disconnected,
    };
    Q_ENUM(AccountStatus)

    explicit AccountHelper(QObject *parent = nullptr);

    QAbstractItemModel *accountsModel();

    enum class AccountModelColumn {
        AccountId,
        AccountEnabled,
        AccountValid,
        ColumnsCount,
        Invalid,
    };

    QString currentAccountId() const;
    AccountStatus currentAccountStatus() const;

    Tp::AccountPtr getAccountById(const QString &identifier) const;

    static int columnToInt(AccountModelColumn column);
    static AccountModelColumn columnFromInt(int columnInt);

public slots:
    void start();
    void stop();

    void setManagerName(const QString &name);
    void setProtocolName(const QString &name);

    void addAccount();
    void removeAccount(const QString &identifier);
    void connectAccount(const QString &identifier);
    void disconnectAccount(const QString &identifier);

signals:
    void currentAccountIdChanged();
    void currentAccountStatusChanged();

protected slots:
    void onAccountManagerReady(Tp::PendingOperation *operation);
    void onNewAccount(const Tp::AccountPtr &account);
    void onAccountCreated(Tp::PendingOperation *operation);
    void onCurrentAccountParametersChanged(Tp::PendingOperation *operation);
    void onAccountSetEnableFinished(Tp::PendingOperation *operation = nullptr);
    void onAccountStateChanged();
    void onAccountValidityChanged();
    void onAccountRequestedPresenceChanged();
    void onAccountCurrentPresenceChanged();

protected:
    void initAccountManager();

    void disconnectAccount(const Tp::AccountPtr &account);
    void setCurrentAccountStatus(AccountStatus status);
    void updateSuitableAccounts();
    void setSuitableAccounts(const QList<Tp::AccountPtr> &accounts);
    void trackAccount(const Tp::AccountPtr &account);
    void stopTrackingAccount(const Tp::AccountPtr &account);
    void updateModelData();
    void updateAccountData(const Tp::AccountPtr &account, AccountModelColumn column);
    void updateModelItemData(QStandardItem *item, const Tp::AccountPtr &account, int columnHint = -1);

    void activateCurrentAccount();
    void reValidateCurrentAccount();
    void enableCurrentAccount();
    Tp::PendingOperation *requestAccountPresence(const Tp::AccountPtr &account, Tp::ConnectionPresenceType type);

protected:
    Tp::AccountManagerPtr m_accountManager;
    QList<Tp::AccountPtr> m_allAccounts;
    QList<Tp::AccountPtr> m_suitableAccounts;
    Tp::AccountPtr m_currentAccount;
    AccountStatus m_accountStatus = AccountStatus::NoAccount;
    QString m_managerName;
    QString m_protocolName;
    QStandardItemModel *m_accountsModel = nullptr;
};

#endif // SIMPLE_ACCOUNT_HELPER_HPP
