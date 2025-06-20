#include "quizviewer.h"
#include "quiztaker.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDebug>

QuizViewer::QuizViewer(const QString &fileName, QWidget *mainWindow)
    : QWidget(nullptr), mainWindowPtr(mainWindow), loadedFileName(fileName)
{
    this->setStyleSheet(R"(
        QWidget {
            background-color: #ffe4f0;
            font-family: "Segoe UI", sans-serif;
            font-size: 16px;
        }
        QListWidget {
            background-color: #fff0f8;
            border: 1px solid #ffaad4;
            border-radius: 6px;
        }
        QLineEdit, QComboBox {
            background-color: white;
            padding: 6px;
            border-radius: 6px;
        }
        QPushButton {
            background-color: #ffaad4;
            border: 2px solid white;
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #ff8fb6;
        }
    )");

    auto *mainLayout = new QVBoxLayout(this);

    listWidget = new QListWidget(this);
    mainLayout->addWidget(listWidget);

    // Элементы редактирования
    questionEdit = new QLineEdit(this);
    mainLayout->addWidget(new QLabel("Вопрос:", this));
    mainLayout->addWidget(questionEdit);

    for (int i = 0; i < 4; ++i) {
        answerEdits[i] = new QLineEdit(this);
        correctBoxes[i] = new QCheckBox("Правильный", this);

        auto *row = new QHBoxLayout;
        row->addWidget(new QLabel(QString("Вариант %1:").arg(i + 1), this));
        row->addWidget(answerEdits[i]);
        row->addWidget(correctBoxes[i]);

        mainLayout->addLayout(row);
    }

    difficultyBox = new QComboBox(this);
    difficultyBox->addItems({"Лёгкий", "Средний", "Сложный"});
    mainLayout->addWidget(new QLabel("Сложность:", this));
    mainLayout->addWidget(difficultyBox);

    // Кнопки
    auto *btnRow = new QHBoxLayout;
    saveButton = new QPushButton("Сохранить изменения", this);
    startButton = new QPushButton("Начать викторину", this);
    btnRow->addWidget(saveButton);
    btnRow->addWidget(startButton);
    mainLayout->addLayout(btnRow);

    connect(saveButton, &QPushButton::clicked, this, &QuizViewer::saveCurrentQuestion);
    connect(startButton, &QPushButton::clicked, this, &QuizViewer::startQuiz);
    connect(listWidget, &QListWidget::itemClicked, this, &QuizViewer::onQuestionSelected);

    loadQuizFile(fileName);
}

void QuizViewer::loadQuizFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл викторины.");
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    quizData = doc.array();

    listWidget->clear();
    for (const QJsonValue &value : quizData) {
        QJsonObject obj = value.toObject();
        listWidget->addItem(obj["question"].toString());
    }
}

void QuizViewer::onQuestionSelected(QListWidgetItem *item)
{
    int index = listWidget->row(item);
    if (index < 0 || index >= quizData.size()) return;

    currentEditingIndex = index;
    QJsonObject obj = quizData[index].toObject();

    questionEdit->setText(obj["question"].toString());

    QJsonArray options = obj["options"].toArray();
    for (int i = 0; i < 4; ++i)
        answerEdits[i]->setText(options[i].toString());

    QJsonArray correct = obj["correct"].toArray();
    for (int i = 0; i < 4; ++i)
        correctBoxes[i]->setChecked(false);
    for (const QJsonValue &val : correct)
        if (val.isDouble())
            correctBoxes[val.toInt()]->setChecked(true);

    difficultyBox->setCurrentIndex(obj["difficulty"].toInt(1) - 1);
}

void QuizViewer::saveCurrentQuestion()
{
    if (currentEditingIndex < 0 || currentEditingIndex >= quizData.size()) return;

    QJsonObject obj;
    obj["question"] = questionEdit->text();

    QJsonArray options;
    for (int i = 0; i < 4; ++i)
        options.append(answerEdits[i]->text());
    obj["options"] = options;

    QJsonArray correct;
    for (int i = 0; i < 4; ++i)
        if (correctBoxes[i]->isChecked())
            correct.append(i);
    obj["correct"] = correct;

    obj["difficulty"] = difficultyBox->currentIndex() + 1;

    quizData[currentEditingIndex] = obj;
    listWidget->item(currentEditingIndex)->setText(obj["question"].toString());

    saveToOriginalFile();
    QMessageBox::information(this, "Успех", "Вопрос успешно обновлён и сохранён.");
}

void QuizViewer::saveToOriginalFile()
{
    QFile file(loadedFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл.");
        return;
    }

    QJsonDocument doc(quizData);
    file.write(doc.toJson());
    file.close();
}

void QuizViewer::startQuiz()
{
    auto *quizTaker = new QuizTaker(loadedFileName);
    quizTaker->setAttribute(Qt::WA_DeleteOnClose);
    quizTaker->setWindowTitle("Прохождение викторины");
    quizTaker->resize(800, 600);
    quizTaker->show();

    this->close();
}

