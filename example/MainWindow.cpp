#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "CContactsModel.hpp"
#include "CComboBoxDelegate.hpp"
#include "PresetsLoader.hpp"

#include "simpleservice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_service(new SimpleCM::Service(this))
{
    ui->setupUi(this);

    m_contactsModel = new CContactsModel(this);
    ui->contactsView->setModel(m_contactsModel);

    ui->contactsView->setItemDelegateForColumn(1, new CComboBoxDelegate(this));

    m_contactsModel->setService(m_service);

    setupPresets();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_registerButton_clicked(bool checked)
{
    if (m_service->isRunning()) {
        // We don't support shutdown (yet)
        return;
    }

    if (checked) {
        startService(ui->managerName->text(), ui->protocolName->text());
        connect(m_service, &SimpleCM::Service::messageSent, this, &MainWindow::addMessageFromSelfContact);
    } else {
        stopService();
        disconnect(m_service, &SimpleCM::Service::messageSent, this, &MainWindow::addMessageFromSelfContact);
    }
}

void MainWindow::on_contactListAddContact_clicked()
{
    QString contact = ui->addContactNameLineEdit->text();
    if (contact.isEmpty()) {
        return;
    }

    m_contactsModel->ensureContact(contact);
    ui->addContactNameLineEdit->clear();
}

void MainWindow::on_sendMessageButton_clicked()
{
    if (!m_service->isRunning()) {
        return;
    }

    QString sender = ui->messagingSenderName->text();
    QString message = ui->messageEdit->toPlainText();

    ui->messageEdit->clear();

    addMessage(sender, message);
}

void MainWindow::addMessageFromSelfContact(const SimpleCM::Message &message)
{
    QString peerContact;
    if (message.to.type == SimpleCM::Peer::Type::Contact) {
        peerContact = message.to.identifier;
        m_contactsModel->ensureContact(peerContact);
    }

    if (peerContact == ui->messagingSenderName->text()) {
        ui->messagesLog->appendPlainText(">" + message.text);
    }

    ui->allMessagesLog->appendPlainText("Message to " + message.to.identifier + "\n");
    ui->allMessagesLog->appendPlainText(message.text);
}

void MainWindow::addMessage(QString sender, QString text)
{
    m_contactsModel->ensureContact(sender);

    if (sender == ui->messagingSenderName->text()) {
        ui->messagesLog->appendPlainText("<" + text);
    }
    ui->allMessagesLog->appendPlainText("Message from " + sender + "\n");
    ui->allMessagesLog->appendPlainText(text);

    SimpleCM::Message message;
    message.from = sender;
    message.to = SimpleCM::Peer::fromContactId(sender);
    message.text = text;

    m_service->addMessage(message);
}

void MainWindow::startService(const QString &cmName, const QString &protocolName)
{
    m_service->setManagerName(cmName);
    m_service->setProtocolName(protocolName);
    m_service->start();
}

void MainWindow::stopService()
{
    m_service->stop();
}

void MainWindow::on_managerPresetsCombo_currentIndexChanged(int index)
{
    const ManagerPreset &preset = m_presets.at(index);
    ui->managerName->setText(preset.name);
    ui->protocolName->setText(preset.protocol);
}

void MainWindow::setupPresets()
{
    m_presets = PresetsLoader::presets();
    ui->managerPresetsCombo->clear();
    for (const ManagerPreset &preset : m_presets) {
        ui->managerPresetsCombo->addItem(preset.name);
    }
}
