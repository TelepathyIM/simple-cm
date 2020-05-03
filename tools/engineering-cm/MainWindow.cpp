#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "CContactsModel.hpp"
#include "CComboBoxDelegate.hpp"
#include "PresetsLoader.hpp"

#ifndef SIMPLECM_ENABLE_LOWLEVEL_API
#define SIMPLECM_ENABLE_LOWLEVEL_API
#endif

#include <TelepathyQt/BaseProtocol>

#include <SimpleCM/Chat>
#include <SimpleCM/Message>
#include <SimpleCM/Service>
#include <SimpleCM/ServiceLowLevel>

#include <QCompleter>

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

    QCompleter *contactsCompleter = new QCompleter(this);
    contactsCompleter->setModel(m_contactsModel);
    contactsCompleter->setCompletionColumn(CContactsModel::Columns::Identifier);
    ui->messagingSenderName->setCompleter(contactsCompleter);
    ui->messagingSenderName->installEventFilter(this);

    ui->accountsView->setModel(m_accountHelper->accountsModel());
    ui->accountsView->setColumnWidth(0, 240);
    connect(ui->accountsView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateAccountControls);

    setupPresets();
    updateTabsState();
    updateAccountControls();
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
    if (checked) {
        ui->registerButton->setText(tr("Stop the manager"));
        startService(ui->managerName->text(), ui->protocolName->text());
        connect(m_service, &SimpleCM::Service::newMessage, this, &MainWindow::onNewMessage);
    } else {
        ui->registerButton->setText(tr("Register the manager"));
        stopService();
        disconnect(m_service, &SimpleCM::Service::newMessage, this, &MainWindow::onNewMessage);
    }

    updateTabsState();
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

void MainWindow::on_messagingSendJson_clicked()
{
    if (!m_service->isRunning()) {
        return;
    }

    QString targetId = ui->messagingSenderName->text();
    QString json = ui->messageEdit->toPlainText();

    ui->messageEdit->clear();

    const SimpleCM::Chat peer = SimpleCM::Chat::fromContactId(targetId);
    m_service->lowLevel()->sendJsonMessage(peer, json.toUtf8());

    SimpleCM::Message message;
    message.from = targetId;
    message.chat = SimpleCM::Chat::fromContactId(targetId);
    message.text = QLatin1String("<JSON>");

    logMessage(message);
}

void MainWindow::onNewMessage(const SimpleCM::Message &message)
{
    logMessage(message);
}

void MainWindow::addMessage(const QString &targetContact, const QString &text)
{
    SimpleCM::Message message;
    message.from = targetContact;
    message.chat = SimpleCM::Chat::fromContactId(targetContact);
    message.text = text;

    m_service->addMessage(message);
}

void MainWindow::logMessage(const SimpleCM::Message &message)
{
    QString peerContact;
    if (message.chat.type == SimpleCM::Chat::Type::Contact) {
        peerContact = message.chat.identifier;
        m_contactsModel->ensureContact(peerContact);
    }

    QString logText = tr("Message in chat with %1 from %2\n").arg(peerContact, message.from);
    ui->allMessagesLog->appendPlainText(logText);

    if (ui->messagingSenderName->text() == peerContact) {
        bool fromRemoteUser = message.chat.identifier == message.from;
        if (fromRemoteUser) {
            ui->messagesLog->appendPlainText("< " + message.text);
        } else {
            ui->messagesLog->appendPlainText("> " + message.text);
        }
    }
}

void MainWindow::startService(const QString &cmName, const QString &protocolName)
{
    m_service->setManagerName(cmName);
    m_service->setProtocolName(protocolName);

    m_service->prepare();
    SimpleCM::ServiceLowLevel *lowLevel = m_service->lowLevel();
    Tp::BaseProtocolPtr protocol = lowLevel->getProtocol();
    protocol->setVCardField(ui->protocolAddressibleVCardFields->text());
    protocol->setEnglishName(ui->protocolDisplayName->text());
    protocol->setIconName(ui->protocolIcon->text());

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
    ui->protocolDisplayName->setText(preset.protocolDisplayName);
    ui->protocolIcon->setText(preset.protocolIcon);
    ui->protocolAddressibleVCardFields->setText(preset.addressableVCardFields.join(QLatin1Char(';')));
}

void MainWindow::setupPresets()
{
    m_presets = PresetsLoader::presets();
    ui->managerPresetsCombo->clear();
    for (const ManagerPreset &preset : m_presets) {
        ui->managerPresetsCombo->addItem(preset.name);
    }
}

void MainWindow::updateTabsState()
{
    ui->tabAccountHelper->setEnabled(m_service->isRunning());
}

void MainWindow::updateAccountControls()
{
    const QString selectedAccount = getSelectedAccount();
    const bool hasSelectedAccount = !selectedAccount.isEmpty();
    ui->removeAccount->setEnabled(hasSelectedAccount);
    if (hasSelectedAccount && m_service->isRunning()) {
        if (selectedAccount == m_accountHelper->currentAccountId()) {
            const AccountHelper::AccountStatus status = m_accountHelper->currentAccountStatus();
            ui->connectAccount->setEnabled(status != AccountHelper::AccountStatus::Connected);
            ui->disconnectAccount->setEnabled(status == AccountHelper::AccountStatus::Connected);
        } else {
            if (m_accountHelper->currentAccountStatus() == AccountHelper::AccountStatus::NoAccount) {
                ui->connectAccount->setEnabled(true);
            }
            ui->disconnectAccount->setEnabled(false);
        }
    } else {
        ui->connectAccount->setEnabled(false);
        ui->disconnectAccount->setEnabled(false);
    }
}

QString MainWindow::getSelectedAccount() const
{
    if (!ui->accountsView->model()) {
        return QString();
    }
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

void MainWindow::on_disconnectAccount_clicked()
{
    const QString accountId = getSelectedAccount();
    m_accountHelper->disconnectAccount(accountId);
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
    updateAccountControls();

    const AccountHelper::AccountStatus status = m_accountHelper->currentAccountStatus();
    const QString statusText = accountStatusToString(status);
    ui->currentAccountStatus->setText(statusText);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->messagingSenderName) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            if (ui->messagingSenderName->text().isEmpty()) {
                ui->messagingSenderName->completer()->complete();
                event->accept();
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
