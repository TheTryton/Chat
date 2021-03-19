#include <common/gui/input_filter.hpp>

COMMON_NAMESPACE_BEGIN
GUI_NAMESPACE_BEGIN

QInputEventFilter::QInputEventFilter(QTextEdit* filteredTextEdit, QObject* parent)
    : QObject(parent)
    , filteredTextEdit(filteredTextEdit)
{
}

bool QInputEventFilter::eventFilter(QObject* object, QEvent* event)
{
    if(object == filteredTextEdit && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Enter)
        {
            emit enterPressed();
            return true;
        }
    }
    return false;
}

GUI_NAMESPACE_END
COMMON_NAMESPACE_END