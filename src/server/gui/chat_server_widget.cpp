#include <server/gui/chat_server_widget.hpp>

SERVER_NAMESPACE_BEGIN
GUI_NAMESPACE_BEGIN

QChatServerWidget::QChatServerWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    setWindowTitle("ChatServer");

    QObject::connect(quitButton, &QPushButton::released, this, &QChatServerWidget::onQuitPressed);
    QObject::connect(stopButton, &QPushButton::released, this, &QChatServerWidget::onStopPressed);
    QObject::connect(startButton, &QPushButton::released, this, &QChatServerWidget::onStartPressed);
    QObject::connect(showLogButton0, &QPushButton::released, this, &QChatServerWidget::onLogPressed);
    QObject::connect(showLogButton1, &QPushButton::released, this, &QChatServerWidget::onLogPressed);
    QObject::connect(serverPortInput, &QLineEdit::returnPressed, this, &QChatServerWidget::onStartPressed);
    QObject::connect(chatInput, &QLineEdit::returnPressed, this, &QChatServerWidget::onReturnPressedChatInput);

    onStateChanged(logic::QServerLogic::State::not_started);
}

void QChatServerWidget::onStateChanged(const logic::QServerLogic::State& state)
{
    switch(state)
    {
        case logic::QServerLogic::State::not_started:{
            mainPages->setCurrentIndex(0);

            chatInput->clear();
            chatBox->clear();
        }break;
        case logic::QServerLogic::State::started:{
            clearError();
            mainPages->setCurrentIndex(1);
        }break;
    }
}

void QChatServerWidget::onErrorOccurred(const boost::system::error_code& error)
{
    qDebug() << QString::fromStdString(error.message());
}

void QChatServerWidget::onFailedToAccept(const boost::system::error_code& error)
{
    onErrorOccurred(error);
    errorLabel->setText(QString::fromStdString(error.message()));
}

void QChatServerWidget::onStopped(const boost::system::error_code& error)
{
    onErrorOccurred(error);
    errorLabel->setText(QString::fromStdString(error.message()));
}

void QChatServerWidget::onParticipantsListChanged(const data::QChatParticipantList& participants)
{
    participantsList->clear();
    std::for_each(std::begin(participants), std::end(participants), [this](const auto& participant){
        participantsList->addItem(participant.name());
    });
}

void QChatServerWidget::onMessageReceived(const data::QChatParticipant& participant, const QString& message)
{
    QString uuidStringified = QString::fromStdString(boost::lexical_cast<std::string>(participant.uuid()));
    QString appendedMessage = "[" + participant.name() + "{" + uuidStringified + "}" + "]: " + message;
    chatBox->append(appendedMessage);
}

void QChatServerWidget::onStartPressed()
{
    emit startRequested(serverPortInput->text());
}
void QChatServerWidget::onStopPressed()
{
    emit stopRequested();
}
void QChatServerWidget::onQuitPressed()
{
    emit quitPressed();
}
void QChatServerWidget::onLogPressed()
{
    emit openConsoleLogPressed();
}
void QChatServerWidget::onReturnPressedChatInput()
{
    auto messageToSend = chatInput->text();
    chatInput->clear();
    emit sendMessageRequested(messageToSend);
}

void QChatServerWidget::clearError()
{
    errorLabel->clear();
}

GUI_NAMESPACE_END
SERVER_NAMESPACE_END