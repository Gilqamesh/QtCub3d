#ifndef WALL_PIECE_H
#define WALL_PIECE_H

#include "v2.h"

enum class WallTexId {
    North,
    South,
    West,
    East,

    SIZE
};

struct wall_piece {
    v2<i32> top_left;
    v2<i32> bot_right;
    v2<i32> wall_text_interval_x;
    WallTexId wall_tex_id;
};

struct floor_piece {
    i32 a;
};

struct ceiling_piece {
    i32 b;
};

#endif // WALL_PIECE_H
