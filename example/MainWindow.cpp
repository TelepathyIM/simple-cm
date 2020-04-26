#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "CContactsModel.hpp"
#include "CComboBoxDelegate.hpp"
#include "PresetsLoader.hpp"

#include "simpleservice.h"

QString MainWindow::accountStatusToString(AccountHelper::AccountStatus status)
{
    switch (status) {
    case AccountHelper::AccountStatus::NoAccount:
        return tr("No account");
    case AccountHelper::AccountStatus::Initialization:
        return tr("Initialization");
    case AccountHelper::AccountStatus::ReValidation:
        return tr("Re-validation");
    case AccountHelper::AccountStatus::Connected:
        return tr("Connected");
    case AccountHelper::AccountStatus::Disconnected:
        return tr("Disconnected");
    }

    return tr("Invalid");
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_service(new SimpleCM::Service(this))
{
    ui->setupUi(this);

    m_accountHelper = new AccountHelper(this);
    connect(m_accountHelper, &AccountHelper::currentAccountIdChanged,
            this, &MainWindow::onCurrentAccountIdChanged);
    connect(m_accountHelper, &AccountHelper::currentAccountStatusChanged,
            this, &MainWindow::onCurrentAccountStatusChanged);

    m_contactsModel = new CContactsModel(this);
    ui->contactsView->setModel(m_contactsModel);

    ui->contactsView->setItemDelegateForColumn(1, new CComboBoxDelegate(this));

    m_contactsModel->setService(m_service);

    ui->accountsView->setModel(m_accountHelper->accountsModel());
    ui->accountsView->setColumnWidth(0, 200);

    setupPresets();
}

MainWindow::~MainWindow()
{
    if (m_service->isRunning()) {
        stopService();
    }
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

    m_accountHelper->setManagerName(cmName);
    m_accountHelper->setProtocolName(protocolName);
    m_accountHelper->start();
}

void MainWindow::stopService()
{
    m_accountHelper->stop();
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

QString MainWindow::getSelectedAccount() const
{
    const QModelIndexList selection = ui->accountsView->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) {
        return QString();
    }

    return getAccountId(selection.constFirst());
}

QString MainWindow::getAccountId(const QModelIndex &accountIndex) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    QModelIndex accountIdIndex = accountIndex.siblingAtColumn(AccountHelper::AccountModelSection::AccountId);
#else
    QModelIndex accountIdIndex = accountIndex.sibling(accountIndex.row(), AccountHelper::AccountModelSection::AccountId);
#endif
    return accountIdIndex.data().toString();
}

void MainWindow::on_addAccount_clicked()
{
    m_accountHelper->addAccount();
}

void MainWindow::on_removeAccount_clicked()
{
    const QString accountId = getSelectedAccount();
    m_accountHelper->removeAccount(accountId);
}

void MainWindow::on_connectAccount_clicked()
{
    const QString accountId = getSelectedAccount();
    m_accountHelper->connectAccount(accountId);
}

void MainWindow::on_accountsView_doubleClicked(const QModelIndex &index)
{
    const QString accountId = getAccountId(index);
    m_accountHelper->connectAccount(accountId);
}

void MainWindow::onCurrentAccountIdChanged()
{
    ui->currentAccountId->setText(m_accountHelper->currentAccountId());
}

void MainWindow::onCurrentAccountStatusChanged()
{
    const AccountHelper::AccountStatus status = m_accountHelper->currentAccountStatus();
    ui->connectAccount->setEnabled(status != AccountHelper::AccountStatus::Connected);
    ui->disconnectAccount->setEnabled(status == AccountHelper::AccountStatus::Connected);

    const QString statusText = accountStatusToString(status);
    ui->currentAccountStatus->setText(statusText);
}
