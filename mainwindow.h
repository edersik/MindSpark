#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "quizeditor.h"
#include "quiztaker.h"
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateQuiz();
    void onOpenQuiz();
    void onAbout();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
