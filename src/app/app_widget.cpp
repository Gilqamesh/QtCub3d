#include "app_widget.h"

#include <QElapsedTimer>
#include <QThread>
#include <QEventLoop>
#include <QMenuBar>
#include <QPushButton>
#include <QAction>
#include <QPixmap>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QScrollBar>

#include "../dialogs/newmapdialog.h"

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

    QMenu* file_menu = menu_bar->addMenu(QIcon(QPixmap(PROJECT_DIR"assets/FileIcon.png")), "File");
    file_menu->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QCheckBox* check_box_center_editor = new QCheckBox(this);
    app_layout->addWidget(check_box_center_editor, 0, 1);
    app_layout->setGeometry(QRect(0, 0, 20, 20));
    check_box_center_editor->setText("Center around player");
    check_box_center_editor->setCheckState(Qt::Checked);
    check_box_center_editor->hide();
    auto center_around_player_lambda = [this](Map_Editor_View* editor, Map_Model* map_model) {
        v2<i32> camera_p((i32) map_model->camera.p.x, map_model->camera.p.y);
        editor->centerAround(map_model->index(camera_p.y, camera_p.x));
        connect(
            map_model, &Map_Model::cameraCellChanged,
            editor, &Map_Editor_View::centerAround
        );
    };
    center_around_player_lambda(_editor, _map_model);
    connect(
        check_box_center_editor, &QCheckBox::stateChanged,
        _editor, [this, check_box_center_editor, center_around_player_lambda](int check_box_state) {
            if (check_box_state == Qt::Unchecked) {
                disconnect(
                    _map_model, &Map_Model::cameraCellChanged,
                    _editor, &Map_Editor_View::centerAround
                );
            } else if (check_box_state == Qt::Checked) {
                center_around_player_lambda(_editor, _map_model);
            }
        }
    );
    check_box_center_editor->setCheckState(Qt::Checked);

    QPushButton* push_button_close_editor = new QPushButton(this);
    app_layout->addWidget(push_button_close_editor, 0, 2);
    push_button_close_editor->setIcon(QIcon(QPixmap(PROJECT_DIR"assets/CloseIcon.png")));
    push_button_close_editor->setText("Close editor");
    push_button_close_editor->setGeometry(QRect(0, 0, 20, 20));
    push_button_close_editor->hide();
    connect(
        push_button_close_editor, &QPushButton::clicked,
        _editor, [this, push_button_close_editor, check_box_center_editor, app_layout]() {
            push_button_close_editor->hide();
            check_box_center_editor->hide();
            _editor->hide();

            app_layout->setColumnStretch(0, 1);
            app_layout->setColumnStretch(1, 0);
            app_layout->setColumnStretch(2, 0);
        }
    );

    QAction* new_map_action = new QAction("New map", file_menu);
    file_menu->addAction(new_map_action);
    connect(
        new_map_action, &QAction::triggered,
        this, [this]() {
            v2<u32> newMapWidthInterval(3, 100);
            v2<u32> newMapHeightInterval(3, 100);
            NewMapDialog* dialog = new NewMapDialog(newMapWidthInterval, newMapHeightInterval, this);
            dialog->setModal(true);
            dialog->show();
            connect(
                dialog, &NewMapDialog::valuesChanged,
                this, [this](i32 width, i32 height) {
                    _map_model->newMap(width, height);
                }
            );
        }
    );

    QAction* editor_open_action = new QAction("Open editor", file_menu);
    file_menu->addAction(editor_open_action);
    connect(
        editor_open_action, &QAction::triggered,
        _editor, [this, push_button_close_editor, check_box_center_editor, app_layout]() {
            push_button_close_editor->show();
            check_box_center_editor->show();
            _editor->show();

            app_layout->setColumnStretch(0, 5);
            app_layout->setColumnStretch(1, 3);
            app_layout->setColumnStretch(2, 2);
        }
    );

    QAction* load_map_action = new QAction("Load map from file", file_menu);
    file_menu->addAction(load_map_action);
    connect(
        load_map_action, &QAction::triggered,
        this, [this]() {
            QString openedFileName = QFileDialog::getOpenFileName(
                this,
                "Open Cub Map",
                "",
                "Cub map file (*cub)"
            );
            if (openedFileName.isEmpty() == false) {
                try {
                    _map_model->readMap(openedFileName.toStdString());
                } catch (const std::exception& err) {
                    QMessageBox::information(this, "Error during loading the map", err.what());
                }
            }
        }
    );
    
    QAction* save_map_action = new QAction("Save map to file", file_menu);
    file_menu->addAction(save_map_action);
    connect(
        save_map_action, &QAction::triggered,
        this, [this]() {
            QString savedFileName = QFileDialog::getSaveFileName(
                this,
                "Save Cub Map",
                "",
                "Cub map file (*.cub)"
            );
            if (savedFileName.isEmpty() == false) {
                try {
                    _map_model->saveMap(savedFileName.toStdString());
                } catch (const std::exception& err) {
                    QMessageBox::information(this, "Error during saving the map", err.what());
                }
            }
        }
    );

    QAction* quit_action = new QAction("Quit", file_menu);
    file_menu->addAction(quit_action);
    connect(
        quit_action, &QAction::triggered,
        this, &App_Widget::destroy
    );
}

void App_Widget::initializeAppLayout(v2<u32> dims) {
    QGridLayout* app_layout = new QGridLayout(this);
    setLayout(app_layout);
    app_layout->setGeometry(QRect(0, 0, dims.x, dims.y));

    initializeMenuBar(app_layout);

    app_layout->addWidget(_renderer, 1, 0);
    app_layout->addWidget(_editor, 1, 1, 1, 2);
    _editor->hide();

    app_layout->setColumnStretch(0, 1);
    app_layout->setColumnStretch(1, 0);
    app_layout->setColumnStretch(2, 0);
}

App_Widget::App_Widget(v2<u32> dims, Map_Model* map_model)
    : _map_model(map_model),
    _renderer(new Renderer_Widget(map_model)),
    _editor(new Map_Editor_View(map_model)) {

    _is_alive = true;

    setGeometry(0, 0, dims.x, dims.y);

    initializeAppLayout(dims);
}

App_Widget::~App_Widget() {
    delete _renderer;
    delete _editor;
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

void App_Widget::closeEvent(QCloseEvent *event) {
    destroy();
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
