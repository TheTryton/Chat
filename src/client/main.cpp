#include <QApplication>
#include <client/logic/client.hpp>
#include <client/gui/chat_client_widget.hpp>

namespace gui = ::CLIENT_NAMESPACE_FULL::GUI_NAMESPACE_FULL;
namespace logic = ::CLIENT_NAMESPACE_FULL::LOGIC_NAMESPACE_FULL;

int main(int argc, char* argv[])
{
    QApplication app{argc, argv};
    boost::asio::io_context context;

    gui::QChatClientWidget widget;
    logic::QClientLogic client{context};

    QObject::connect(&client, &logic::QClientLogic::currentStateChanged, &widget, &gui::QChatClientWidget::onStateChanged);
    QObject::connect(&client, &logic::QClientLogic::errorOccured, &widget, &gui::QChatClientWidget::onErrorOccurred);
    QObject::connect(&client, &logic::QClientLogic::failedToConnect, &widget, &gui::QChatClientWidget::onFailedToConnect);
    QObject::connect(&client, &logic::QClientLogic::failedToLogIn, &widget, &gui::QChatClientWidget::onFailedToLogIn);
    QObject::connect(&client, &logic::QClientLogic::connectionLost, &widget, &gui::QChatClientWidget::onConnectionLost);
    QObject::connect(&client, &logic::QClientLogic::failedToChangeClientName, &widget, &gui::QChatClientWidget::onFailedToChangeClientName);
    QObject::connect(&client, &logic::QClientLogic::clientNameChanged, &widget, &gui::QChatClientWidget::onClientNameChanged);
    QObject::connect(&client, &logic::QClientLogic::clientUuidChanged, &widget, &gui::QChatClientWidget::onClientUuidChanged);
    QObject::connect(&client, &logic::QClientLogic::clientsListChanged, &widget, &gui::QChatClientWidget::onParticipantsListChanged);
    QObject::connect(&client, &logic::QClientLogic::chatMessageReceived, &widget, &gui::QChatClientWidget::onMessageReceived);

    QObject::connect(&widget, &gui::QChatClientWidget::connectionRequested, &client, &logic::QClientLogic::resolveAndConnect);
    QObject::connect(&widget, &gui::QChatClientWidget::loginRequested, &client, &logic::QClientLogic::logIn);
    QObject::connect(&widget, &gui::QChatClientWidget::nameChangeRequested, &client, &logic::QClientLogic::changeName);
    QObject::connect(&widget, &gui::QChatClientWidget::sendMessageRequested, &client, &logic::QClientLogic::sendChatMessage);
    QObject::connect(&widget, &gui::QChatClientWidget::forceParticipantsListRequested, &client, &logic::QClientLogic::requestParticipantListUpdate);
    QObject::connect(&widget, &gui::QChatClientWidget::disconnectRequested, &client, &logic::QClientLogic::disconnect);
    QObject::connect(&widget, &gui::QChatClientWidget::quitPressed, &client, &logic::QClientLogic::quit);
    //QObject::connect(&widget, &gui::QChatClientWidget::openConsoleLogPressed, &client, &logic::QClientLogic::logIn);

    widget.show();
    std::thread asio([&context](){
        context.run();
    });
    auto result = app.exec();
    context.stop();
    asio.join();
    return result;
}
