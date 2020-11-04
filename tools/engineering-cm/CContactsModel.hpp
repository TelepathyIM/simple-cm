#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractTableModel>
#include <QList>

struct SContact {
    QString identifier;
    QString presence;
};

namespace SimpleCM {

class Service;

} // SimpleCM

class CContactsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class Column {
        Identifier,
        Presence,
        Count,
        Invalid
    };

    explicit CContactsModel(QObject *parent = 0);
    void setService(SimpleCM::Service *service);

    QVector<Column> columns() const;
    void setColumns(const QVector<Column> &columns);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void ensureContact(const QString &identifier);

    Qt::ItemFlags flags(const QModelIndex &index) const;

signals:

public slots:

private:
    int addContact(const QString identifier);
    Column intToColumn(int column) const;

    SimpleCM::Service *m_service = nullptr;
    QVector<Column> m_columns;
    QVector<SContact> m_contacts;

};

inline int CContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_contacts.count();
}

#endif // CONTACTLISTMODEL_H
