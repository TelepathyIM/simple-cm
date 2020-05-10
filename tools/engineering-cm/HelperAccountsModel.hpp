#ifndef SIMPLECM_HELPER_ACCOUNTS_MODEL
#define SIMPLECM_HELPER_ACCOUNTS_MODEL

#include "AccountsModel.hpp"

class HelperAccountsModel : public AccountsModel
{
public:
    explicit HelperAccountsModel(QObject *parent = nullptr);
    void setFilterRules(const QString &managerName, const QString &protocolName);

    bool filterAcceptAccount(const Tp::AccountPtr &account) const override;
    void sortAccounts(QList<Tp::AccountPtr> *accounts) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    QString m_managerName;
    QString m_protocolName;
};

#endif // SIMPLECM_HELPER_ACCOUNTS_MODEL
