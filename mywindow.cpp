#include "mywindow.h"
#include "./ui_mywindow.h"

#include <QPainter>
#include <QKeyEvent>
#include <QDockWidget>
#include "clamp.h"

MyWindow::MyWindow(const string& cubmap_path)
    : ui(new Ui::MyWindow)
    , cubmap_model(new CubMap_Model(cubmap_path))
    , cubmap_view(new CubMap_View())
    , cubmap_delegate(new CubMap_Delegate())
    , cursor(Qt::CrossCursor)
    , is_alive(true)
{
    ui->setupUi(this);
    showFullScreen();

    this->setBaseSize(width(), height());

    // todo: input
    is_cursor_free = false;
    is_w_down = false;
    is_s_down = false;
    is_a_down = false;
    is_d_down = false;

    // texture loading
    wall_textures[static_cast<u32>(WallTexId::North)] = QPixmap("../Cub3d/assets/North.jpg");
    wall_textures[static_cast<u32>(WallTexId::South)] = QPixmap("../Cub3d/assets/South.jpg");
    wall_textures[static_cast<u32>(WallTexId::West)] = QPixmap("../Cub3d/assets/West.jpg");
    wall_textures[static_cast<u32>(WallTexId::East)] = QPixmap("../Cub3d/assets/East.jpg");

    setCursor(cursor);
    cursor.setPos(screen(), width() / 2, height() / 2);

    cubmap_editor = menuBar()->addMenu("CubMap editor");
    // QAction* cubmap_editor_open_act = new QAction(this);
    // connect(cubmap_editor_open_act, &QAction::triggered, cubmap_view, &CubMap_View::show());
    // cubmap_editor->addAction(cubmap_editor_open_act);

    cubmap_view->setModel(cubmap_model);
    cubmap_view->setItemDelegate(cubmap_delegate);
    cubmap_view->setBaseSize(width() / 2, height() / 2);
}

MyWindow::~MyWindow()
{
    delete ui;
    delete cubmap_model;
    delete cubmap_view;
    delete cubmap_delegate;
}

void MyWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape: {
            is_alive = false;
        } break ;
        case Qt::Key_F1: {
            is_cursor_free = !is_cursor_free;
        } break ;
        case Qt::Key_F2: {
            if (this->menuBar()->isHidden()) {
                this->menuBar()->show();
            } else {
                this->menuBar()->hide();
            }
        }
        case Qt::Key_W: {
            is_w_down = true;
        } break ;
        case Qt::Key_S: {
            is_s_down = true;
        } break ;
        case Qt::Key_A: {
            is_a_down = true;
        } break ;
        case Qt::Key_D: {
            is_d_down = true;
        } break ;
    }
}

void MyWindow::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_W: {
            is_w_down = false;
        } break ;
        case Qt::Key_S: {
            is_s_down = false;
        } break ;
        case Qt::Key_A: {
            is_a_down = false;
        } break ;
        case Qt::Key_D: {
            is_d_down = false;
        } break ;
    }
}

void MyWindow::mouseMoveEvent(QMouseEvent* event) {
}

void MyWindow::closeEvent(QCloseEvent* event) {
    this->is_alive = false;
}

void MyWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    drawFloor(painter);
    drawWall(painter);
    cubmap_view->update();
}

void MyWindow::drawFloor(QPainter& painter) {
    // for (u32 x = 0; x < width(); ++x) {
    // }
}

void MyWindow::drawWall(QPainter& painter) {
    enum class Side {
        Vertical,
        Horizontal
    };
    struct wall_piece {
        v2<i32> top_left;
        v2<i32> bot_right;
        v2<i32> wall_text_interval_x;
        WallTexId wall_tex_id;
    };
    vector<wall_piece> wall_pieces;
    v2<i32> prev_wall_index(-1, -1);
    WallTexId last_wall_tex_id;
    v2<i32> last_bot_right;
    r32 last_wall_tex_x;
    for (u32 col = 0; col < (u32) width(); ++col) {
        r32 camera_x = 2.0 * (r32) col / (r32) width() - 1.0f;
        v2<r32> raydir = cubmap_model->_camera.dir + cubmap_model->_camera.plane * camera_x;
        v2<i32> map_p((i32) cubmap_model->_camera.p.x, (i32) cubmap_model->_camera.p.y);

        constexpr r32 EPSILON = 0.000001f;
        v2<r32> delta_dist;
        if (fabs(raydir.x) < EPSILON) {
            delta_dist.x = INFINITY;
        } else {
            delta_dist.x = fabs(1.0f / raydir.x);
        }
        if (fabs(raydir.y) < EPSILON) {
            delta_dist.y = INFINITY;
        } else {
            delta_dist.y = fabs(1.0f / raydir.y);
        }

        v2<i32> ray_step;
        v2<r32> side_dist;
        if (raydir.x < 0.0f) {
            ray_step.x = -1;
            side_dist.x = (cubmap_model->_camera.p.x - map_p.x) * delta_dist.x;
        } else {
            ray_step.x = 1;
            side_dist.x = (map_p.x + 1.0f - cubmap_model->_camera.p.x) * delta_dist.x;
        }
        if (raydir.y < 0.0f) {
            ray_step.y = -1;
            side_dist.y = (cubmap_model->_camera.p.y - map_p.y) * delta_dist.y;
        } else {
            ray_step.y = 1;
            side_dist.y = (map_p.y + 1.0f - cubmap_model->_camera.p.y) * delta_dist.y;
        }

        CubMap_Model::CubMap_ModelCell hit = cubmap_model->_map[map_p.y][map_p.x];
        Side side = Side::Horizontal;
        while (hit != CubMap_Model::CubMap_ModelCell::Wall) {
            if (side_dist.x < side_dist.y) {
                side_dist.x += delta_dist.x;
                map_p.x += ray_step.x;
                side = Side::Horizontal;
            } else {
                side_dist.y += delta_dist.y;
                map_p.y += ray_step.y;
                side = Side::Vertical;
            }
            hit = cubmap_model->_map[map_p.y][map_p.x];
        }
        r32 perp_wall_dist;
        if (side == Side::Horizontal) {
            perp_wall_dist = side_dist.x - delta_dist.x;
        } else {
            perp_wall_dist = side_dist.y - delta_dist.y;
        }
        r32 line_height = (i32) ((r32) height() / perp_wall_dist);
        v2<i32> vertical_draw_interval_unclamped(
            ((r32) height() - line_height) / 2.0f,
            ((r32) height() + line_height) / 2.0f
        );

        WallTexId wall_tex_id;
        if (side == Side::Vertical) {
            if (ray_step.y == 1) {
                wall_tex_id = WallTexId::North;
            } else {
                wall_tex_id = WallTexId::South;
            }
        } else {
            if (ray_step.x == 1) {
                wall_tex_id = WallTexId::East;
            } else {
                wall_tex_id = WallTexId::West;
            }
        }

        const QPixmap& wall_text = wall_textures[static_cast<u32>(wall_tex_id)];
        r32 wall_x;
        if (side == Side::Horizontal) {
            wall_x = cubmap_model->_camera.p.y + perp_wall_dist * raydir.y;
        } else {
            wall_x = cubmap_model->_camera.p.x + perp_wall_dist * raydir.x;
        }
        wall_x -= floor(wall_x);
        v2<i32> text_dims(wall_text.width(), wall_text.height());
        r32 wall_y = 0.0f;
        if ((i32) line_height > height()) {
            wall_y = (line_height - (r32) height()) / (2.0f * line_height);
        }
        r32 wall_step = (r32) text_dims.y / line_height;
        v2<r32> tex_start_offset(
            (i32) (wall_x * (r32) text_dims.x),
            (i32) (wall_y * (r32) text_dims.y)
        );
        if ((side == Side::Horizontal && raydir.x > 0.0f) ||
            (side == Side::Vertical && raydir.y < 0.0f)
        ) {
            tex_start_offset.x = (r32) text_dims.x - tex_start_offset.x - 1.0f;
        }

        if (prev_wall_index.x == -1 && prev_wall_index.y == -1) {
            prev_wall_index = map_p;
            wall_piece wp;
            wp.top_left = v2<i32>(col, vertical_draw_interval_unclamped.y);
            wp.bot_right = v2<i32>(-1, -1);
            wp.wall_text_interval_x.x = tex_start_offset.x;
            wp.wall_text_interval_x.y = tex_start_offset.y;
            // top_left
            wall_pieces.push_back(wp);
        } else if (!(prev_wall_index.x == map_p.x && prev_wall_index.y == map_p.y && last_wall_tex_id == wall_tex_id)) {
            prev_wall_index = map_p;
            // bot right + tex
            wall_pieces.back().bot_right = last_bot_right;
            wall_pieces.back().wall_tex_id = last_wall_tex_id;
            wall_pieces.back().wall_text_interval_x.y = last_wall_tex_x;
            // start new
            wall_pieces.push_back(wall_piece());
            wall_pieces.back().top_left = v2<i32>(col, vertical_draw_interval_unclamped.y);
            wall_pieces.back().bot_right = v2<i32>(-1, -1);
            wall_pieces.back().wall_text_interval_x.x = tex_start_offset.x;
        }
        last_wall_tex_id = wall_tex_id;
        last_bot_right = v2<i32>(col, vertical_draw_interval_unclamped.x);
        last_wall_tex_x = tex_start_offset.x;
        i32 debug = 0.0f;
        ++debug;
    }
    // last wall piece
    if (wall_pieces.back().bot_right.x == -1 && wall_pieces.back().bot_right.y == -1) {
        wall_pieces.back().bot_right = last_bot_right;
        wall_pieces.back().wall_tex_id = last_wall_tex_id;
        wall_pieces.back().wall_text_interval_x.y = last_wall_tex_x;
    }
    // draw
    for (const auto& wp : wall_pieces) {
        v2<i32> top_right(
            wp.bot_right.x,
            height() - wp.bot_right.y
        );
        v2<i32> bot_left(
            wp.top_left.x,
            height() - wp.top_left.y
        );
        const QPixmap& pixmap = wall_textures[static_cast<u32>(wp.wall_tex_id)];
        painter.setPen(Qt::NoPen);

        QPoint points[4] = {
            QPoint(bot_left.x, bot_left.y),
            QPoint(wp.bot_right.x, wp.bot_right.y),
            QPoint(top_right.x, top_right.y),
            QPoint(wp.top_left.x, wp.top_left.y)
        };
        QTransform brush_transform;
        if (QTransform::quadToQuad(
            QPolygon({
                QPoint(wp.wall_text_interval_x.x, 0),
                QPoint(wp.wall_text_interval_x.y, 0),
                QPoint(wp.wall_text_interval_x.y, pixmap.height()),
                QPoint(wp.wall_text_interval_x.x, pixmap.height())
            }),
            QPolygon({points[0], points[1], points[2], points[3]}),
            brush_transform
        ) == false) {
            continue ;
        }
        QBrush brush(pixmap);
        brush.setTransform(brush_transform);
        painter.setBrush(brush);
        painter.drawPolygon(points, 4, Qt::OddEvenFill);
    }
}

bool MyWindow::isPWalkable(u32 x, u32 y) {
    if (cubmap_model->isIndexValid(x, y) &&
        cubmap_model->_map[y][x] == CubMap_Model::CubMap_ModelCell::Empty
    ) {
        return true;
    }

    return false;
}

void MyWindow::updatePosition(r32 dt) {
    if (is_w_down) {
        v2<r32> old_p = cubmap_model->_camera.p;
        v2<r32> dp = v2_normalize(cubmap_model->_camera.dir) * dt;
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
    if (is_s_down) {
        v2<r32> old_p = cubmap_model->_camera.p;
        v2<r32> dp = -(v2_normalize(cubmap_model->_camera.dir) * dt);
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
    if (is_a_down) {
        v2<r32> old_p = cubmap_model->_camera.p;
        v2<r32> dp = -(v2_normalize(cubmap_model->_camera.plane) * dt);
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
    if (is_d_down) {
        v2<r32> old_p = cubmap_model->_camera.p;
        v2<r32> dp = v2_normalize(cubmap_model->_camera.plane) * dt;
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            cubmap_model->_camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
}

void MyWindow::updateOrientation() {
    if (is_cursor_free) {
        return ;
    }

    i32 mouse_x = cursor.pos().x();
    r32 mouse_dx = mouse_x - (r32) width() / 2.0f;
    cursor.setPos(width() / 2, height() / 2);

    r32 max_mouse_dx = width();
    mouse_dx = clamp_value(-max_mouse_dx, mouse_dx, max_mouse_dx);
    mouse_dx = (mouse_dx < 0.0f ? -1.5 : 1.5 ) * sqrt(abs(mouse_dx));
    mouse_dx /= 100.0f;
    cubmap_model->_camera.updateDirection(mouse_dx);
}

void MyWindow::updateAndRender(r32 dt) {
    LOG(dt);
    updateOrientation();
    updatePosition(dt);
    this->update(); // calls paintEvent
}

bool MyWindow::isAlive(void) const {
    return this->is_alive;
}
