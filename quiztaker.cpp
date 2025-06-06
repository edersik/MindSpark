#include "quiztaker.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>
#include <random>
#include <QTimer>
#include <QHeaderView>

QuizTaker::QuizTaker(const QString &fileName, QWidget *parent)
    : QWidget(parent), currentQuestionIndex(0), score(0)
{
    this->resize(800, 600);
    this->setMinimumSize(600, 400);
    layout = new QVBoxLayout(this);

    questionLabel = new QLabel(this);
    questionLabel->setWordWrap(true);
    layout->addWidget(questionLabel);

    for (int i = 0; i < 4; ++i) {
        optionBoxes[i] = new QCheckBox(this);
        layout->addWidget(optionBoxes[i]);
    }

    submitButton = new QPushButton("Ответить", this);
    layout->addWidget(submitButton);

    connect(submitButton, &QPushButton::clicked, this, &QuizTaker::submitAnswer);


    scoreTable = new QTableWidget(this);
    scoreTable->setColumnCount(2);
    scoreTable->setHorizontalHeaderLabels({"ФИО", "Баллы"});
    scoreTable->horizontalHeader()->setStretchLastSection(true);
    scoreTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scoreTable->setSelectionMode(QAbstractItemView::NoSelection);
    scoreTable->setFixedHeight(200);
    layout->addWidget(scoreTable);

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        quizData = doc.array();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть викторину.");
    }


    this->setStyleSheet(R"(
    QWidget {
        background-color: #ffe4f0;
        font-family: "Segoe UI", sans-serif;
        font-size: 18px;
    }
    QLabel {
        color: #d81b60;
        font-size: 22px;
        font-weight: bold;
        padding: 8px;
    }
    QCheckBox {
        color: #6a1b9a;
        font-size: 20px;
        padding: 6px;
    }
    QPushButton {
        background-color: #ffaad4;
        border: 2px solid white;
        border-radius: 10px;
        color: white;
        font-weight: bold;
        padding: 12px 20px;
        font-size: 20px;
    }
    QPushButton:hover {
        background-color: #ff8fb6;
    }
)");

    loadQuestion();

    int totalSeconds = 0;
    for (const QJsonValue &val : quizData) {
        int difficulty = val.toObject().value("difficulty").toInt(1);
        switch (difficulty) {
        case 1: totalSeconds += 30; break;
        case 2: totalSeconds += 60; break;
        case 3: totalSeconds += 90; break;
        default: totalSeconds += 60;
        }
    }
    remainingTime = QTime(0, 0).addSecs(totalSeconds);


    timerLabel = new QLabel(this);
    timerLabel->setText(remainingTime.toString("mm:ss"));
    layout->addWidget(timerLabel);

    quizTimer = new QTimer(this);
    connect(quizTimer, &QTimer::timeout, this, &QuizTaker::updateTimer);
    quizTimer->start(1000);

    loadScoresToTable();
    auto *btnRow = new QHBoxLayout;
    againButton = new QPushButton("Пройти снова", this);
    exitButton  = new QPushButton("Выход",        this);
    btnRow->addWidget(againButton);
    btnRow->addWidget(exitButton);
    layout->addLayout(btnRow);

    againButton->hide();
    exitButton ->hide();

    connect(againButton, &QPushButton::clicked, this, &QuizTaker::restartQuiz);
    connect(exitButton , &QPushButton::clicked, this, &QWidget::close);

}

void QuizTaker::loadQuestion()
{
    if (currentQuestionIndex >= quizData.size()) {
        finishQuiz();
        return;
    }

    QJsonObject obj = quizData[currentQuestionIndex].toObject();
    QString question = obj["question"].toString();
    QJsonArray optionsArray = obj["options"].toArray();

    QJsonArray correctArray = obj["correct"].toArray();
    QSet<QString> correctAnswers;
    for (const QJsonValue &val : correctArray) {
        int idx = val.toInt();
        if (idx >= 0 && idx < optionsArray.size())
            correctAnswers.insert(optionsArray[idx].toString());
    }
    currentCorrectAnswers = correctAnswers;

    QStringList options;
    for (const QJsonValue &val : optionsArray)
        options << val.toString();

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(options.begin(), options.end(), g);

    questionLabel->setText(QString("Вопрос %1:\n%2").arg(currentQuestionIndex + 1).arg(question));

    for (int i = 0; i < 4; ++i) {
        optionBoxes[i]->setText(options[i]);
        optionBoxes[i]->setChecked(false);
    }
}

void QuizTaker::submitAnswer()
{
    QSet<QString> selectedAnswers;
    for (int i = 0; i < 4; ++i) {
        if (optionBoxes[i]->isChecked()) {
            selectedAnswers.insert(optionBoxes[i]->text());
        }
    }

    if (selectedAnswers.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы один вариант!");
        return;
    }

    if (selectedAnswers == currentCorrectAnswers) {
        QJsonObject obj = quizData[currentQuestionIndex].toObject();
        int difficulty = obj.value("difficulty").toInt(1);
        score += difficulty;
    }

    currentQuestionIndex++;
    loadQuestion();
}

void QuizTaker::timeIsUp()
{
    quizTimer->stop();
    QMessageBox::information(this, "Время вышло", "Время вышло! Викторина завершена.");
    finishQuiz(true);
}


void QuizTaker::updateTimer()
{
    remainingTime = remainingTime.addSecs(-1);
    timerLabel->setText(remainingTime.toString("mm:ss"));

    if (remainingTime == QTime(0, 0, 0)) {
        timeIsUp();
    }
}

void QuizTaker::finishQuiz(bool timeUp)
{
     quizTimer->stop();
    QMessageBox::information(this,"Результат",
                             QString("Вы набрали %1 балл(ов).").arg(score));
    if (score>0) askForNameAndSaveScore();
    showScoreTableOnly();
}




void QuizTaker::askForNameAndSaveScore()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Введите ФИО",
                                         "Пожалуйста, введите ФИО для таблицы рекордов:",
                                         QLineEdit::Normal, "", &ok);

    if (ok && !name.trimmed().isEmpty()) {
        QFile file("scores.json");
        QJsonArray scoresArray;

        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            scoresArray = doc.array();
            file.close();
        }

        QJsonObject newRecord;
        newRecord["name"] = name.trimmed();
        newRecord["score"] = score;

        scoresArray.append(newRecord);

        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(scoresArray);
            file.write(doc.toJson());
            file.close();
        }
        loadScoresToTable();
    }
}

void QuizTaker::loadScoresToTable()
{
    QFile file("scores.json");
    if (!file.open(QIODevice::ReadOnly)) {
        scoreTable->setRowCount(0);
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray scoresArray = doc.array();

    QList<QJsonObject> records;
    for (const auto& val : scoresArray) {
        records.append(val.toObject());
    }
    std::sort(records.begin(), records.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["score"].toInt() > b["score"].toInt();
    });

    scoreTable->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        scoreTable->setItem(i, 0, new QTableWidgetItem(records[i]["name"].toString()));
        scoreTable->setItem(i, 1, new QTableWidgetItem(QString::number(records[i]["score"].toInt())));
    }
}



void QuizTaker::showScoreTableOnly() {
    questionLabel->hide();
    for (int i = 0; i < 4; ++i)
        optionBoxes[i]->hide();
    submitButton->hide();
    timerLabel->hide();

    scoreTable->clearContents();
    QFile file("scores.json");
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray scoresArray = doc.array();

    QList<QJsonObject> records;
    for (const auto& val : scoresArray)
        records.append(val.toObject());

    std::sort(records.begin(), records.end(), [](const QJsonObject &a, const QJsonObject &b) {
        return a["score"].toInt() > b["score"].toInt();
    });

    scoreTable->setRowCount(records.size());
    for (int i = 0; i < records.size(); ++i) {
        scoreTable->setItem(i, 0, new QTableWidgetItem(records[i]["name"].toString()));
        scoreTable->setItem(i, 1, new QTableWidgetItem(QString::number(records[i]["score"].toInt())));
    }

    scoreTable->show();
    againButton->show();
    exitButton ->show();

}

void QuizTaker::restartQuiz()
{
    currentQuestionIndex = 0;
    score = 0;

    questionLabel->show();
    for (int i = 0; i < 4; ++i) optionBoxes[i]->show();
    submitButton   ->show();
    timerLabel     ->show();
    againButton    ->hide();
    exitButton     ->hide();
    scoreTable     ->hide();

    int totalSeconds = 0;
    for (const QJsonValue &val : quizData) {
        int difficulty = val.toObject().value("difficulty").toInt(1);
        switch (difficulty) {
        case 1: totalSeconds += 30; break;
        case 2: totalSeconds += 60; break;
        case 3: totalSeconds += 90; break;
        default: totalSeconds += 60;
        }
    }
    remainingTime = QTime(0, 0).addSecs(totalSeconds);
    timerLabel->setText(remainingTime.toString("mm:ss"));
    timerLabel->setText(remainingTime.toString("mm:ss"));
    quizTimer->start(1000);

    loadQuestion();
}

