#pragma once

#include <server/logic/server.hpp>
#include <common/gui/input_filter.hpp>
#include <../../../ui/server/ui_server_widget.h>

SERVER_NAMESPACE_BEGIN
GUI_NAMESPACE_BEGIN

using namespace ::COMMON_NAMESPACE_FULL::GUI_NAMESPACE_FULL;

namespace logic = ::SERVER_NAMESPACE_FULL::LOGIC_NAMESPACE_FULL;
namespace data = ::COMMON_NAMESPACE_FULL::DATA_NAMESPACE_FULL;
namespace messages = ::COMMON_NAMESPACE_FULL::MESSAGES_NAMESPACE_FULL;

class QChatServerWidget : public QWidget, private Ui::ChatServerWidget
{
Q_OBJECT
public:
    explicit QChatServerWidget(QWidget* parent = nullptr);
signals:
    void startRequested(const QString& port);
    void sendMessageRequested(const QString& message);
    void stopRequested();
    void quitPressed();

    void openConsoleLogPressed();
public slots:
    void onStateChanged(const logic::QServerLogic::State& state);
    void onErrorOccurred(const boost::system::error_code& error);
    void onFailedToAccept(const boost::system::error_code& reason);
    void onStopped(const boost::system::error_code& reason);
    void onParticipantsListChanged(const data::QChatParticipantList& participants);
    void onMessageReceived(const data::QChatParticipant& participant, const QString& message);
private slots:
    void onStartPressed();
    void onStopPressed();
    void onQuitPressed();
    void onLogPressed();
    void onReturnPressedChatInput();
public:
    void clearError();
};

GUI_NAMESPACE_END
SERVER_NAMESPACE_END