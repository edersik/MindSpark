#ifndef QUIZEDITOR_H
#define QUIZEDITOR_H

#include <QWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>

class QuizEditor : public QWidget {
    Q_OBJECT

public:
    QuizEditor(QWidget *parent = nullptr);
    QListWidget* getQuestionList();

private slots:
    void addQuestion();
    void saveQuiz();

private:
    QLineEdit *questionEdit;
    QLineEdit *optionEdits[4];
    QCheckBox *checkBoxes[4];

    QListWidget *questionList;
    QPushButton *addButton;
    QPushButton *saveButton;
    QComboBox *difficultyBox;

};

#endif // QUIZEDITOR_H
