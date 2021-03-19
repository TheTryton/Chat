#pragma once

#include <client/logic/client.hpp>
#include <common/gui/input_filter.hpp>
#include <../../../ui/client/ui_client_widget.h>

CLIENT_NAMESPACE_BEGIN
GUI_NAMESPACE_BEGIN

using namespace ::COMMON_NAMESPACE_FULL::GUI_NAMESPACE_FULL;

namespace logic = ::CLIENT_NAMESPACE_FULL::LOGIC_NAMESPACE_FULL;
namespace data = ::COMMON_NAMESPACE_FULL::DATA_NAMESPACE_FULL;
namespace messages = ::COMMON_NAMESPACE_FULL::MESSAGES_NAMESPACE_FULL;

class QChatClientWidget : public QWidget, private Ui::ChatClientWidget
{
    Q_OBJECT
private:
    QString currentParticipantName;
public:
    explicit QChatClientWidget(QWidget* parent = nullptr);
signals:
    void connectionRequested(const QString& serverAddress);
    void loginRequested(const QString& participantName);
    void nameChangeRequested(QString newParticipantName);
    void sendMessageRequested(const QString& message);
    void forceParticipantsListRequested();
    void disconnectRequested();
    void quitPressed();

    void openConsoleLogPressed();
public slots:
    void onStateChanged(const logic::QClientLogic::State& state);
    void onErrorOccurred(const boost::system::error_code& error);
    void onFailedToConnect(const boost::system::error_code& reason);
    void onFailedToLogIn(const messages::login_error& reason);
    void onConnectionLost(const boost::system::error_code& reason);
    void onFailedToChangeClientName(const messages::login_error& reason);
    void onClientNameChanged(const QString& newName);
    void onClientUuidChanged(const boost::uuids::uuid& newUuid);
    void onParticipantsListChanged(const data::QChatParticipantList& participants);
    void onMessageReceived(const data::QChatParticipant& participant, const QString& message);
private slots:
    void onConnectPressed();
    void onDisconnectPressed();
    void onBackPressed();
    void onLoginPressed();
    void onQuitPressed();
    void onLogPressed();
    void onReturnPressedClientName();
    void onReturnPressedChatInput();
    void onForcedRefreshParticipantList();
public:
    void clearError();
};

GUI_NAMESPACE_END
CLIENT_NAMESPACE_END