#ifndef CONTACTS_MODEL_HPP
#define CONTACTS_MODEL_HPP

#include <QAbstractTableModel>
#include <QList>

struct SContact {
    QString identifier;
    QString presence;
};

namespace SimpleCM {

class Service;

} // SimpleCM

class ContactsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns {
        Identifier,
        Presence,
        ColumnsCount
    };

    explicit ContactsModel(QObject *parent = nullptr);
    void setService(SimpleCM::Service *service);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void ensureContact(const QString &identifier);

    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    int addContact(const QString identifier);

    SimpleCM::Service *m_service = nullptr;
    QList<SContact> m_contacts;
};

#endif // CONTACTS_MODEL_HPP
