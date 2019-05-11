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

        connect(m_protocol, &SimpleProtocol::clientSendMessage, this, &MainWindow::addMessageFromSelfContact);

        m_contactsModel->setProtocol(m_protocol);

        cm->addProtocol(proto);
        cm->registerObject();
    } else {
        cm.reset();
        proto.reset();
        m_protocol = 0;
    }
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

void MainWindow::on_sendMessageButton_clicked()
{
    if (!m_protocol) {
        return;
    }

    QString sender = ui->senderName->text();
    QString message = ui->messageEdit->toPlainText();

    ui->messageEdit->clear();

    addMessage(sender, message);

    m_protocol->addMessage(sender, message);
}

void MainWindow::addMessageFromSelfContact(QString target, QString message)
{
    m_contactsModel->ensureContact(target);

    if (target == ui->senderName->text()) {
        ui->messagesLog->appendPlainText(">" + message);
    }

    ui->allMessagesLog->appendPlainText("Message to " + target + "\n");
    ui->allMessagesLog->appendPlainText(message);
}

void MainWindow::addMessage(QString sender, QString message)
{
    m_contactsModel->ensureContact(sender);

    if (sender == ui->senderName->text()) {
        ui->messagesLog->appendPlainText("<" + message);
    }
    ui->allMessagesLog->appendPlainText("Message from " + sender + "\n");
    ui->allMessagesLog->appendPlainText(message);
}
