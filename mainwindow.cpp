#include "mainwindow.h"

#include "ui_mainwindow.h"

/**
 * @brief MainWindow::MainWindow Creates a new Main Window UI.
 * @param parent Qt parent widget.
 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->PresetSelect->setEnabled(true);
  ui->Resolution->setValue(40);
  ui->Resolution->setRange(1, 100);
  ui->showGradient->setChecked(true);
}

void MainWindow::on_PresetSelect_currentIndexChanged(int index){
  if (index == 0)
    ui->MainDisplay->meshChanged(MeshPreset::PAPER);
  if (index == 1)
    ui->MainDisplay->meshChanged(MeshPreset::BASIC);
  if (index == 2)
    ui->MainDisplay->meshChanged(MeshPreset::APPLE);
}

void MainWindow::on_Resolution_valueChanged(int value){
  ui->MainDisplay->resolutionChanged(value);
}

void MainWindow::on_curveControl_toggled(bool checked){
  ui->MainDisplay->curveControlChanged(checked);
}

void MainWindow::on_showGradient_toggled(bool checked){
  ui->MainDisplay->showGradientChanged(checked);
}

/**
 * @brief MainWindow::~MainWindow Deconstructs the main window.
 */
MainWindow::~MainWindow() {
  delete ui;
}
