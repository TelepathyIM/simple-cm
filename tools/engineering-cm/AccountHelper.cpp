#include "AccountHelper.hpp"

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingFailure>
#include <TelepathyQt/PendingStringList>

#include "HelperAccountsModel.hpp"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcSimpleAccountHelper, "simple.accountHelper", QtDebugMsg)

AccountHelper::AccountHelper(QObject *parent)
    : QObject(parent)
{
}

AccountsModel *AccountHelper::accountsModel()
{
    if (!m_accountsModel) {
        m_accountsModel = new HelperAccountsModel(this);
        if (m_accountManager) {
            m_accountsModel->init(m_accountManager);
            m_accountsModel->setFilterRules(m_managerName, m_protocolName);
        }
    }

    return m_accountsModel;
}

QString AccountHelper::currentAccountId() const
{
    return m_currentAccount.isNull() ? QString() : m_currentAccount->uniqueIdentifier();
}

AccountHelper::AccountStatus AccountHelper::currentAccountStatus() const
{
    return m_accountStatus;
}

Tp::AccountPtr AccountHelper::getAccountById(const QString &identifier) const
{
    if (!m_accountManager) {
        return Tp::AccountPtr();
    }
    for (const Tp::AccountPtr &account : m_accountManager->allAccounts()) {
        if (account->uniqueIdentifier() != identifier) {
            continue;
        }
        return account;
    }

    return Tp::AccountPtr();
}

void AccountHelper::start()
{
    if (!m_accountManager) {
        initAccountManager();
    }

    if (m_accountsModel) {
        m_accountsModel->init(m_accountManager);
        m_accountsModel->setFilterRules(m_managerName, m_protocolName);
    }
}

void AccountHelper::stop()
{
    if (!m_currentAccount) {
        return;
    }

    disconnectAccount(m_currentAccount);
}

void AccountHelper::setManagerName(const QString &name)
{
    m_managerName = name;
}

void AccountHelper::setProtocolName(const QString &name)
{
    m_protocolName = name;
}

void AccountHelper::addAccount()
{
    QString displayName;
    const QVariantMap parameters = {
        { "self_id", "local_user" },
    };
    const QVariantMap properties = {
        { TP_QT_IFACE_ACCOUNT + QLatin1String(".Enabled"), false },
    };

    Tp::PendingAccount *account = m_accountManager->createAccount(m_managerName, m_protocolName,
                                                                  displayName, parameters,
                                                                  properties);
    account->setObjectName("createAccount()");
    connect(account, &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountOperationFinished);
}

void AccountHelper::removeAccount(const QString &identifier)
{
    Tp::AccountPtr account = getAccountById(identifier);
    if (!account) {
        return;
    }

    Tp::PendingOperation *removeOperation = account->remove();
    removeOperation->setObjectName("Remove account");
    connect(removeOperation, &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountOperationFinished);
}

void AccountHelper::connectAccount(const QString &identifier)
{
    if (identifier.isEmpty()) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Account id is empty";
        return;
    }

    if (m_accountStatus != AccountStatus::NoAccount) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Active account already selected";
        return;
    }

    setCurrentAccount(getAccountById(identifier));

    if (!m_currentAccount) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Unable to find account" << identifier;
        return;
    }

    emit currentAccountIdChanged();
    setCurrentAccountStatus(AccountStatus::Initialization);

    activateCurrentAccount();
}

void AccountHelper::disconnectAccount(const QString &identifier)
{
    if (identifier.isEmpty()) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Account id is empty";
        return;
    }

    Tp::AccountPtr account = getAccountById(identifier);
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Unable to find account" << identifier;
        return;
    }
    disconnectAccount(account);
}

void AccountHelper::disconnectAccount(const Tp::AccountPtr &account)
{
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "No account given";
        return;
    }

    // account const reference can be the same object as m_currentAccount.
    // We need to work with another copy to safely reset currentAccount value.
    Tp::AccountPtr acc = account;
    if (m_currentAccount == account) {
        setCurrentAccount(Tp::AccountPtr());
        setCurrentAccountStatus(AccountStatus::NoAccount);
    }

    if (acc->isChangingPresence() || acc->isOnline()) {
        requestAccountPresence(acc, Tp::ConnectionPresenceTypeOffline);
    }
    acc->setEnabled(false);
}

void AccountHelper::setCurrentAccount(const Tp::AccountPtr &account)
{
    if (m_currentAccount == account) {
        return;
    }

    if (m_currentAccount) {
        disconnect(m_currentAccount.data(), &Tp::Account::stateChanged,
                   this, &AccountHelper::onAccountStateChanged);
        disconnect(m_currentAccount.data(), &Tp::Account::validityChanged,
                   this, &AccountHelper::onAccountValidityChanged);
        disconnect(m_currentAccount.data(), &Tp::Account::requestedPresenceChanged,
                   this, &AccountHelper::onAccountRequestedPresenceChanged);
        disconnect(m_currentAccount.data(), &Tp::Account::currentPresenceChanged,
                   this, &AccountHelper::onAccountCurrentPresenceChanged);
    }
    m_currentAccount = account;
    if (m_currentAccount) {
        connect(m_currentAccount.data(), &Tp::Account::stateChanged,
                this, &AccountHelper::onAccountStateChanged);
        connect(m_currentAccount.data(), &Tp::Account::validityChanged,
                this, &AccountHelper::onAccountValidityChanged);
        connect(m_currentAccount.data(), &Tp::Account::requestedPresenceChanged,
                this, &AccountHelper::onAccountRequestedPresenceChanged);
        connect(m_currentAccount.data(), &Tp::Account::currentPresenceChanged,
                this, &AccountHelper::onAccountCurrentPresenceChanged);
    }
}

void AccountHelper::initAccountManager()
{
    const Tp::Features accountFeatures = Tp::Account::FeatureCore | Tp::Account::FeatureProtocolInfo;
    Tp::AccountFactoryPtr accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                      accountFeatures);
    m_accountManager = Tp::AccountManager::create(accountFactory);

    connect(m_accountManager->becomeReady(), &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountManagerReady);
}

void AccountHelper::setCurrentAccountStatus(AccountHelper::AccountStatus status)
{
    qCDebug(lcSimpleAccountHelper) << __func__ << "account:" << currentAccountId() << status;
    if (m_accountStatus == status) {
        return;
    }
    m_accountStatus = status;
    emit currentAccountStatusChanged();
}

void AccountHelper::activateCurrentAccount()
{
    qCDebug(lcSimpleAccountHelper) << __func__ << currentAccountId() << currentAccountStatus();
    if (!m_currentAccount) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "No account";
        return;
    }

    if (!m_currentAccount->isValidAccount()) {
        if (m_accountStatus == AccountStatus::NeedToValidate) {
            qCCritical(lcSimpleAccountHelper) << __func__ << "Unable to make the account valid.";
            disconnectAccount(currentAccountId());
            return;
        }
        setCurrentAccountStatus(AccountStatus::NeedToValidate);
        reValidateCurrentAccount();
        return;
    }

    if (!m_currentAccount->isEnabled()) {
        if (m_accountStatus == AccountStatus::NeedToEnable) {
            qCCritical(lcSimpleAccountHelper) << __func__ << "Unable to enable the account.";
            disconnectAccount(currentAccountId());
            return;
        }
        setCurrentAccountStatus(AccountStatus::NeedToEnable);
        enableCurrentAccount();
        return;
    }

    bool connected = m_currentAccount->currentPresence().type() == Tp::ConnectionPresenceTypeAvailable;
    if (!connected) {
        if (m_accountStatus == AccountStatus::NeedToConnect) {
            qCDebug(lcSimpleAccountHelper) << __func__
                                           << "Account online status requested "
                                              "but not become the current one yet.";
            return;
        }
        setCurrentAccountStatus(AccountStatus::NeedToConnect);
        requestAccountPresence(m_currentAccount, Tp::ConnectionPresenceTypeAvailable);
        return;
    }

    setCurrentAccountStatus(AccountStatus::Connected);
}

Tp::PendingOperation *AccountHelper::requestAccountPresence(const Tp::AccountPtr &account, Tp::ConnectionPresenceType type)
{
    if (!account) {
        QString errorString = QLatin1String("No account given");
        return new Tp::PendingFailure(TP_QT_ERROR_INVALID_ARGUMENT, errorString, m_accountManager);
    }
    const Tp::PresenceSpecList presenceList = account->allowedPresenceStatuses();
    Tp::Presence presence;
    for (const Tp::PresenceSpec &spec : presenceList) {
        if (spec.bareSpec().type == type) {
            presence = spec.presence();
            break;
        }
    }
    if (!presence.isValid()) {
        QString errorString = QLatin1String("Unable to get valid presence for account");
        qCWarning(lcSimpleAccountHelper) << __func__ << errorString
                                         << account;
        return new Tp::PendingFailure(TP_QT_ERROR_NOT_AVAILABLE, errorString, account);
    }

    return account->setRequestedPresence(presence);
}

void AccountHelper::onAccountManagerReady(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        qCCritical(lcSimpleAccountHelper) << "Unable to init account manager:"
                                          << operation->errorName() << operation->errorMessage();
        return;
    }

    qCDebug(lcSimpleAccountHelper) << "Account manager is ready.";
}

void AccountHelper::onAccountCreated(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        qCWarning(lcSimpleAccountHelper) << operation->errorName() << operation->errorMessage();
        return;
    }
}

void AccountHelper::reValidateCurrentAccount()
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    // Update an account parameter to trigger re-validation
    QVariantMap parameters = m_currentAccount->parameters();
    qCDebug(lcSimpleAccountHelper) << m_currentAccount->parameters();

    Tp::PendingOperation *operation = m_currentAccount->updateParameters(parameters, { });
    operation->setObjectName("Account re-validation");
    connect(operation, &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountOperationFinished);
}

void AccountHelper::enableCurrentAccount()
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    Tp::PendingOperation *operation = m_currentAccount->setEnabled(true);
    operation->setObjectName("Account setEnabled(true)");
    connect(operation, &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountOperationFinished);
}

void AccountHelper::onAccountOperationFinished(Tp::PendingOperation *operation)
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    if (operation) {
        if (operation->isError()) {
            qCWarning(lcSimpleAccountHelper) << __func__ << operation
                                             << operation->errorName() << operation->errorMessage();
            return;
        }
    }
}

void AccountHelper::onAccountStateChanged()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Invalid call";
        return;
    }

    if (m_currentAccount == account) {
        activateCurrentAccount();
    }
}

void AccountHelper::onAccountValidityChanged()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Invalid call";
        return;
    }

    qCDebug(lcSimpleAccountHelper) << __func__ << account->uniqueIdentifier()
                                   << "valid:" << account->isValidAccount();

    if (m_currentAccount == account) {
        activateCurrentAccount();
    }
}

void AccountHelper::onAccountRequestedPresenceChanged()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Invalid call";
        return;
    }

    qCDebug(lcSimpleAccountHelper) << __func__ << account->uniqueIdentifier()
                                   << "requested presence:" << account->requestedPresence().type();

    // There is no reaction on requested presense changed.
}

void AccountHelper::onAccountCurrentPresenceChanged()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Invalid call";
        return;
    }

    qCDebug(lcSimpleAccountHelper) << __func__ << account->uniqueIdentifier()
                                   << "presence:" << account->currentPresence().type();

    if (m_currentAccount == account) {
        activateCurrentAccount();
    }
}
