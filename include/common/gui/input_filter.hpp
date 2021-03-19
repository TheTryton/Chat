#pragma once

#include <common/includes.hpp>

COMMON_NAMESPACE_BEGIN
GUI_NAMESPACE_BEGIN

class QInputEventFilter : public QObject
{
Q_OBJECT
private:
    QTextEdit* filteredTextEdit;
public:
    explicit QInputEventFilter(QTextEdit* filteredTextEdit, QObject* parent = nullptr);
public:
    virtual bool eventFilter(QObject* object, QEvent* event) override;
signals:
    void enterPressed();
};

GUI_NAMESPACE_END
COMMON_NAMESPACE_END