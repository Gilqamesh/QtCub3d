#ifndef MAP_EDITOR_MVC_H
#define MAP_EDITOR_MVC_H

#include "../map/map_model.h"
#include "map_editor_delegate.h"
#include "map_editor_view.h"

class Map_Editor_Mvc: public Map_Editor_View {
public:
    Map_Editor_Mvc(Map_Model* map_model);
    virtual ~Map_Editor_Mvc() = default;

private:
    Map_Editor_Delegate map_editor_delegate;
    Map_Editor_View map_editor_view;
};

#endif
