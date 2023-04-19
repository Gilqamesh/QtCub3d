#include "app_widget.h"

#include <QElapsedTimer>
#include <QThread>
#include <QEventLoop>
#include <QMenuBar>
#include <QPushButton>
#include <QAction>
#include <QPixmap>

static inline void setWidgetGeometry(const QWidget* parent, QWidget* child, v2<r32> normalized_top_left_p, v2<r32> normalized_bot_right_p) {
    QRect parent_rect = parent->geometry();
    assert(parent_rect.width() > 0 && parent_rect.height() > 0);
    assert(normalized_bot_right_p.x > normalized_top_left_p.x && normalized_bot_right_p.y > normalized_top_left_p.y);
    child->setGeometry(QRect(
        (i32) (normalized_top_left_p.x * (r32) parent_rect.width()),
        (i32) (normalized_top_left_p.y * (r32) parent_rect.height()),
        (i32) ((normalized_bot_right_p.x - normalized_top_left_p.x) * (r32) parent_rect.width()),
        (i32) ((normalized_bot_right_p.y - normalized_top_left_p.y) * (r32) parent_rect.height())
    ));
}

void App_Widget::initializeMenuBar(QGridLayout* app_layout) {
    QMenuBar* menu_bar = new QMenuBar(this);
    // setWidgetGeometry(this, menu_bar, v2<r32>(0.2f, 0.2f), v2<r32>(0.8f, 0.4f));
    menu_bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    app_layout->addWidget(menu_bar, 0, 0);

    QMenu* file_menu = menu_bar->addMenu(QIcon(QPixmap("../assets/FileIcon.png")), "File");
    file_menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QPushButton* close_editor = new QPushButton(this);
    app_layout->addWidget(close_editor, 0, 2);
    close_editor->setIcon(QIcon(QPixmap("../assets/CloseIcon.png")));
    close_editor->setGeometry(QRect(0, 0, 20, 20));
    close_editor->hide();
    QAction* close_action = new QAction(_editor);
    connect(
        close_editor, &QPushButton::clicked,
        _editor, [this, close_editor]() {
            close_editor->hide();
            _editor->hide();
        }
    );

    QAction* quit_action = new QAction("Quit", file_menu);
    file_menu->addAction(quit_action);
    connect(
        quit_action, &QAction::triggered,
        this, &App_Widget::destroy
    );

    QAction* editor_open_action = new QAction("Open editor", file_menu);
    file_menu->addAction(editor_open_action);
    connect(
        editor_open_action, &QAction::triggered,
        _editor, [this, close_editor]() {
            close_editor->show();
            _editor->show();
        }
    );
}

void App_Widget::initializeAppLayout(v2<u32> dims) {
    QGridLayout* app_layout = new QGridLayout(this);
    setLayout(app_layout);
    app_layout->setGeometry(QRect(0, 0, dims.x, dims.y));

    initializeMenuBar(app_layout);

    app_layout->addWidget(_renderer, 1, 0);
    app_layout->addWidget(_editor, 1, 2);
    _editor->hide();
}

App_Widget::App_Widget(v2<u32> dims, Renderer_Widget* renderer, Map_Editor_Mvc* editor)
    : _renderer(renderer),
    _editor(editor) {

    _is_alive = true;

    setGeometry(0, 0, dims.x, dims.y);

    initializeAppLayout(dims);
}

void App_Widget::exec() {
    show();

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

void App_Widget::destroy() {
    _is_alive = false;
}

bool App_Widget::isAlive() const {
    return _is_alive;
}

void App_Widget::updateAndRender(r32 dt) {
    _renderer->updateAndRender(dt);
    _editor->update();
}