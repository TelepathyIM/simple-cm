#include "HelperAccountsModel.hpp"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcSimpleAccountModel, "simple.accountModel", QtDebugMsg)

HelperAccountsModel::HelperAccountsModel(QObject *parent)
    : AccountsModel(parent)
{
}

void HelperAccountsModel::setFilterRules(const QString &managerName, const QString &protocolName)
{
    m_managerName = managerName;
    m_protocolName = protocolName;

    invalidateFilter();
}

bool HelperAccountsModel::filterAcceptAccount(const Tp::AccountPtr &account) const
{
    if (account->protocolName() != m_protocolName) {
        return false;
    }
    if (account->cmName() != m_managerName) {
        return false;
    }
    qCDebug(lcSimpleAccountModel) << __func__
                                     << "Suitable account:" << account->uniqueIdentifier();

    return true;
}

void HelperAccountsModel::sortAccounts(QList<Tp::AccountPtr> *accounts) const
{
    const auto comparator = [](const Tp::AccountPtr &left, const Tp::AccountPtr &right) {
        return left->uniqueIdentifier() < right->uniqueIdentifier();
    };
    std::sort(accounts->begin(), accounts->end(), comparator);
}

QVariant HelperAccountsModel::data(const QModelIndex &index, int role) const
{
    const QVariant value = AccountsModel::data(index, role);
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        break;
    default:
        return value;
    }

    const Column column = columns().at(index.column());
    switch (column) {
    case Column::Enabled:
        return value.toBool() ? tr("Enabled") : tr("Disabled");
    case Column::Valid:
        return value.toBool() ? tr("Valid") : tr("Invalid");
    default:
        break;
    }

    return value;
}
