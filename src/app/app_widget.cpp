#include "app_widget.h"

#include <QElapsedTimer>
#include <QThread>
#include <QEventLoop>
#include <QMenuBar>
#include <QPushButton>
#include <QAction>
#include <QPixmap>

App_Widget::App_Widget()
    : My_Widget(new QWidget()) {
    QGridLayout* layout = new QGridLayout(getWidget());
    getWidget()->setLayout(layout);

    QMenuBar* menu_bar = new QMenuBar(getWidget());
    layout->addWidget(menu_bar, 0, 0);

    QMenu* file_menu = menu_bar->addMenu("File");

    QPushButton* exit_button = new QPushButton(getWidget());
    layout->addWidget(exit_button, 0, 3);
    exit_button->setIcon(QIcon(QPixmap("../Cub3d/assets/CloseIcon.png")));
    exit_button->setGeometry(QRect(0, 0, 20, 20));
    QAction* close_action = new QAction(getWidget());
    getWidget()->connect(
        exit_button, &QPushButton::clicked,
        getWidget(), [=]() {
            is_alive = false;
        }
    );
}

QGridLayout* App_Widget::getLayout() {
    return reinterpret_cast<QGridLayout*>(getWidget()->layout());
}

void App_Widget::exec() {
    constexpr r64 frame_budge_fps = 60.0;
    constexpr u64 frame_budget_ns = 1000000000.0 / frame_budge_fps;

    QElapsedTimer timer;
    timer.start();
    u64 frame_time_end_ns = timer.nsecsElapsed();

    QEventLoop events;
    while (isAlive()) {
        u64 frame_time_start_ns = timer.nsecsElapsed();
        r32 dt_s = (frame_time_start_ns - frame_time_end_ns) / 1000000000.0;
        frame_time_end_ns = frame_time_start_ns;

        events.processEvents();
        updateAndRender(dt_s);

        u64 cur_frame_time_end_ns = timer.nsecsElapsed();
        while (cur_frame_time_end_ns - frame_time_start_ns < frame_budget_ns) {
            _mm_pause();
            cur_frame_time_end_ns = timer.nsecsElapsed();
        }
    }
}

void App_Widget::addWidget(My_Widget* widget) {
    widgets.insert(widget);
}

void App_Widget::updateAndRender(r32 dt) {
    auto widget_it = widgets.begin();
    while (widget_it != widgets.end()) {
        if ((*widget_it)->isAlive()) {
            (*widget_it)->updateAndRender(dt);
            ++widget_it;
        } else {
            widget_it = widgets.erase(widget_it);
        }
    }
}
