#ifndef QUIZTAKER_H
#define QUIZTAKER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSet>
#include <QJsonArray>
#include <QTimer>
#include <QTime>
#include <QTableWidget>
#include <QHBoxLayout>


class QuizTaker : public QWidget
{
    Q_OBJECT

public:
    explicit QuizTaker(const QString &fileName, QWidget *parent = nullptr);

private slots:
    void submitAnswer();
    void updateTimer();
    void timeIsUp();
    void restartQuiz();

private:
    void loadQuestion();
    void finishQuiz(bool timeUp = false);
    void askForNameAndSaveScore();
    void loadScoresToTable();
    void initScoreTable();
    void showScoreTableOnly();

    QTableWidget *scoreTable;
    QPushButton  *againButton;
    QPushButton  *exitButton;

    QLabel *questionLabel;
    QCheckBox *optionBoxes[4];
    QSet<QString> currentCorrectAnswers;
    QPushButton *submitButton;

    QVBoxLayout *layout;

    QJsonArray quizData;
    int currentQuestionIndex;
    int score;

    QTimer *quizTimer;
    QTime remainingTime;
    QLabel *timerLabel;

};

#endif // QUIZTAKER_H
