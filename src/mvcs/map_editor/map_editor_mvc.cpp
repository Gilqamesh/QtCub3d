#include "map_editor_mvc.h"

Map_Editor_Mvc::Map_Editor_Mvc(Map_Model* map_model) {
    setModel(map_model);
    setItemDelegate(&map_editor_delegate);
}
