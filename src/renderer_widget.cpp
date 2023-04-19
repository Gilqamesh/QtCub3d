#include "renderer_widget.h"
#include "./ui_renderer_widget.h"

#include <QPainter>
#include <QKeyEvent>

#include "clamp.h"

#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

Renderer_Widget::Renderer_Widget(Map_Model* map_model)
    : ui(new Ui::Renderer_Widget),
    _map_model(map_model) {
    ui->setupUi(this);

    is_alive = true;

    // configure size
    // this->setFixedSize(1024, 768);
    // this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // this->setGeometry(0, 0, 1024, 768);
    // this->setBaseSize(1024, 768);

    // texture loading
    wall_textures[static_cast<u32>(WallTexId::North)] = QImage("../assets/North.jpg");
    wall_textures[static_cast<u32>(WallTexId::South)] = QImage("../assets/South.jpg");
    wall_textures[static_cast<u32>(WallTexId::West)] = QImage("../assets/West.jpg");
    wall_textures[static_cast<u32>(WallTexId::East)] = QImage("../assets/East.jpg");
    for (u32 wall_tex_id = 0; wall_tex_id < static_cast<u32>(WallTexId::SIZE); ++wall_tex_id) {
        // note: rotate images for sequential access during wall casting
        QTransform transform;
        transform.rotate(-90);
        wall_textures[wall_tex_id] = wall_textures[wall_tex_id].transformed(transform);
    }
    floor_tex = QImage("../assets/Floor.jpg");
    ceiling_tex = QImage("../assets/Ceiling.jpg");

    // todo: input
    is_w_down = false;
    is_s_down = false;
    is_a_down = false;
    is_d_down = false;

    v2<u32> framebuffer_dims(512, 512);
    // floor_ceiling_framebuffer = QImage(framebuffer_dims.x, framebuffer_dims.y, QImage::Format::Format_ARGB32);
    // wall_framebuffer = QImage(framebuffer_dims.x, framebuffer_dims.y, QImage::Format::Format_ARGB32);
    u32 floor_ceiling_framebuffer_data_size = framebuffer_dims.x * framebuffer_dims.y * sizeof(QRgb);
    uchar* floor_ceiling_framebuffer_data = (uchar*) _aligned_malloc(floor_ceiling_framebuffer_data_size, 64);
    floor_ceiling_framebuffer = QImage(
        floor_ceiling_framebuffer_data,
        framebuffer_dims.x,
        framebuffer_dims.y,
        QImage::Format::Format_ARGB32,
        [](void* data) {
            _aligned_free(data);
        }, floor_ceiling_framebuffer_data
    );
    u32 wall_framebuffer_framebuffer_data_size = framebuffer_dims.y * framebuffer_dims.x * sizeof(QRgb);
    uchar* wall_framebuffer_framebuffer_data = (uchar*) _aligned_malloc(wall_framebuffer_framebuffer_data_size, 64);
    wall_framebuffer = QImage(
        wall_framebuffer_framebuffer_data,
        framebuffer_dims.y,
        framebuffer_dims.x,
        QImage::Format::Format_ARGB32,
        [](void* data) {
            _aligned_free(data);
        }, wall_framebuffer_framebuffer_data
    );

    // mode
    setMode(Mode::NotPlaying);

    // editor
    // QGridLayout* editor_layout = new QGridLayout();
    // editor_layout->setParent(this->centralWidget()->layout());
    // editor_layout->setGeometry(QRect(0, height() / 2, width() / 2, height() / 2));
    // editor_layout->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    // editor_layout->addWidget(map_editor_view, 0, 0);

    // QMenu* cubmap_editor = menuBar()->addMenu("CubMap editor");
    // QAction* cubmap_editor_open_act = new QAction("&Edit", map_editor_view);
    // connect(
    //     cubmap_editor_open_act, &QAction::triggered,
    //     map_editor_view, [=](){
    //         map_editor_view->show();
    //     }
    // );
    // cubmap_editor->addAction(cubmap_editor_open_act);
}

Renderer_Widget::~Renderer_Widget() {
    delete ui;
}

void Renderer_Widget::setMode(Mode mode) {
    this->mode = mode;
    switch (mode) {
        case Mode::Playing: {
            this->grabKeyboard();
            this->grabMouse(QCursor(Qt::BlankCursor));
            this->cursor().setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
        } break ;
        case Mode::NotPlaying: {
            this->releaseKeyboard();
            this->releaseMouse();
        } break ;
        default: throw std::runtime_error("not implemented");
    }
}

void Renderer_Widget::mousePressEvent(QMouseEvent *event) {
    setMode(Mode::Playing);
}

void Renderer_Widget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape: {
            if (mode == Mode::Playing) {
                // setCursor(QCursor(Qt::ArrowCursor));
                setMode(Mode::NotPlaying);
            } else {
                setMode(Mode::Playing);
            }
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

void Renderer_Widget::keyReleaseEvent(QKeyEvent *event) {
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

void Renderer_Widget::mouseMoveEvent(QMouseEvent* event) {
}

void Renderer_Widget::closeEvent(QCloseEvent* event) {
    this->is_alive = false;
}

// note: assumes premultiplied alpha for src
static void copyRotatedImage(QImage* dest, QImage* src) {
    v2<u32> src_dims(src->width(), src->height());
    v2<u32> dest_dims(dest->width(), dest->height());
    assert(src_dims.x == dest_dims.y && src_dims.y == dest_dims.x && "for now no interpolation for scaled images");
    QRgb* src_raw = reinterpret_cast<QRgb*>(src->bits());
    QRgb* dest_raw = reinterpret_cast<QRgb*>(dest->bits());
    u32 dest_col = 0;
    u32 dest_row = 0;
    u32 src_col = 0;
    u32 src_row = 0;

    constexpr u32 cache_line_size = 64;
    constexpr u32 block_size = cache_line_size / sizeof(QRgb);
    assert(
        dest_dims.x % block_size == 0 && dest_dims.y % block_size == 0 &&
        src_dims.x % block_size == 0 && src_dims.y % block_size == 0
    );
    // note: copy the rotated src Image into dest buffer by blocks
    while (src_row < src_dims.y) {
        src_col = 0;
        dest_row = 0;
        u64 cur_cy = __rdtsc();
        while (src_col < src_dims.x) {
            for (u32 j = 0; j < block_size; ++j) {
                u32 cur_src_col = src_col + j;
                u32 cur_dest_row = dest_row + j;
                
                for (u32 i = 0; i < block_size; ++i) {
                    u32 cur_src_row = src_row + i;
                    u32 cur_dest_col = dest_col + i;

                    QRgb* dest = dest_raw + cur_dest_row * dest_dims.x + cur_dest_col;
                    QRgb* src = src_raw + cur_src_row * src_dims.x + cur_src_col;
                    r32 alpha = (1.0f - (r32) qAlpha(*src) / 255.0f);
                    i32 destRed = qRed(*dest);
                    i32 destGreen = qGreen(*dest);
                    i32 destBlue = qBlue(*dest);
                    i32 srcRed = qRed(*src);
                    i32 srcGreen = qGreen(*src);
                    i32 srcBlue = qBlue(*src);
                    // note: no alpha premultiplication from here on as the whole routine consists of two framebuffers
                    *dest =  qRgba(
                        destRed * alpha + srcRed,
                        destGreen * alpha + srcGreen,
                        destBlue * alpha + srcBlue,
                        0xff
                    );
                }
            }
            src_col += block_size;
            dest_row += block_size;
        }
        dest_col += block_size;
        src_row += block_size;
    }
}

void Renderer_Widget::paintEvent(QPaintEvent* event) {
    copyRotatedImage(&floor_ceiling_framebuffer, &wall_framebuffer);

    QRect target_rec(0, 0, this->width(), this->height());
    QPainter painter(this);

    painter.drawImage(target_rec, floor_ceiling_framebuffer, QRect(0, 0, floor_ceiling_framebuffer.width(), floor_ceiling_framebuffer.height()));

    // note: draw crosshair
    v2<i32> crosshair_offset(target_rec.x() / 80, target_rec.y() / 460);
    v2<i32> crosshair_dims = crosshair_offset * 2;
    painter.fillRect(
        target_rec.x() / 2 - crosshair_offset.x, target_rec.y() / 2 - crosshair_offset.y,
        crosshair_dims.x, crosshair_dims.y,
        Qt::black
    );
    painter.fillRect(
        target_rec.x() / 2 - crosshair_offset.y, target_rec.y() / 2 - crosshair_offset.x,
        crosshair_dims.y, crosshair_dims.x,
        Qt::black
    );
}

bool Renderer_Widget::isPWalkable(u32 x, u32 y) {
    if (_map_model->isIndexValid(x, y) &&
        _map_model->getData(x, y) == Map_Model::Cell::Empty ||
        _map_model->getData(x, y) == Map_Model::Cell::Player
    ) {
        return true;
    }

    return false;
}

void Renderer_Widget::updatePosition(r32 dt) {
    v2<i32> old_map_p(
        _map_model->camera.p.x,
        _map_model->camera.p.y
    );
    if (is_w_down) {
        v2<r32> old_p = _map_model->camera.p;
        v2<r32> dp = v2_normalize(_map_model->camera.dir) * dt;
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            _map_model->camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            _map_model->camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
    if (is_s_down) {
        v2<r32> old_p = _map_model->camera.p;
        v2<r32> dp = -(v2_normalize(_map_model->camera.dir) * dt);
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            _map_model->camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            _map_model->camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
    if (is_a_down) {
        v2<r32> old_p = _map_model->camera.p;
        v2<r32> dp = -(v2_normalize(_map_model->camera.plane) * dt);
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            _map_model->camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            _map_model->camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }
    if (is_d_down) {
        v2<r32> old_p = _map_model->camera.p;
        v2<r32> dp = v2_normalize(_map_model->camera.plane) * dt;
        if (isPWalkable(old_p.x + dp.x, old_p.y)) {
            _map_model->camera.updatePosition(v2<r32>(dp.x, 0.0f));
        }
        if (isPWalkable(old_p.x, old_p.y + dp.y)) {
            _map_model->camera.updatePosition(v2<r32>(0.0f, dp.y));
        }
    }

    _map_model->setData(old_map_p.x, old_map_p.y, Map_Model::Cell::Empty);
    _map_model->setData(
        (i32) _map_model->camera.p.x,
        (i32) _map_model->camera.p.y,
        Map_Model::Cell::Player
    );
}

void Renderer_Widget::updateOrientation() {
    QPoint mouse_p = mapFromGlobal(cursor().pos());
    i32 half_screen_p_x = width() / 2.0f;
    r32 mouse_dx = mouse_p.x() - half_screen_p_x;
    this->cursor().setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));

    r32 max_mouse_dx = this->width() / 2;
    mouse_dx = clamp_value(-max_mouse_dx, mouse_dx, max_mouse_dx);
    r32 mouse_attenuation = 0.005f;
    if (mouse_dx < 0.0f) {
        mouse_attenuation *= -1.0f;
    }
    mouse_dx = mouse_attenuation * pow(abs(mouse_dx), 0.75);
    r32 mouse_max_dphi = 0.75f;
    mouse_dx = clamp_value(-mouse_max_dphi, mouse_dx, mouse_max_dphi);
    _map_model->camera.updateDirection(mouse_dx);
}

void Renderer_Widget::updateAndRender(r32 dt) {
    // LOG("dt(s): " << dt);

    if (mode == Mode::Playing) {
        updateOrientation();
        updatePosition(dt);
    }
    updateFloorAndCeiling();
    updateWall();
    this->update(); // calls paintEvent
}

static inline int bitCount(i32 x) {
    if (x == 0) {
        return 0;
    }
    return 1 + bitCount(x & (x - 1));
}

static inline bool isPowOf2(i32 x) {
    return bitCount(x) == 1;
}

void Renderer_Widget::updateFloorAndCeiling() {
    QRgb* floor_text_raw = reinterpret_cast<QRgb*>(floor_tex.bits());
    QRgb* ceiling_text_raw = reinterpret_cast<QRgb*>(ceiling_tex.bits());
    QRgb* floor_ceiling_framebuffer_raw = reinterpret_cast<QRgb*>(floor_ceiling_framebuffer.bits());
    v2<i32> floor_tex_dims(floor_tex.width(), floor_tex.height());
    v2<i32> ceiling_tex_dims(ceiling_tex.width(), ceiling_tex.height());
    v2<i32> floor_ceiling_framebuffer_dims(floor_ceiling_framebuffer.width(), floor_ceiling_framebuffer.height()); 

    // note: assumptions taken
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

    for (i32 row = floor_ceiling_framebuffer_dims.y - 1; row > floor_ceiling_framebuffer_dims.y / 2; --row) {
        r32 row_distance = ((r32) floor_ceiling_framebuffer_dims.y / 2.0f) / ((r32) row - (r32) floor_ceiling_framebuffer_dims.y / 2.0f);
        r32 tex_step_x = row_distance * 2.0f * _map_model->camera.plane.x / (r32) floor_ceiling_framebuffer_dims.x;
        r32 tex_step_y = row_distance * 2.0f * _map_model->camera.plane.y / (r32) floor_ceiling_framebuffer_dims.x;
        __m128 tex_step_x_m128 = _mm_set1_ps(tex_step_x);
        __m128 tex_step_y_m128 = _mm_set1_ps(tex_step_y);

        QRgb* framebuffer_floor_scanline = floor_ceiling_framebuffer_raw + row * floor_ceiling_framebuffer_dims.x;
        QRgb* framebuffer_ceiling_scanline = floor_ceiling_framebuffer_raw + (floor_ceiling_framebuffer_dims.y - row - 1) * floor_ceiling_framebuffer_dims.x;

        r32 tex_x = _map_model->camera.p.x + row_distance * (_map_model->camera.dir.x - _map_model->camera.plane.x);
        r32 tex_y = _map_model->camera.p.y + row_distance * (_map_model->camera.dir.y - _map_model->camera.plane.y);
        __m128 tex_x_m128_4[4];
        __m128 tex_y_m128_4[4];
        for (u32 i = 0; i < 4; ++i) {
            tex_x_m128_4[i] = _mm_set_ps(
                tex_x - (i32) tex_x,
                (tex_x + tex_step_x) - (i32) (tex_x + tex_step_x),
                (tex_x + 2.0f * tex_step_x) - (i32) (tex_x + 2.0f * tex_step_x),
                (tex_x + 3.0f * tex_step_x) - (i32) (tex_x + 3.0f * tex_step_x)
            );
            tex_y_m128_4[i] = _mm_set_ps(
                tex_y - (i32) tex_y,
                (tex_y + tex_step_y) - (i32) (tex_y + tex_step_y),
                (tex_y + 2.0f * tex_step_y) - (i32) (tex_y + 2.0f * tex_step_y),
                (tex_y + 3.0f * tex_step_y) - (i32) (tex_y + 3.0f * tex_step_y)
            );
            tex_x += 4.0f * tex_step_x;
            tex_y += 4.0f * tex_step_y;
        }

        i32 col = 0;
        for (; col + 16 <= floor_ceiling_framebuffer_dims.x; col += 16) {
            __m128i floors[4];
            __m128i ceilings[4];
            for (u32 i = 0; i < 4; ++i) {
                floors[i] = _mm_add_epi32(
                    _mm_mullo_epi32(
                        _mm_and_si128(
                            _mm_cvttps_epi32(_mm_mul_ps(floor_tex_y_dim_m128, tex_y_m128_4[i])),
                            floor_tex_y_dim_m128i_minus_1
                        ),
                        floor_tex_x_dim_m128i
                    ),
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(floor_tex_x_dim_m128, tex_x_m128_4[i])),
                        floor_tex_x_dim_m128i_minus_1
                    )
                );
                ceilings[i] = _mm_add_epi32(
                    _mm_mullo_epi32(
                        _mm_and_si128(
                            _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_y_dim_m128, tex_y_m128_4[i])),
                            ceiling_tex_y_dim_m128i_minus_1
                        ),
                        ceiling_tex_x_dim_m128i
                    ),
                    _mm_and_si128(
                        _mm_cvttps_epi32(_mm_mul_ps(ceiling_tex_x_dim_m128, tex_x_m128_4[i])),
                        ceiling_tex_x_dim_m128i_minus_1
                    )
                );
                tex_x_m128_4[i] = _mm_add_ps(tex_x_m128_4[i], _mm_mul_ps(sixteen_m128, tex_step_x_m128));
                tex_y_m128_4[i] = _mm_add_ps(tex_y_m128_4[i], _mm_mul_ps(sixteen_m128, tex_step_y_m128));
            }

            for (u32 i = 0; i < 4; ++i) {
                _mm_storeu_epi32(
                    framebuffer_floor_scanline + col + i * 4, _mm_i32gather_epi32((const int*) floor_text_raw, _mm_shuffle_epi32(floors[i], _MM_SHUFFLE(0, 1, 2, 3)), 4));
                _mm_storeu_epi32(
                    framebuffer_ceiling_scanline + col + i * 4, _mm_i32gather_epi32((const int*) ceiling_text_raw, _mm_shuffle_epi32(ceilings[i], _MM_SHUFFLE(0, 1, 2, 3)), 4));
            }
        }

        tex_x = _mm_cvtss_f32(tex_x_m128_4[3]);
        tex_y = _mm_cvtss_f32(tex_y_m128_4[3]);
        for (; col < floor_ceiling_framebuffer_dims.x; ++col) {
            i32 floor = ((((i32) ((r32) floor_tex_dims.y * tex_y)) & (floor_tex_dims.y - 1)) *
                floor_tex_dims.x) + (((i32) ((r32) floor_tex_dims.x * tex_y)) & (floor_tex_dims.x - 1));
            i32 ceiling = ((((i32) ((r32) ceiling_tex_dims.y * tex_y)) & (ceiling_tex_dims.y - 1)) *
                ceiling_tex_dims.x) + (((i32) ((r32) ceiling_tex_dims.x * tex_y)) & (ceiling_tex_dims.x - 1));
            tex_x += tex_step_x;
            tex_y += tex_step_y;
            framebuffer_floor_scanline[col] = floor_text_raw[floor];
            framebuffer_ceiling_scanline[col] = ceiling_text_raw[ceiling];
        }
    }
}

void Renderer_Widget::updateWall() {
        enum class Side {
        Vertical,
        Horizontal
    };

    QRgb* floor_ceiling_framebuffer_raw = reinterpret_cast<QRgb*>(wall_framebuffer.bits());
    v2<i32> framebuffer_bits_dims(wall_framebuffer.width(), wall_framebuffer.height());
    v2<i32> renderer_dims(framebuffer_bits_dims.y, framebuffer_bits_dims.x);

    // note: clear alpha channel and do an alpha premultply essentially
    memset(floor_ceiling_framebuffer_raw, 0, framebuffer_bits_dims.x * framebuffer_bits_dims.y * sizeof(*floor_ceiling_framebuffer_raw));

    for (i32 col = 0; col < renderer_dims.x; ++col) {
        r32 camera_x = 2.0 * (r32) col / (r32) renderer_dims.x - 1.0f;
        v2<r32> raydir = _map_model->camera.dir + _map_model->camera.plane * camera_x;
        v2<r32> camera_p = _map_model->camera.p;
        v2<i32> map_p((i32) camera_p.x, (i32) camera_p.y);

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
            side_dist.x = (camera_p.x - map_p.x) * delta_dist.x;
        } else {
            ray_step.x = 1;
            side_dist.x = (map_p.x + 1.0f - camera_p.x) * delta_dist.x;
        }
        if (raydir.y < 0.0f) {
            ray_step.y = -1;
            side_dist.y = (camera_p.y - map_p.y) * delta_dist.y;
        } else {
            ray_step.y = 1;
            side_dist.y = (map_p.y + 1.0f - camera_p.y) * delta_dist.y;
        }

        Map_Model::Cell hit = _map_model->getData(map_p.x, map_p.y);
        Side side = Side::Horizontal;
        while (hit != Map_Model::Cell::Wall) {
            if (side_dist.x < side_dist.y) {
                side_dist.x += delta_dist.x;
                map_p.x += ray_step.x;
                side = Side::Horizontal;
            } else {
                side_dist.y += delta_dist.y;
                map_p.y += ray_step.y;
                side = Side::Vertical;
            }
            hit = _map_model->getData(map_p.x, map_p.y);
        }
        r32 perp_wall_dist;
        if (side == Side::Horizontal) {
            perp_wall_dist = side_dist.x - delta_dist.x;
        } else {
            perp_wall_dist = side_dist.y - delta_dist.y;
        }
        r32 line_height = (i32) ((r32) renderer_dims.y / perp_wall_dist);
        v2<i32> vertical_draw_interval_clamped(
            clamp_value(0, (i32) (((r32) renderer_dims.y - line_height) / 2.0f), renderer_dims.y - 1),
            clamp_value(0, (i32) (((r32) renderer_dims.y + line_height) / 2.0f), renderer_dims.y - 1)
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
            wall_tex_x = camera_p.y + perp_wall_dist * raydir.y;
        } else {
            wall_tex_x = camera_p.x + perp_wall_dist * raydir.x;
        }
        wall_tex_x -= floor(wall_tex_x);

        r32 tex_step = (r32) text_dims.y / line_height;
	    r32 tex_p = (vertical_draw_interval_clamped.x + (line_height - (r32) renderer_dims.y) / 2.0f) * tex_step;

        i32 tex_start_offset_x = wall_tex_x * (r32) text_dims.x;
        if ((side == Side::Horizontal && raydir.x > 0.0f) ||
            (side == Side::Vertical && raydir.y < 0.0f)
        ) {
            tex_start_offset_x = text_dims.x - tex_start_offset_x - 1;
        }

        // u64 _inner_sub = __rdtsc();
        i32 vertical_stripe_length = vertical_draw_interval_clamped.y - vertical_draw_interval_clamped.x;
        i32 row = 0;
        for (; row + 8 <= vertical_stripe_length; row += 8) {
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

            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 1] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_2];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 2] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_3];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 3] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_4];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 4] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_5];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 5] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_6];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 6] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_7];
            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row + 7] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y_8];
        }
        for (; row < vertical_stripe_length; ++row) {
            i32 cur_row = row + vertical_draw_interval_clamped.x;

            i32 tex_start_offset_y = ((i32) tex_p) & (text_dims.y - 1);
            tex_p += tex_step;

            floor_ceiling_framebuffer_raw[col * framebuffer_bits_dims.x + cur_row] = wall_bits[tex_start_offset_x * text_dims.y + tex_start_offset_y];
        }
    }
}
