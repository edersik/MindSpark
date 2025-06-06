#ifndef QUIZVIEWER_H
#define QUIZVIEWER_H

#include <QWidget>
#include <QString>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

class QListWidget;
class QPushButton;

class QuizViewer : public QWidget
{
    Q_OBJECT

public:
    explicit QuizViewer(const QString &fileName, QWidget *mainWindow = nullptr);

private slots:
    void startQuiz();

private:
    QWidget *mainWindowPtr;
    QListWidget *listWidget;
    QPushButton *startButton;
};

#endif // QUIZVIEWER_H
