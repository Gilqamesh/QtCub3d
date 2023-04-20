#include "newmapdialog.h"
#include "ui_newmapdialog.h"

#include <QScrollBar>

NewMapDialog::NewMapDialog(v2<u32> widthInterval, v2<u32> heightInterval, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::NewMapDialog) {
    ui->setupUi(this);

    ui->widthScrollBar->setMinimum(widthInterval.x);
    ui->widthScrollBar->setMaximum(widthInterval.y);
    u32 widthCurValue = (widthInterval.x + widthInterval.y) / 2;
    ui->minWidthLabelValue->setText(std::to_string(widthInterval.x).c_str());
    ui->maxWidthLabelValue->setText(std::to_string(widthInterval.y).c_str());
    ui->widthScrollBar->setValue(widthCurValue);
    ui->widthCurValue->setText(std::to_string(widthCurValue).c_str());
    ui->widthScrollBar->connect(
        ui->widthScrollBar, &QScrollBar::valueChanged,
        ui->widthCurValue, [this](int num) {
            ui->widthCurValue->setText(std::to_string(num).c_str());
        }
    );

    ui->heightScrollBar->setMinimum(heightInterval.x);
    ui->heightScrollBar->setMaximum(heightInterval.y);
    u32 heightCurValue = (heightInterval.x + heightInterval.y) / 2;
    ui->minHeightLabelValue->setText(std::to_string(heightInterval.x).c_str());
    ui->maxHeightLabelValue->setText(std::to_string(heightInterval.y).c_str());
    ui->heightScrollBar->setValue(heightCurValue);
    ui->heightCurValue->setText(std::to_string(heightCurValue).c_str());
    ui->heightScrollBar->connect(
        ui->heightScrollBar, &QScrollBar::valueChanged,
        ui->heightCurValue, [this](int num) {
            ui->heightCurValue->setText(std::to_string(num).c_str());
        }
    );
}

NewMapDialog::~NewMapDialog() {
    delete ui;
}

void NewMapDialog::on_dialogButtonBox_accepted() {
    emit valuesChanged(ui->widthScrollBar->value(), ui->heightScrollBar->value());

    accept();
}


void NewMapDialog::on_dialogButtonBox_rejected() {
    reject();
}
