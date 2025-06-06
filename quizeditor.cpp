#include "quizeditor.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>



QuizEditor::QuizEditor(QWidget *parent)
    : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);

    questionEdit = new QLineEdit(this);
    questionEdit->setPlaceholderText("Введите вопрос");
    layout->addWidget(questionEdit);

    for (int i = 0; i < 4; ++i) {
        optionEdits[i] = new QLineEdit(this);
        optionEdits[i]->setPlaceholderText(QString("Вариант %1").arg(i + 1));

        checkBoxes[i] = new QCheckBox("Правильный", this);

        auto *row = new QHBoxLayout;
        row->addWidget(checkBoxes[i]);
        row->addWidget(optionEdits[i]);
        layout->addLayout(row);
    }


    difficultyBox = new QComboBox(this);
    difficultyBox->addItem("Лёгкий", 1);
    difficultyBox->addItem("Средний", 2);
    difficultyBox->addItem("Сложный", 3);
    layout->addWidget(new QLabel("Сложность:"));
    layout->addWidget(difficultyBox);


    addButton = new QPushButton("Добавить вопрос", this);
    saveButton = new QPushButton("Сохранить тест", this);
    layout->addWidget(addButton);
    layout->addWidget(saveButton);

    questionList = new QListWidget(this);
    layout->addWidget(questionList);

    connect(addButton, &QPushButton::clicked, this, &QuizEditor::addQuestion);
    connect(saveButton, &QPushButton::clicked, this, &QuizEditor::saveQuiz);

    this->setStyleSheet(R"(
        QWidget {
            background-color: #ffe4f0;
            font-family: "Segoe UI", sans-serif;
        }
        QLineEdit, QListWidget {
            background: #fff0f8;
            border: 1px solid #ffaad4;
            border-radius: 6px;
            padding: 4px;
        }
        QRadioButton {
            color: #d81b60;
        }
        QPushButton {
            background-color: #ffaad4;
            border: 2px solid white;
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 6px;
        }
        QPushButton:hover {
            background-color: #ff8fb6;
        }
    )");
}

void QuizEditor::addQuestion() {
    QString question = questionEdit->text();

    QStringList options;
    for (int i = 0; i < 4; ++i) {
        options << optionEdits[i]->text();
    }

    QJsonArray correctIndexes;
    for (int i = 0; i < 4; ++i) {
        if (checkBoxes[i]->isChecked()) {
            correctIndexes.append(i);
        }
    }

    if (question.isEmpty() || options.contains("") || correctIndexes.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля и выберите хотя бы один правильный ответ");
        return;
    }

    QJsonObject questionData;
    int difficulty = difficultyBox->currentData().toInt();
    questionData["difficulty"] = difficulty;
    questionData["question"] = question;
    questionData["correct"] = correctIndexes;

    QJsonArray optionsArray;
    for (const QString& option : options) {
        optionsArray.append(option);
    }
    questionData["options"] = optionsArray;

    QString difficultyStr;
    switch (difficulty) {
    case 1: difficultyStr = "Лёгкий"; break;
    case 2: difficultyStr = "Средний"; break;
    case 3: difficultyStr = "Сложный"; break;
    default: difficultyStr = "Неизвестно";
    }

    QStringList correctOptions;
    for (auto val : correctIndexes) {
        int idx = val.toInt();
        if (idx >= 0 && idx < options.size()) {
            correctOptions << options[idx];
        }
    }

    QString display = QString("Вопрос: %1\nПравильные ответы: %2\nСложность: %3")
                          .arg(question)
                          .arg(correctOptions.join(", "))
                          .arg(difficultyStr);
    QListWidgetItem* item = new QListWidgetItem(display);
    item->setData(Qt::UserRole, questionData);
    questionList->addItem(item);

    questionEdit->clear();
    for (int i = 0; i < 4; ++i) {
        optionEdits[i]->clear();
        checkBoxes[i]->setChecked(false);
    }
}


void QuizEditor::saveQuiz() {
    if (questionList->count() == 0) {
        QMessageBox::warning(this, "Ошибка", "Нет вопросов для сохранения");
        return;
    }

    QJsonArray quizArray;
    for (int i = 0; i < questionList->count(); ++i) {
        quizArray.append(questionList->item(i)->data(Qt::UserRole).toJsonObject());
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить викторину", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(quizArray).toJson());
        file.close();
        QMessageBox::information(this, "Успех", "Викторина сохранена");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл");
    }
}

QListWidget* QuizEditor::getQuestionList()
{
    return questionList;
}
