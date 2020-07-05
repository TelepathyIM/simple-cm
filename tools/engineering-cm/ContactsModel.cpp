#include "ContactsModel.hpp"

#include <SimpleCM/Service>

ContactsModel::ContactsModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void ContactsModel::setService(SimpleCM::Service *service)
{
    m_service = service;
}

int ContactsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnsCount;
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_contacts.count();
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
    if ((role != Qt::DisplayRole) && (role != Qt::EditRole)) {
        return QVariant();
    }

    int section = index.column();
    int contactIndex = index.row();

    if ((contactIndex < 0) || (contactIndex > rowCount())) {
        return QVariant();
    }

    switch (section) {
    case Identifier:
        return m_contacts.at(contactIndex).identifier;
    case Presence:
        return m_contacts.at(contactIndex).presence;
    default:
        break;
    }

    return QVariant();
}

QVariant ContactsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case Identifier:
        return tr("Identifier");
    case Presence:
        return tr("Presence");
    default:
        break;
    }

    return QVariant();
}

bool ContactsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!m_service) {
        return false;
    }

    if (role != Qt::EditRole) {
        return false;
    }

    int section = index.column();
    int contactIndex = index.row();

    if ((contactIndex < 0) || (contactIndex > rowCount())) {
        return false;
    }

    QString strValue = value.toString();

    switch(section) {
    case Presence:
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

void ContactsModel::ensureContact(const QString &identifier)
{
    for (int i = 0; i < m_contacts.count(); ++i) {
        if (m_contacts.at(i).identifier == identifier) {
            return;
        }
    }

    addContact(identifier);
}

Qt::ItemFlags ContactsModel::flags(const QModelIndex &index) const
{
    switch(index.column()) {
    case Identifier:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    case Presence:
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    default:
        QAbstractTableModel::flags(index);
    }

    return Qt::NoItemFlags;
}

int ContactsModel::addContact(const QString identifier)
{
    SContact contact;
    contact.identifier = identifier;
    contact.presence = "unknown";

    int newContactIndex = m_contacts.count();
    beginInsertRows(QModelIndex(), newContactIndex, newContactIndex);
    m_contacts.append(contact);
    endInsertRows();

    m_service->addContact(contact.identifier);

    return newContactIndex;
}