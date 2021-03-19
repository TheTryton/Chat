#include <QApplication>
#include <server/logic/server.hpp>
#include <server/gui/chat_server_widget.hpp>

namespace gui = ::SERVER_NAMESPACE_FULL::GUI_NAMESPACE_FULL;
namespace logic = ::SERVER_NAMESPACE_FULL::LOGIC_NAMESPACE_FULL;

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    boost::asio::io_context context;

    gui::QChatServerWidget widget;
    logic::QServerLogic server{context};

    QObject::connect(&server, &logic::QServerLogic::currentStateChanged, &widget, &gui::QChatServerWidget::onStateChanged);
    QObject::connect(&server, &logic::QServerLogic::errorOccured, &widget, &gui::QChatServerWidget::onErrorOccurred);
    QObject::connect(&server, &logic::QServerLogic::failedToAccept, &widget, &gui::QChatServerWidget::onFailedToAccept);
    QObject::connect(&server, &logic::QServerLogic::stopped, &widget, &gui::QChatServerWidget::onStopped);
    QObject::connect(&server, &logic::QServerLogic::clientsListChanged, &widget, &gui::QChatServerWidget::onParticipantsListChanged);
    QObject::connect(&server, &logic::QServerLogic::chatMessageReceived, &widget, &gui::QChatServerWidget::onMessageReceived);

    QObject::connect(&widget, &gui::QChatServerWidget::startRequested, &server, &logic::QServerLogic::start);
    QObject::connect(&widget, &gui::QChatServerWidget::stopRequested, &server, &logic::QServerLogic::stop);
    QObject::connect(&widget, &gui::QChatServerWidget::quitPressed, &server, &logic::QServerLogic::quit);
    QObject::connect(&widget, &gui::QChatServerWidget::sendMessageRequested, &server, &logic::QServerLogic::sendChatMessage);
    //QObject::connect(&widget, &gui::QChatServerWidget::openConsoleLogPressed, &server, &logic::QChatServerWidget::logIn);

    widget.show();
    std::thread asio([&context](){
        context.run();
    });
    auto result = app.exec();
    context.stop();
    asio.join();
    return result;
}
