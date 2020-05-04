#include "AccountHelper.hpp"

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingFailure>
#include <TelepathyQt/PendingStringList>

#include <QLoggingCategory>
#include <QStandardItemModel>

Q_LOGGING_CATEGORY(lcSimpleAccountHelper, "simple.accountHelper", QtDebugMsg)

AccountHelper::AccountHelper(QObject *parent)
    : QObject(parent)
{
}

QAbstractItemModel *AccountHelper::accountsModel()
{
    if (!m_accountsModel) {
        m_accountsModel = new QStandardItemModel(this);
        m_accountsModel->setColumnCount(columnToInt(AccountModelColumn::ColumnsCount));
        updateModelData();
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
    for (const Tp::AccountPtr &suitableAccount : m_suitableAccounts) {
        if (suitableAccount->uniqueIdentifier() != identifier) {
            continue;
        }
        return suitableAccount;
    }

    return Tp::AccountPtr();
}

int AccountHelper::columnToInt(AccountHelper::AccountModelColumn column)
{
    return static_cast<int>(column);
}

AccountHelper::AccountModelColumn AccountHelper::columnFromInt(int columnInt)
{
    if ((columnInt < 0) || (columnInt > static_cast<int>(AccountModelColumn::ColumnsCount))) {
        return AccountModelColumn::Invalid;
    }
    return static_cast<AccountModelColumn>(columnInt);
}

void AccountHelper::start()
{
    if (!m_accountManager) {
        initAccountManager();
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
    connect(account, &Tp::PendingOperation::finished, this, &AccountHelper::onAccountCreated);
}

void AccountHelper::removeAccount(const QString &identifier)
{
    Tp::AccountPtr account = getAccountById(identifier);
    if (!account) {
        return;
    }
    m_suitableAccounts.removeOne(account);
    m_allAccounts.removeOne(account);

    Tp::PendingOperation *removeOperation = account->remove();
    connect(removeOperation, &Tp::PendingOperation::finished,
            this, &AccountHelper::updateSuitableAccounts);
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

    m_currentAccount = getAccountById(identifier);
    connect(m_currentAccount.data(), &Tp::Account::stateChanged,
            this, &AccountHelper::onAccountStateChanged);

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
        m_currentAccount.reset();
        setCurrentAccountStatus(AccountStatus::NoAccount);
    }

    if (acc->isChangingPresence() || acc->isOnline()) {
        requestAccountPresence(acc, Tp::ConnectionPresenceTypeOffline);
    }
    acc->setEnabled(false);
}

void AccountHelper::initAccountManager()
{
    const Tp::Features accountFeatures = Tp::Account::FeatureCore | Tp::Account::FeatureProtocolInfo;
    Tp::AccountFactoryPtr accountFactory = Tp::AccountFactory::create(QDBusConnection::sessionBus(),
                                                                      accountFeatures);
    m_accountManager = Tp::AccountManager::create(accountFactory);

    connect(m_accountManager->becomeReady(), &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountManagerReady);
    connect(m_accountManager.data(), &Tp::AccountManager::newAccount,
            this, &AccountHelper::onNewAccount);
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

void AccountHelper::updateSuitableAccounts()
{
    QList<Tp::AccountPtr> accounts;
    for (const Tp::AccountPtr &account : m_allAccounts) {
        if (account->protocolName() != m_protocolName) {
            continue;
        }
        if (account->cmName() != m_managerName) {
            continue;
        }
        accounts << account;
        qCWarning(lcSimpleAccountHelper) << __func__
                                         << "Suitable account:" << account->uniqueIdentifier();
    }

    setSuitableAccounts(accounts);
}

void AccountHelper::setSuitableAccounts(const QList<Tp::AccountPtr> &accounts)
{
    for (const Tp::AccountPtr &trackedAccount : m_suitableAccounts) {
        if (!accounts.contains(trackedAccount)) {
            stopTrackingAccount(trackedAccount);
        }
    }
    for (const Tp::AccountPtr &newAccount : accounts) {
        if (!m_suitableAccounts.contains(newAccount)) {
            trackAccount(newAccount);
        }
    }
    m_suitableAccounts = accounts;
    updateModelData();
}

void AccountHelper::trackAccount(const Tp::AccountPtr &account)
{
    connect(account.data(), &Tp::Account::stateChanged,
            this, &AccountHelper::onAccountStateChanged);
    connect(account.data(), &Tp::Account::validityChanged,
            this, &AccountHelper::onAccountValidityChanged);
    connect(account.data(), &Tp::Account::requestedPresenceChanged,
            this, &AccountHelper::onAccountRequestedPresenceChanged);
    connect(account.data(), &Tp::Account::currentPresenceChanged,
            this, &AccountHelper::onAccountCurrentPresenceChanged);
}

void AccountHelper::stopTrackingAccount(const Tp::AccountPtr &account)
{
    disconnect(account.data(), &Tp::Account::stateChanged,
               this, &AccountHelper::onAccountStateChanged);
    disconnect(account.data(), &Tp::Account::validityChanged,
               this, &AccountHelper::onAccountValidityChanged);
    disconnect(account.data(), &Tp::Account::requestedPresenceChanged,
               this, &AccountHelper::onAccountRequestedPresenceChanged);
    disconnect(account.data(), &Tp::Account::currentPresenceChanged,
               this, &AccountHelper::onAccountCurrentPresenceChanged);
}

void AccountHelper::updateModelData()
{
    if (!m_accountsModel) {
        return;
    }

    if (m_accountsModel->rowCount() > 0) {
        m_accountsModel->removeRows(0, m_accountsModel->rowCount());
    }
    for (const Tp::AccountPtr &account : m_suitableAccounts) {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem();
        rowItems << new QStandardItem();
        rowItems << new QStandardItem();
        for (int column = 0; column < rowItems.count(); ++column) {
            updateModelItemData(rowItems.at(column), account, column);
        }
        m_accountsModel->appendRow(rowItems);
    }
}

void AccountHelper::updateAccountData(const Tp::AccountPtr &account, AccountHelper::AccountModelColumn column)
{
    const int row = m_suitableAccounts.indexOf(account);
    const int columnInt = columnToInt(column);
    QStandardItem *item = m_accountsModel->item(row, columnInt);
    updateModelItemData(item, account, columnInt);
}

void AccountHelper::updateModelItemData(QStandardItem *item, const Tp::AccountPtr &account, int columnHint)
{
    if (columnHint < 0) {
        columnHint = item->column();
    }
    AccountModelColumn column = columnFromInt(columnHint);

    switch (column) {
    case AccountModelColumn::AccountId:
        item->setText(account->uniqueIdentifier());
        break;
    case AccountModelColumn::AccountEnabled:
        item->setText(account->isEnabled() ? tr("Enabled") : tr("Disabled"));
        break;
    case AccountModelColumn::AccountValid:
        item->setText(account->isValidAccount() ? tr("Valid") : tr("Invalid"));
        break;
    case AccountModelColumn::ColumnsCount:
    case AccountModelColumn::Invalid:
        // Invalid
        break;
    }
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
    m_allAccounts = m_accountManager->allAccounts();
    updateSuitableAccounts();
}

void AccountHelper::onNewAccount(const Tp::AccountPtr &account)
{
    m_allAccounts = m_accountManager->allAccounts();
    updateSuitableAccounts();
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
    connect(operation, &Tp::PendingOperation::finished,
            this, &AccountHelper::onCurrentAccountParametersChanged);
}

void AccountHelper::onCurrentAccountParametersChanged(Tp::PendingOperation *operation)
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    if (!m_currentAccount->isValidAccount()) {
        qCCritical(lcSimpleAccountHelper) << __func__ << "The account is still invalid.";

        disconnectAccount(currentAccountId());
        return;
    }

    if (!m_currentAccount->isEnabled()) {
        enableCurrentAccount();
        return;
    }
    onAccountSetEnableFinished();

    activateCurrentAccount();
}

void AccountHelper::enableCurrentAccount()
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    Tp::PendingOperation *operation = m_currentAccount->setEnabled(true);
    connect(operation, &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountSetEnableFinished);
}

void AccountHelper::onAccountSetEnableFinished(Tp::PendingOperation *operation)
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    if (operation) {
        if (operation->isError()) {
            qCWarning(lcSimpleAccountHelper) << __func__
                                             << operation->errorName() << operation->errorMessage();
            return;
        }
    }

    if (!m_currentAccount->isEnabled()) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "The account is still disabled.";
        return;
    }
}

void AccountHelper::onAccountStateChanged()
{
    Tp::Account *account = qobject_cast<Tp::Account *>(sender());
    if (!account) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Invalid call";
        return;
    }

    qCDebug(lcSimpleAccountHelper) << __func__ << account->uniqueIdentifier()
                                   << "enabled:" << account->isEnabled();
    updateAccountData(Tp::AccountPtr(account), AccountModelColumn::AccountEnabled);

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
    updateAccountData(Tp::AccountPtr(account), AccountModelColumn::AccountValid);

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
