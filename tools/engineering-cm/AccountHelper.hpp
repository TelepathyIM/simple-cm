#ifndef SIMPLE_ACCOUNT_HELPER_HPP
#define SIMPLE_ACCOUNT_HELPER_HPP

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingReady>
#include <TelepathyQt/ServiceTypes>
#include <TelepathyQt/Types>

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)
QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class AccountHelper : public QObject
{
    Q_OBJECT
public:
    enum class AccountStatus {
        NoAccount,
        Initialization,
        ReValidation,
        Connected,
        Disconnected,
    };
    Q_ENUM(AccountStatus)

    explicit AccountHelper(QObject *parent = nullptr);

    QAbstractItemModel *accountsModel();

    enum AccountModelSection {
        AccountId,
        AccountEnabled,
        AccountValid,
        SectionsCount,
    };

    QString currentAccountId() const;
    AccountStatus currentAccountStatus() const;

    Tp::AccountPtr getAccountById(const QString &identifier) const;

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
    void onAccountValid();
    void onAccountSetEnableFinished(Tp::PendingOperation *operation = nullptr);
    void onAccountStateChanged();
    void onAccountEnabled();

protected:
    void initAccountManager();

    void setCurrentAccountStatus(AccountStatus status);
    void updateSuitableAccounts();
    void setSuitableAccounts(const QList<Tp::AccountPtr> &accounts);
    void updateModelData();

    void reValidateAccount();
    void enableAccount();
    void requestAccountOnline();
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
