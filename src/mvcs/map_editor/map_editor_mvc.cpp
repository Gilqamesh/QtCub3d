#include "map_editor_mvc.h"

Map_Editor_Mvc::Map_Editor_Mvc(Map_Model* map_model)
    : My_Widget(new Map_Editor_View()) {
    reinterpret_cast<Map_Editor_View*>(getWidget())->setModel(map_model);
    reinterpret_cast<Map_Editor_View*>(getWidget())->setItemDelegate(&map_editor_delegate);
}

void Map_Editor_Mvc::updateAndRender(r32 dt) {
    getWidget()->update();
}
