#ifndef NEWMAPDIALOG_H
#define NEWMAPDIALOG_H

#include <QDialog>

#include "../mvcs/map/map_model.h"

#include "../defs.h"

namespace Ui {
class NewMapDialog;
}

class NewMapDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewMapDialog(v2<u32> widthInterval, v2<u32> heightInterval, QWidget *parent = nullptr);
    ~NewMapDialog();

private:
    Ui::NewMapDialog *ui;

private slots:
    void on_dialogButtonBox_accepted();
    void on_dialogButtonBox_rejected();

signals:
    void valuesChanged(i32 width, i32 height);
};

#endif // NEWMAPDIALOG_H
