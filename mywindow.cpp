#include "mywindow.h"
#include "./ui_mywindow.h"

#include <QPainter>
#include <QKeyEvent>
#include <QDockWidget>
#include "clamp.h"

#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

MyWindow::MyWindow(const std::string& cubmap_path)
    : ui(new Ui::MyWindow)
    , cubmap_model(new CubMap_Model(cubmap_path))
    , cubmap_view(new CubMap_View())
    , cubmap_delegate(new CubMap_Delegate())
    , cursor(Qt::CrossCursor)
    , is_alive(true)
{
    ui->setupUi(this);

    this->setFixedSize(1024, 768);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // this->setBaseSize(width(), height());

    // todo: input
    is_cursor_free = false;
    is_w_down = false;
    is_s_down = false;
    is_a_down = false;
    is_d_down = false;

    // texture loading
    wall_textures[static_cast<u32>(WallTexId::North)] = QImage("../Cub3d/assets/North.jpg");
    wall_textures[static_cast<u32>(WallTexId::South)] = QImage("../Cub3d/assets/South.jpg");
    wall_textures[static_cast<u32>(WallTexId::West)] = QImage("../Cub3d/assets/West.jpg");
    wall_textures[static_cast<u32>(WallTexId::East)] = QImage("../Cub3d/assets/East.jpg");
    for (u32 wall_tex_id = 0; wall_tex_id < static_cast<u32>(WallTexId::SIZE); ++wall_tex_id) {
        // note: rotate images for sequential access during wall casting
        QTransform transform;
        transform.rotate(90);
        wall_textures[wall_tex_id] = wall_textures[wall_tex_id].transformed(transform);
    }
    floor_tex = QImage("../Cub3d/assets/Floor.jpg");
    ceiling_tex = QImage("../Cub3d/assets/Ceiling.jpg");


    // framebuffer
    framebuffer = QImage(width(), height(), QImage::Format_ARGB32);
    framebuffer_rl = QImage(height(), width(), QImage::Format_ARGB32);

    setCursor(cursor);
    QRect global_p = geometry();
    cursor.setPos(global_p.x() + width() / 2, global_p.y() + height() / 2);

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
        } break ;
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
    cubmap_view->update();
    render();
}

static int bitCount(i32 x) {
    if (x == 0) {
        return 0;
    }
    return 1 + bitCount(x & (x - 1));
}

static bool isPowOf2(i32 x) {
    return bitCount(x) == 1;
}

void MyWindow::updateFloor() {
    QRgb* floor_text = reinterpret_cast<QRgb*>(floor_tex.bits());
    QRgb* ceiling_text = reinterpret_cast<QRgb*>(ceiling_tex.bits());
    QRgb* framebuffer_bits = reinterpret_cast<QRgb*>(framebuffer.bits());
    v2<i32> floor_tex_dims(floor_tex.width(), floor_tex.height());
    v2<i32> ceiling_tex_dims(ceiling_tex.width(), ceiling_tex.height());
    i32 framebuffer_width = framebuffer.width();
    // u64 _inner_t_s = 0;
    v2<i32> dims(width(), height());

    // note: assumptions taken
    assert(dims.x % 4 == 0 && "Screen width must be divisible by 4 for loop unrolling"); // could fallback to a worse routine I guess
    assert(
        isPowOf2(floor_tex_dims.x) &&
        isPowOf2(floor_tex_dims.y) &&
        isPowOf2(ceiling_tex_dims.x) &&
        isPowOf2(ceiling_tex_dims.y)
    );

    __m128 sixteen_m128 = _mm_set1_ps(16.0f);
    __m128 floor_tex_x_dim_m128 = _mm_set1_ps((r32) floor_tex_dims.x);
    __m128 floor_tex_y_dim_m128 = _mm_set1_ps((r32) floor_tex_dims.y);
    __m128 ceiling_tex_x_dim_m128 = _mm_set1_ps((r32) ceiling_tex_dims.x);
    __m128 ceiling_tex_y_dim_m128 = _mm_set1_ps((r32) ceiling_tex_dims.y);
    __m128i floor_tex_x_dim_m128i = _mm_set1_epi32(floor_tex_dims.x);
    __m128i ceiling_tex_x_dim_m128i = _mm_set1_epi32(ceiling_tex_dims.x);
    __m128i floor_tex_x_dim_m128i_minus_1 = _mm_set1_epi32(floor_tex_dims.x - 1);
    __m128i floor_tex_y_dim_m128i_minus_1 = _mm_set1_epi32(floor_tex_dims.y - 1);
    __m128i ceiling_tex_x_dim_m128i_minus_1 = _mm_set1_epi32(ceiling_tex_dims.x - 1);
    __m128i ceiling_tex_y_dim_m128i_minus_1 = _mm_set1_epi32(ceiling_tex_dims.y - 1);

    // u64 _uft = __rdtsc();
    // u64 _outer_to = 0;
    for (i32 row = dims.y - 1; row > dims.y / 2; --row) {
        r32 row_distance = ((r32) dims.y / 2.0f) / ((r32) row - (r32) dims.y / 2.0f);
        r32 tex_step_x = row_distance * 2.0f * cubmap_model->_camera.plane.x / (r32) dims.x;
        r32 tex_step_y = row_distance * 2.0f * cubmap_model->_camera.plane.y / (r32) dims.x;
        __m128 tex_step_x_m128 = _mm_set1_ps(tex_step_x);
        __m128 tex_step_y_m128 = _mm_set1_ps(tex_step_y);

        QRgb* framebuffer_floor_scanline = framebuffer_bits + row * framebuffer_width;
        QRgb* framebuffer_ceiling_scanline = framebuffer_bits + (dims.y - row - 1) * framebuffer_width;

        r32 tex_x = cubmap_model->_camera.p.x + row_distance * (cubmap_model->_camera.dir.x - cubmap_model->_camera.plane.x);
        r32 tex_y = cubmap_model->_camera.p.y + row_distance * (cubmap_model->_camera.dir.y - cubmap_model->_camera.plane.y);
        __m128 tex_x_m128 = _mm_set_ps(
            tex_x - (i32) tex_x,
            (tex_x + tex_step_x) - (i32) (tex_x + tex_step_x),
            (tex_x + 2.0f * tex_step_x) - (i32) (tex_x + 2.0f * tex_step_x),
            (tex_x + 3.0f * tex_step_x) - (i32) (tex_x + 3.0f * tex_step_x)
        );
        __m128 tex_y_m128 = _mm_set_ps(
            tex_y - (i32) tex_y,
            (tex_y + tex_step_y) - (i32) (tex_y + tex_step_y),
            (tex_y + 2.0f * tex_step_y) - (i32) (tex_y + 2.0f * tex_step_y),
            (tex_y + 3.0f * tex_step_y) - (i32) (tex_y + 3.0f * tex_step_y)
        );
        tex_x += 4.0f * tex_step_x;
        tex_y += 4.0f * tex_step_y;

        __m128 tex_x_m128_2 = _mm_set_ps(
            tex_x - (i32) tex_x,
            (tex_x + tex_step_x) - (i32) (tex_x + tex_step_x),
            (tex_x + 2.0f * tex_step_x) - (i32) (tex_x + 2.0f * tex_step_x),
            (tex_x + 3.0f * tex_step_x) - (i32) (tex_x + 3.0f * tex_step_x)
        );
        __m128 tex_y_m128_2 = _mm_set_ps(
            tex_y - (i32) tex_y,
            (tex_y + tex_step_y) - (i32) (tex_y + tex_step_y),
            (tex_y + 2.0f * tex_step_y) - (i32) (tex_y + 2.0f * tex_step_y),
            (tex_y + 3.0f * tex_step_y) - (i32) (tex_y + 3.0f * tex_step_y)
        );
        tex_x += 4.0f * tex_step_x;
        tex_y += 4.0f * tex_step_y;

        __m128 tex_x_m128_3 = _mm_set_ps(
            tex_x - (i32) tex_x,
            (tex_x + tex_step_x) - (i32) (tex_x + tex_step_x),
            (tex_x + 2.0f * tex_step_x) - (i32) (tex_x + 2.0f * tex_step_x),
            (tex_x + 3.0f * tex_step_x) - (i32) (tex_x + 3.0f * tex_step_x)
        );
        __m128 tex_y_m128_3 = _mm_set_ps(
            tex_y - (i32) tex_y,
            (tex_y + tex_step_y) - (i32) (tex_y + tex_step_y),
            (tex_y + 2.0f * tex_step_y) - (i32) (tex_y + 2.0f * tex_step_y),
            (tex_y + 3.0f * tex_step_y) - (i32) (tex_y + 3.0f * tex_step_y)
        );
        tex_x += 4.0f * tex_step_x;
        tex_y += 4.0f * tex_step_y;

        __m128 tex_x_m128_4 = _mm_set_ps(
            tex_x - (i32) tex_x,
            (tex_x + tex_step_x) - (i32) (tex_x + tex_step_x),
            (tex_x + 2.0f * tex_step_x) - (i32) (tex_x + 2.0f * tex_step_x),
            (tex_x + 3.0f * tex_step_x) - (i32) (tex_x + 3.0f * tex_step_x)
        );
        __m128 tex_y_m128_4 = _mm_set_ps(
            tex_y - (i32) tex_y,
            (tex_y + tex_step_y) - (i32) (tex_y + tex_step_y),
            (tex_y + 2.0f * tex_step_y) - (i32) (tex_y + 2.0f * tex_step_y),
            (tex_y + 3.0f * tex_step_y) - (i32) (tex_y + 3.0f * tex_step_y)
        );
        tex_x += 4.0f * tex_step_x;
        tex_y += 4.0f * tex_step_y;

        for (i32 col = 0; col < dims.x; col += 16) {
            // u64 _inner_t_s_cur = __rdtsc();

            __m128i floor_xs = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(floor_tex_y_dim_m128, tex_y_m128)),
                        floor_tex_y_dim_m128i_minus_1
                    ),
                    floor_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(floor_tex_x_dim_m128, tex_x_m128)),
                    floor_tex_x_dim_m128i_minus_1
                )
            );
            __m128i ceiling_xs = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_y_dim_m128, tex_y_m128)),
                        ceiling_tex_y_dim_m128i_minus_1
                    ),
                    ceiling_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_x_dim_m128, tex_x_m128)),
                    ceiling_tex_x_dim_m128i_minus_1
                )
            );
            tex_x_m128 = _mm_add_ps(tex_x_m128, _mm_mul_ps(sixteen_m128, tex_step_x_m128));
            tex_y_m128 = _mm_add_ps(tex_y_m128, _mm_mul_ps(sixteen_m128, tex_step_y_m128));

            __m128i floor_xs_2 = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(floor_tex_y_dim_m128, tex_y_m128_2)),
                        floor_tex_y_dim_m128i_minus_1
                    ),
                    floor_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(floor_tex_x_dim_m128, tex_x_m128_2)),
                    floor_tex_x_dim_m128i_minus_1
                )
            );
            __m128i ceiling_xs_2 = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_y_dim_m128, tex_y_m128_2)),
                        ceiling_tex_y_dim_m128i_minus_1
                    ),
                    ceiling_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_x_dim_m128, tex_x_m128_2)),
                    ceiling_tex_x_dim_m128i_minus_1
                )
            );
            tex_x_m128_2 = _mm_add_ps(tex_x_m128_2, _mm_mul_ps(sixteen_m128, tex_step_x_m128));
            tex_y_m128_2 = _mm_add_ps(tex_y_m128_2, _mm_mul_ps(sixteen_m128, tex_step_y_m128));

            __m128i floor_xs_3 = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(floor_tex_y_dim_m128, tex_y_m128_3)),
                        floor_tex_y_dim_m128i_minus_1
                    ),
                    floor_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(floor_tex_x_dim_m128, tex_x_m128_3)),
                    floor_tex_x_dim_m128i_minus_1
                )
            );
            __m128i ceiling_xs_3 = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_y_dim_m128, tex_y_m128_3)),
                        ceiling_tex_y_dim_m128i_minus_1
                    ),
                    ceiling_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_x_dim_m128, tex_x_m128_3)),
                    ceiling_tex_x_dim_m128i_minus_1
                )
            );
            tex_x_m128_3 = _mm_add_ps(tex_x_m128_3, _mm_mul_ps(sixteen_m128, tex_step_x_m128));
            tex_y_m128_3 = _mm_add_ps(tex_y_m128_3, _mm_mul_ps(sixteen_m128, tex_step_y_m128));

            __m128i floor_xs_4 = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(floor_tex_y_dim_m128, tex_y_m128_4)),
                        floor_tex_y_dim_m128i_minus_1
                    ),
                    floor_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(floor_tex_x_dim_m128, tex_x_m128_4)),
                    floor_tex_x_dim_m128i_minus_1
                )
            );
            __m128i ceiling_xs_4 = _mm_add_epi32(
                _mm_mullo_epi32(
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_y_dim_m128, tex_y_m128_4)),
                        ceiling_tex_y_dim_m128i_minus_1
                    ),
                    ceiling_tex_x_dim_m128i
                ),
                _mm_and_si128(
                    _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_x_dim_m128, tex_x_m128_4)),
                    ceiling_tex_x_dim_m128i_minus_1
                )
            );
            tex_x_m128_4 = _mm_add_ps(tex_x_m128_4, _mm_mul_ps(sixteen_m128, tex_step_x_m128));
            tex_y_m128_4 = _mm_add_ps(tex_y_m128_4, _mm_mul_ps(sixteen_m128, tex_step_y_m128));

            // _inner_t_s += __rdtsc() - _inner_t_s_cur;

            // u64 _outer_to_s = __rdtsc();

            _mm_storeu_epi32(framebuffer_floor_scanline + col, _mm_i32gather_epi32((const int*) floor_text, _mm_shuffle_epi32(floor_xs, _MM_SHUFFLE(0, 1, 2, 3)), 4));
            _mm_storeu_epi32(framebuffer_floor_scanline + col + 4, _mm_i32gather_epi32((const int*) floor_text, _mm_shuffle_epi32(floor_xs_2, _MM_SHUFFLE(0, 1, 2, 3)), 4));
            _mm_storeu_epi32(framebuffer_floor_scanline + col + 8, _mm_i32gather_epi32((const int*) floor_text, _mm_shuffle_epi32(floor_xs_3, _MM_SHUFFLE(0, 1, 2, 3)), 4));
            _mm_storeu_epi32(framebuffer_floor_scanline + col + 12, _mm_i32gather_epi32((const int*) floor_text, _mm_shuffle_epi32(floor_xs_4, _MM_SHUFFLE(0, 1, 2, 3)), 4));

            _mm_storeu_epi32(framebuffer_ceiling_scanline + col, _mm_i32gather_epi32((const int*) ceiling_text, _mm_shuffle_epi32(ceiling_xs, _MM_SHUFFLE(0, 1, 2, 3)), 4));
            _mm_storeu_epi32(framebuffer_ceiling_scanline + col + 4, _mm_i32gather_epi32((const int*) ceiling_text, _mm_shuffle_epi32(ceiling_xs_2, _MM_SHUFFLE(0, 1, 2, 3)), 4));
            _mm_storeu_epi32(framebuffer_ceiling_scanline + col + 8, _mm_i32gather_epi32((const int*) ceiling_text, _mm_shuffle_epi32(ceiling_xs_3, _MM_SHUFFLE(0, 1, 2, 3)), 4));
            _mm_storeu_epi32(framebuffer_ceiling_scanline + col + 12, _mm_i32gather_epi32((const int*) ceiling_text, _mm_shuffle_epi32(ceiling_xs_4, _MM_SHUFFLE(0, 1, 2, 3)), 4));

            // _outer_to += __rdtsc() - _outer_to_s;
        }
    }
    // u64 _e = __rdtsc();
    // LOG("Cy(M) _outer_to: " << _outer_to / 1000000.0);
    // LOG("Cy(M) inner: " << _inner_t_s / 1000000.0);
    // LOG("Cy(M) update floor: " << (_e - _uft) / 1000000.0);
}

void MyWindow::updateWall() {
    enum class Side {
        Vertical,
        Horizontal
    };

    QRgb* framebuffer_bits = reinterpret_cast<QRgb*>(framebuffer_rl.bits());
    v2<i32> framebuffer_bits_dims(framebuffer_rl.width(), framebuffer_rl.height());
    v2<i32> dims(width(), height());

    // u64 _inner_total = 0;
    // u64 _outer_total = __rdtsc();

    // note: clear alpha channel and do an alpha premultply essentially
    memset(framebuffer_bits, 0, framebuffer_bits_dims.x * framebuffer_bits_dims.y * sizeof(*framebuffer_bits));

    for (i32 col = 0; col < dims.x; ++col) {
        r32 camera_x = 2.0 * (r32) col / (r32) dims.x - 1.0f;
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
        r32 line_height = (i32) ((r32) dims.y / perp_wall_dist);
        v2<i32> vertical_draw_interval_clamped(
            clamp_value(0, (i32) (((r32) dims.y - line_height) / 2.0f), dims.y - 1),
            clamp_value(0, (i32) (((r32) dims.y + line_height) / 2.0f), dims.y - 1)
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
        QImage& wall_text = wall_textures[static_cast<u32>(wall_tex_id)];
        v2<i32> text_dims(wall_text.width(), wall_text.height());
        assert(isPowOf2(text_dims.x) && isPowOf2(text_dims.y));
        QRgb* wall_bits = reinterpret_cast<QRgb*>(wall_text.bits());

        r32 wall_tex_x;
        if (side == Side::Horizontal) {
            wall_tex_x = cubmap_model->_camera.p.y + perp_wall_dist * raydir.y;
        } else {
            wall_tex_x = cubmap_model->_camera.p.x + perp_wall_dist * raydir.x;
        }
        wall_tex_x -= floor(wall_tex_x);

        r32 tex_step = (r32) text_dims.y / line_height;
	    r32 tex_p = (vertical_draw_interval_clamped.x + (line_height - (r32) dims.y) / 2.0f) * tex_step;

        i32 tex_start_offset_x = wall_tex_x * (r32) text_dims.x;
        if ((side == Side::Horizontal && raydir.x > 0.0f) ||
            (side == Side::Vertical && raydir.y < 0.0f)
        ) {
            tex_start_offset_x = text_dims.x - tex_start_offset_x - 1;
        }

        // u64 _inner_sub = __rdtsc();
        i32 vertical_stripe_length = vertical_draw_interval_clamped.y - vertical_draw_interval_clamped.x;
        i32 row = 0;
        for (; row < vertical_stripe_length - 8; row += 8) {
            i32 cur_row = row + vertical_draw_interval_clamped.x;

            i32 tex_start_offset_y = ((i32) tex_p) & (text_dims.y - 1);
            i32 tex_start_offset_y_2 = ((i32) (tex_p + tex_step)) & (text_dims.y - 1);
            i32 tex_start_offset_y_3 = ((i32) (tex_p + 2.0f * tex_step)) & (text_dims.y - 1);
            i32 tex_start_offset_y_4 = ((i32) (tex_p + 3.0f * tex_step)) & (text_dims.y - 1);
            i32 tex_start_offset_y_5 = ((i32) (tex_p + 4.0f * tex_step)) & (text_dims.y - 1);
            i32 tex_start_offset_y_6 = ((i32) (tex_p + 5.0f * tex_step)) & (text_dims.y - 1);
            i32 tex_start_offset_y_7 = ((i32) (tex_p + 6.0f * tex_step)) & (text_dims.y - 1);
            i32 tex_start_offset_y_8 = ((i32) (tex_p + 7.0f * tex_step)) & (text_dims.y - 1);
            tex_p += 8.0f * tex_step;

            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 1] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_2];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 2] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_3];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 3] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_4];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 4] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_5];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 5] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_6];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 6] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_7];
            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row + 7] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_8];
        }
        for (; row < vertical_stripe_length; ++row) {
            i32 cur_row = row + vertical_draw_interval_clamped.x;

            i32 tex_start_offset_y = ((i32) tex_p) & (text_dims.y - 1);
            tex_p += tex_step;

            framebuffer_bits[col * framebuffer_bits_dims.x + cur_row] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y];
        }
        // _inner_total += __rdtsc() - _inner_sub;
    }
    // _outer_total = __rdtsc() - _outer_total;
    // LOG("Cy(M) wall inner: " << _inner_total / 1000000.0);
    // LOG("Cy(M) wall outer: " << _outer_total / 1000000.0);
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
    QRect global_p = geometry();
    r32 mouse_dx = mouse_x - (global_p.x() + (r32) width() / 2.0f);
    cursor.setPos(global_p.x() + width() / 2, global_p.y() + height() / 2);

    r32 max_mouse_dx = width();
    mouse_dx = clamp_value(-max_mouse_dx, mouse_dx, max_mouse_dx);
    mouse_dx = (mouse_dx < 0.0f ? -1.5 : 1.5 ) * sqrt(abs(mouse_dx));
    mouse_dx /= 100.0f;
    cubmap_model->_camera.updateDirection(mouse_dx);
}

void MyWindow::render() {
    u64 _rs = __rdtsc();
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), framebuffer);

    painter.translate(width() / 2, height() / 2);
    painter.rotate(-90.0);
    painter.translate(-framebuffer_rl.width() / 2, -framebuffer_rl.height() / 2);
    painter.drawImage(QPoint(0, 0), framebuffer_rl);
    painter.resetTransform();

    LOG("Cy(M) render: " << (__rdtsc() - _rs) / 1000000.0);
}

void MyWindow::updateAndRender(r32 dt) {
    LOG("dt(s): " << dt);
    updateOrientation();
    updatePosition(dt);
    updateFloor();
    updateWall();
    update(); // calls paintEvent
}

bool MyWindow::isAlive(void) const {
    return this->is_alive;
}
