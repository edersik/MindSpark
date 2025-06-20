#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "quizeditor.h"
#include "quizviewer.h"
#include "quiztaker.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>

void MainWindow::onCreateQuiz() {
    auto *editor = new QuizEditor(nullptr);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setWindowTitle("Редактор викторин");
    editor->resize(400, 600);
    editor->show();
}

void MainWindow::onOpenQuiz()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть викторину", "", "Файлы викторин (*.json)");
    if (fileName.isEmpty())
        return;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Выберите действие");
    msgBox.setText("Что вы хотите сделать с викториной?");
    QPushButton *viewButton = msgBox.addButton("📖 Посмотреть и редактировать", QMessageBox::ActionRole);
    QPushButton *takeButton = msgBox.addButton("🏁 Пройти", QMessageBox::ActionRole);
    QPushButton *exitButton = msgBox.addButton("🚪 Выход", QMessageBox::RejectRole);

    msgBox.exec();

    if (msgBox.clickedButton() == viewButton) {
        auto *viewer = new QuizViewer(fileName, this);
        viewer->setWindowTitle("Просмотр и редактирование викторины");
        viewer->resize(600, 700);
        viewer->show();
    }
    else if (msgBox.clickedButton() == takeButton) {
        auto *taker = new QuizTaker(fileName);
        taker->setWindowTitle("Прохождение викторины");
        taker->resize(800, 600);
        taker->show();
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "О программе", "Милое приложение для викторин\nКурсовая работа Ерофеевой Дарьи Денисовны и Новиковой Дарьи Дмитриевны 🐾");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setStyleSheet("QMainWindow { background-color: #FFC0CB; }");

    QMenu *fileMenu = menuBar()->addMenu("Файл");
    QAction *createQuizAction = fileMenu->addAction("Создать викторину");
    QAction *openQuizAction = fileMenu->addAction("Открыть викторину");
    QAction *exitAction = fileMenu->addAction("Выход");

    QMenu *helpMenu = menuBar()->addMenu("Помощь");
    QAction *aboutAction = helpMenu->addAction("О программе");

    connect(createQuizAction, &QAction::triggered, this, &MainWindow::onCreateQuiz);
    connect(openQuizAction, &QAction::triggered, this, &MainWindow::onOpenQuiz);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);

    QLabel *label = new QLabel(this);
    QPixmap pixmap(":/capibara.jpg");
    label->setPixmap(pixmap.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setAlignment(Qt::AlignCenter);

    QPushButton *btnCreate = new QPushButton("Создать викторину");
    QPushButton *btnOpen = new QPushButton("Открыть викторину");

    btnCreate->setStyleSheet("QPushButton { background-color: #FF69B4; border: 2px solid white; color: white; padding: 10px; border-radius: 10px; }");
    btnOpen->setStyleSheet("QPushButton { background-color: #FF69B4; border: 2px solid white; color: white; padding: 10px; border-radius: 10px; }");

    connect(btnCreate, &QPushButton::clicked, this, &MainWindow::onCreateQuiz);
    connect(btnOpen, &QPushButton::clicked, this, &MainWindow::onOpenQuiz);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(label);
    layout->addSpacing(20);
    layout->addWidget(btnCreate);
    layout->addWidget(btnOpen);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    setCentralWidget(central);
    setWindowTitle("Милое приложение Викторин 💖");
    resize(600, 500);
}

MainWindow::~MainWindow() {
    delete ui;
}
