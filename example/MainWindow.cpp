#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "CContactsModel.hpp"

#include "CComboBoxDelegate.hpp"

#include "protocol.h"

#include <TelepathyQt/BaseConnectionManager>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Debug>
#include <TelepathyQt/Types>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_protocol(0)
{
    Tp::registerTypes();
    Tp::enableDebug(true);
    Tp::enableWarnings(true);

    ui->setupUi(this);

    m_contactsModel = new CContactsModel(this);
    ui->contactsView->setModel(m_contactsModel);

    ui->contactsView->setItemDelegateForColumn(1, new CComboBoxDelegate(this));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_registerButton_clicked(bool checked)
{
    if (checked == bool(m_protocol)) {
        return;
    }

    static Tp::BaseProtocolPtr proto;
    static Tp::BaseConnectionManagerPtr cm;

    if (checked) {
        proto = Tp::BaseProtocol::create<SimpleProtocol>(QDBusConnection::sessionBus(), ui->protocolName->text());
        cm = Tp::BaseConnectionManager::create(QDBusConnection::sessionBus(), ui->cmNameEdit->text());

        m_protocol = static_cast<SimpleProtocol*> (proto.data());
        m_protocol->setConnectionManagerName(cm->name());
        m_protocol->setEnglishName(ui->displayNameEdit->text());
        m_protocol->setIconName(ui->iconEdit->text());
        m_protocol->setVCardField(ui->vcardField->text());

        connect(m_protocol, SIGNAL(messageReceived(QString,QString)), SLOT(whenMessage(QString,QString)));

        m_contactsModel->setProtocol(m_protocol);

        cm->addProtocol(proto);
        cm->registerObject();
    } else {
        proto->deleteLater();
        cm->deleteLater();
        m_protocol = 0;
    }
}

void MainWindow::on_sendMessageButton_clicked()
{
    if (!m_protocol) {
        return;
    }

    QString sender = ui->senderName->text();
    QString message = ui->messageEdit->toPlainText();

    ui->messageEdit->clear();

    whenMessage(sender, message, /* fromSelfcontact */ false /* Here we send message from someone to ourself */ );

    m_protocol->sendMessage(sender, message);
}

void MainWindow::whenMessage(QString sender, QString message, bool fromSelfcontact)
{
    m_contactsModel->ensureContact(sender);

    if (sender == ui->senderName->text()) {
        if (fromSelfcontact) {
            ui->messagesLog->appendPlainText(">" + message);
        } else {
            ui->messagesLog->appendPlainText("<" + message);
        }
    }

    if (fromSelfcontact) {
        ui->allMessagesLog->appendPlainText("Message to " + sender + "\n");
    } else {
        ui->allMessagesLog->appendPlainText("Message from " + sender + "\n");
    }
    ui->allMessagesLog->appendPlainText(message);
}

void MainWindow::on_addContactButton_clicked()
{
    QString contact = ui->addContactNameLineEdit->text();
    if (contact.isEmpty()) {
        return;
    }

    m_protocol->addContact(contact);
    m_contactsModel->ensureContact(contact);
    ui->addContactNameLineEdit->clear();
}
