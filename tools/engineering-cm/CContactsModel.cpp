#include "CContactsModel.hpp"

#include <SimpleCM/Service>

CContactsModel::CContactsModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_service(0)
{
}

void CContactsModel::setService(SimpleCM::Service *service)
{
    m_service = service;
}

QVector<CContactsModel::Column> CContactsModel::columns() const
{
    return m_columns;
}

void CContactsModel::setColumns(const QVector<Column> &columns)
{
    m_columns = columns;
}

int CContactsModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_columns.count();
    } else {
        return 0;
    }
}

QVariant CContactsModel::data(const QModelIndex &index, int role) const
{
    if ((role != Qt::DisplayRole) && (role != Qt::EditRole)) {
        return QVariant();
    }

    const Column column = intToColumn(index.column());
    if (column == Column::Invalid) {
        return QVariant();
    }

    int contactIndex = index.row();

    if ((contactIndex < 0) || (contactIndex > rowCount())) {
        return QVariant();
    }

    const SContact &contact = m_contacts.at(contactIndex);

    switch (column) {
    case Column::Identifier:
        return contact.identifier;
    case Column::Handle:
        return contact.handle;
    case Column::Presence:
        return contact.presence;
    case Column::Count:
    case Column::Invalid:
        break;
    }

    return QVariant();
}

QVariant CContactsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const Column column = intToColumn(section);
    if (column == Column::Invalid) {
        return QVariant();
    }

    switch (column) {
    case Column::Identifier:
        return tr("Identifier");
    case Column::Handle:
        return tr("Handle");
    case Column::Presence:
        return tr("Presence");
    case Column::Count:
    case Column::Invalid:
        break;
    }

    return QVariant();
}

bool CContactsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!m_service) {
        return false;
    }

    if (role != Qt::EditRole) {
        return false;
    }

    const Column column = intToColumn(index.column());
    if (column == Column::Invalid) {
        return false;
    }

    int contactIndex = index.row();

    if ((contactIndex < 0) || (contactIndex > rowCount())) {
        return false;
    }

    QString strValue = value.toString();

    switch(column) {
    case Column::Presence:
        if ((strValue != "available")
                && (strValue != "unknown")
                && (strValue != "offline")) {
            return false;
        }

        m_contacts[contactIndex].presence = value.toString();
        m_service->setContactPresence(m_contacts.at(contactIndex).identifier, m_contacts.at(contactIndex).presence);
        return true;
    default:
        return false;
    }
}

void CContactsModel::ensureContact(const QString &identifier)
{
    for (int i = 0; i < m_contacts.count(); ++i) {
        if (m_contacts.at(i).identifier == identifier) {
            return;
        }
    }

    addContact(identifier);
}

Qt::ItemFlags CContactsModel::flags(const QModelIndex &index) const
{
    const Column column = intToColumn(index.column());
    switch(column) {
    case Column::Identifier:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    case Column::Presence:
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    default:
        QAbstractTableModel::flags(index);
    }

    return Qt::NoItemFlags;
}

int CContactsModel::addContact(const QString identifier)
{
    SContact contact;
    contact.identifier = identifier;
    contact.presence = "unknown";
    contact.handle = m_service->addContact(contact.identifier);

    int newContactIndex = m_contacts.count();
    beginInsertRows(QModelIndex(), newContactIndex, newContactIndex);
    m_contacts.append(contact);
    endInsertRows();

    return newContactIndex;
}

CContactsModel::Column CContactsModel::intToColumn(int columnIndex) const
{
    if ((columnIndex < 0) || (columnIndex >= m_columns.count())) {
        return Column::Invalid;
    }
    return m_columns.at(columnIndex);
}
