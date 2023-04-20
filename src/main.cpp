#include "app/app_widget.h"
#include "mvcs/map/map_model.h"

#include <QApplication>

#include "defs.h"

int main(int argc, char *argv[]) {
    try {
        QApplication app(argc, argv);

        Map_Model map_model("../Cub3d/maps/map.cub");

        v2<u32> app_widget_dims(1280, 720);
        App_Widget app_widget(app_widget_dims, &map_model);

        app_widget.exec();

    } catch (const std::exception& err) {
        LOG(err.what());
    }
}
