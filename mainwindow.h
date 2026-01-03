#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QMainWindow>


namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class represents the main window.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

 private slots:
  void on_PresetSelect_currentIndexChanged(int index);
  void on_Resolution_valueChanged(int value);
  void on_curveControl_toggled(bool checked);
  void on_showGradient_toggled(bool checked);
 private:
  Ui::MainWindow *ui;
};

#endif  // MAINWINDOW_H
