#include "AccountHelper.hpp"

#include <TelepathyQt/Account>
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingFailure>
#include <TelepathyQt/PendingStringList>

#include <QLoggingCategory>
#include <QStandardItemModel>

Q_LOGGING_CATEGORY(lcSimpleAccountHelper, "simple.accountHelper", QtWarningMsg)

AccountHelper::AccountHelper(QObject *parent)
    : QObject(parent)
{
}

QAbstractItemModel *AccountHelper::accountsModel()
{
    if (!m_accountsModel) {
        m_accountsModel = new QStandardItemModel(this);
        m_accountsModel->setColumnCount(AccountModelSection::SectionsCount);
        updateModelData();
    }

    return m_accountsModel;
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

    if (m_currentAccount->isChangingPresence() || m_currentAccount->isOnline()) {
        requestAccountPresence(m_currentAccount, Tp::ConnectionPresenceTypeOffline);
    }

    m_currentAccount.reset();
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
        { "self_id", "abc_self_id" },
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

    if (m_currentAccount) {
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

    if (!m_currentAccount->isValidAccount()) {
        reValidateAccount();
        return;
    }
    onAccountValid();
}

void AccountHelper::disconnectAccount(const QString &identifier)
{
    Tp::AccountPtr account = getAccountById(identifier);
    if (account->isChangingPresence() || account->isOnline()) {
        requestAccountPresence(account, Tp::ConnectionPresenceTypeOffline);
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
    connect(m_accountManager.data(), &Tp::AccountManager::newAccount,
            this, &AccountHelper::onNewAccount);
}

void AccountHelper::updateSuitableAccounts()
{
    m_suitableAccounts.clear();
    for (const Tp::AccountPtr &account : m_allAccounts) {
        if (account->protocolName() != m_protocolName) {
            continue;
        }
        if (account->cmName() != m_managerName) {
            continue;
        }
        m_suitableAccounts << account;
        qCWarning(lcSimpleAccountHelper) << __func__
                                         << "Suitable account:" << account->uniqueIdentifier();
    }

    updateModelData();
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
        rowItems << new QStandardItem(account->uniqueIdentifier());
        QString validText = account->isValidAccount() ? tr("Valid") : tr("Invalid");
        rowItems << new QStandardItem(validText);
        m_accountsModel->appendRow(rowItems);
    }
}

void AccountHelper::requestAccountOnline()
{
    if (!m_currentAccount->isValid() || !m_currentAccount->isValidAccount()) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "invalid account";
        return;
    }

    if (!m_currentAccount->isEnabled()) {
        qCWarning(lcSimpleAccountHelper) << __func__ << "Error: account"
                                         << m_currentAccount->uniqueIdentifier() << "is disabled";
        return;
    }

    requestAccountPresence(m_currentAccount, Tp::ConnectionPresenceTypeAvailable);
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
    updateSuitableAccounts();
}

void AccountHelper::onAccountCreated(Tp::PendingOperation *operation)
{
    if (operation->isError()) {
        qCWarning(lcSimpleAccountHelper) << operation->errorName() << operation->errorMessage();
        return;
    }
    m_allAccounts = m_accountManager->allAccounts();
    updateSuitableAccounts();
}

void AccountHelper::reValidateAccount()
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    // Update an account parameter to trigger re-validation
    QVariantMap parameters = m_currentAccount->parameters();
    qCDebug(lcSimpleAccountHelper) << m_currentAccount->parameters();

    Tp::PendingOperation *operation = m_currentAccount->updateParameters(parameters, { });
    connect(operation, &Tp::PendingOperation::finished,
            this, &AccountHelper::onAccountValid);
}

void AccountHelper::onAccountValid()
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    if (!m_currentAccount->isValidAccount()) {
        qCCritical(lcSimpleAccountHelper) << __func__ << "The account is still invalid.";
        return;
    }
    updateSuitableAccounts();

    if (!m_currentAccount->isEnabled()) {
        enableAccount();
        return;
    }
    onAccountSetEnableFinished();
}

void AccountHelper::enableAccount()
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
    qCDebug(lcSimpleAccountHelper) << __func__ << m_currentAccount->uniqueIdentifier()
                                   << "enabled:" << m_currentAccount->isEnabled();
    if (m_currentAccount->isEnabled()) {
        onAccountEnabled();
    }
}

void AccountHelper::onAccountEnabled()
{
    qCDebug(lcSimpleAccountHelper) << __func__;
    updateModelData();
    requestAccountOnline();
}
