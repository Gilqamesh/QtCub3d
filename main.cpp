
#include "mywindow.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QThread>
#include "cubmap_model.h"

#include "defs.h"

int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);
        CubMap_Model cubmap_model("../Cub3d/maps/map.cub");
        MyWindow w(800, 600, &cubmap_model);
        w.show();
        
        constexpr r64 frame_budge_fps = 60.0;
        constexpr u64 frame_budget_ns = 1000000000.0 / frame_budge_fps;

        QElapsedTimer timer;
        timer.start();
        u64 frame_time_end_ns = timer.nsecsElapsed();

        while (w.isAlive()) {
            u64 frame_time_start_ns = timer.nsecsElapsed();
            r32 dt_s = (frame_time_start_ns - frame_time_end_ns) / 1000000000.0;
            frame_time_end_ns = frame_time_start_ns;

            a.processEvents();
            w.updateAndRender(dt_s);

            u64 cur_frame_time_end_ns = timer.nsecsElapsed();
            while (cur_frame_time_end_ns - frame_time_start_ns < frame_budget_ns) {
                QThread::yieldCurrentThread();
                cur_frame_time_end_ns = timer.nsecsElapsed();
            }
        }
    } catch (const exception& err) {
        LOG(err.what());
    }
}
