#ifndef APP_WIDGET_H
#define APP_WIDGET_H

#include <QGridLayout>

#include "../common/mywidget.h"
#include "../defs.h"

#include <unordered_set>

class App_Widget: public My_Widget {
public:
    App_Widget();

    void exec();
    void addWidget(My_Widget* widget);

    QGridLayout* getLayout();

private:
    std::unordered_set<My_Widget*> widgets;

private:
    virtual void updateAndRender(r32 dt) override;
};

#endif
