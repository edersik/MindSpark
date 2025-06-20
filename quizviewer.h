#ifndef QUIZVIEWER_H
#define QUIZVIEWER_H

#include <QWidget>
#include <QString>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

class QuizViewer : public QWidget
{
    Q_OBJECT

public:
    explicit QuizViewer(const QString &fileName, QWidget *mainWindow = nullptr);

private slots:
    void startQuiz();
    void onQuestionSelected(QListWidgetItem *item);
    void saveCurrentQuestion();

private:
    void loadQuizFile(const QString &fileName);
    void saveToOriginalFile();

    QWidget *mainWindowPtr;
    QString loadedFileName;
    QJsonArray quizData;

    QListWidget *listWidget;
    QPushButton *startButton;
    QPushButton *saveButton;

    QLineEdit *questionEdit;
    QLineEdit *answerEdits[4];
    QCheckBox *correctBoxes[4];
    QComboBox *difficultyBox;

    int currentEditingIndex = -1;
};

#endif // QUIZVIEWER_H
