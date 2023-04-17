#ifndef MY_WIDGET_H
#define MY_WIDGET_H

#include "../defs.h"

#include <QWidget>

class My_Widget {
public:
    My_Widget(QWidget* widget): _widget(widget), is_alive(true) {}
    virtual ~My_Widget() { delete _widget; }

    virtual void updateAndRender(r32 dt) = 0;

public:
    inline bool isAlive() const { return is_alive; }
    QWidget* getWidget() { return _widget; }

private:
    QWidget* _widget;

protected:
    bool is_alive;
};

#endif
