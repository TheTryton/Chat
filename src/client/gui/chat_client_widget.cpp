#include <client/gui/chat_client_widget.hpp>

CLIENT_NAMESPACE_BEGIN
GUI_NAMESPACE_BEGIN

QChatClientWidget::QChatClientWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    setWindowTitle("ChatClient");

    QObject::connect(chatInput, &QLineEdit::returnPressed, this, &QChatClientWidget::onReturnPressedChatInput);
    QObject::connect(quitButton, &QPushButton::released, this, &QChatClientWidget::onQuitPressed);
    QObject::connect(disconnectButton, &QPushButton::released, this, &QChatClientWidget::onDisconnectPressed);
    QObject::connect(connectButton, &QPushButton::released, this, &QChatClientWidget::onConnectPressed);
    QObject::connect(backButton, &QPushButton::released, this, &QChatClientWidget::onBackPressed);
    QObject::connect(loginButton, &QPushButton::released, this, &QChatClientWidget::onLoginPressed);
    QObject::connect(showLogButton0, &QPushButton::released, this, &QChatClientWidget::onLogPressed);
    QObject::connect(showLogButton1, &QPushButton::released, this, &QChatClientWidget::onLogPressed);
    QObject::connect(nicknameInput, &QLineEdit::returnPressed, this, &QChatClientWidget::onReturnPressedClientName);
    QObject::connect(serverAddressInput, &QLineEdit::returnPressed, this, &QChatClientWidget::onConnectPressed);
    QObject::connect(participantNameInput, &QLineEdit::returnPressed, this, &QChatClientWidget::onLoginPressed);

    onStateChanged(logic::QClientLogic::State::disconnected);
    connectProgressBar->setMaximum(5);
}

void QChatClientWidget::onStateChanged(const logic::QClientLogic::State& state)
{
    switch(state)
    {
        case logic::QClientLogic::State::disconnected:{
            mainPages->setCurrentIndex(0);
            connectionPages->setCurrentIndex(0);

            chatInput->clear();
            chatBox->clear();
            nicknameInput->clear();
            participantNameInput->clear();
            serverAddressInput->setEnabled(true);
            connectButton->setEnabled(true);
            connectProgressBar->setValue(0);
            connectProgressBar->setFormat("%v/%m disconnected");
        }break;
        case logic::QClientLogic::State::resolving:{
            clearError();
            mainPages->setCurrentIndex(0);
            connectionPages->setCurrentIndex(0);

            serverAddressInput->setEnabled(false);
            connectButton->setEnabled(false);
            connectProgressBar->setValue(1);
            connectProgressBar->setFormat("%v/%m resolving");
        }break;
        case logic::QClientLogic::State::connecting:{
            clearError();
            mainPages->setCurrentIndex(0);
            connectionPages->setCurrentIndex(0);

            serverAddressInput->setEnabled(false);
            connectButton->setEnabled(false);
            connectProgressBar->setValue(2);
            connectProgressBar->setFormat("%v/%m connecting");
        }break;
        case logic::QClientLogic::State::connected:{
            mainPages->setCurrentIndex(0);
            connectionPages->setCurrentIndex(1);

            participantNameInput->setEnabled(true);
            loginButton->setEnabled(true);
            backButton->setEnabled(true);
            connectProgressBar->setValue(3);
            connectProgressBar->setFormat("%v/%m connected");
        }break;
        case logic::QClientLogic::State::logging_in:{
            clearError();
            mainPages->setCurrentIndex(0);
            connectionPages->setCurrentIndex(1);

            participantNameInput->setEnabled(false);
            loginButton->setEnabled(false);
            backButton->setEnabled(false);
            connectProgressBar->setValue(4);
            connectProgressBar->setFormat("%v/%m logging in");
        }break;
        case logic::QClientLogic::State::logged_in:{
            mainPages->setCurrentIndex(1);

            connectProgressBar->setValue(5);
            connectProgressBar->setFormat("%v/%m logged in");
        }break;
    }
}

void QChatClientWidget::onErrorOccurred(const boost::system::error_code& error)
{
    qDebug() << QString::fromStdString(error.message());
}

void QChatClientWidget::onFailedToConnect(const boost::system::error_code& error)
{
    onErrorOccurred(error);
    errorLabel->setText(QString::fromStdString(error.message()));
}

void QChatClientWidget::onFailedToLogIn(const messages::login_error& reason)
{
    QString reason_str = "unknown";
    switch(reason)
    {
        case messages::login_error::duplicate_name:
            reason_str = "duplicate name";
            break;
        case messages::login_error::too_many_clients:
            reason_str = "too many clients";
            break;
    }
    reason_str = "Failed to log in: " + reason_str;
    qDebug() << reason_str;
    errorLabel->setText(reason_str);
}

void QChatClientWidget::onConnectionLost(const boost::system::error_code& error)
{
    onErrorOccurred(error);
    errorLabel->setText(QString::fromStdString(error.message()));
}

void QChatClientWidget::onFailedToChangeClientName(const messages::login_error& reason)
{
    QString reason_str = "unknown";
    switch(reason)
    {
        case messages::login_error::duplicate_name:
            reason_str = "duplicate name";
            break;
        case messages::login_error::too_many_clients:
            reason_str = "too many clients";
            break;
    }
    reason_str = "Failed to log in: " + reason_str;
    qDebug() << reason_str;
    participantNameInput->setText(currentParticipantName);
}

void QChatClientWidget::clearError()
{
    errorLabel->clear();
}

void QChatClientWidget::onConnectPressed()
{
    emit connectionRequested(serverAddressInput->text());
}
void QChatClientWidget::onDisconnectPressed()
{
    emit disconnectRequested();
}
void QChatClientWidget::onBackPressed()
{
    emit disconnectRequested();
}
void QChatClientWidget::onLoginPressed()
{
    emit loginRequested(participantNameInput->text());
}
void QChatClientWidget::onQuitPressed()
{
    emit quitPressed();
}
void QChatClientWidget::onLogPressed()
{
    emit openConsoleLogPressed();
}
void QChatClientWidget::onReturnPressedClientName()
{
    emit nameChangeRequested(nicknameInput->text());
}
void QChatClientWidget::onReturnPressedChatInput()
{
    auto messageToSend = chatInput->text();
    chatInput->clear();
    emit sendMessageRequested(messageToSend);
}
void QChatClientWidget::onForcedRefreshParticipantList()
{
    emit forceParticipantsListRequested();
}

void QChatClientWidget::onParticipantsListChanged(const data::QChatParticipantList& participants)
{
    participantsList->clear();
    std::for_each(std::begin(participants), std::end(participants), [this](const auto& participant){
        participantsList->addItem(participant.name());
    });
}

void QChatClientWidget::onClientNameChanged(const QString& newName)
{
    currentParticipantName = newName;
    nicknameInput->setText(newName);
}

void QChatClientWidget::onClientUuidChanged(const boost::uuids::uuid& newUuid)
{
    // dummy
}

void QChatClientWidget::onMessageReceived(const data::QChatParticipant& participant, const QString& message)
{
    QString uuidStringified = QString::fromStdString(boost::lexical_cast<std::string>(participant.uuid()));
    QString appendedMessage = "[" + participant.name() + "{" + uuidStringified + "}" + "]: " + message;
    chatBox->append(appendedMessage);
}

GUI_NAMESPACE_END
CLIENT_NAMESPACE_END