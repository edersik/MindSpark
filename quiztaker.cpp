#include "quiztaker.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QComboBox>
#include <random>

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

    initScoreTable();

    QFile file(fileName);
    quizFileName = QFileInfo(fileName).fileName();
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
    })");

    loadQuestion();

    int totalSeconds = 0;
    for (const QJsonValue &val : quizData) {
        int difficulty = val.toObject().value("difficulty").toInt(1);
        switch (difficulty) {
        case 1: totalSeconds += 20; break;
        case 2: totalSeconds += 35; break;
        case 3: totalSeconds += 90; break;
        default: totalSeconds += 35;
        }
    }
    remainingTime = QTime(0, 0).addSecs(totalSeconds);

    timerLabel = new QLabel(this);
    timerLabel->setText(remainingTime.toString("mm:ss"));
    layout->addWidget(timerLabel);

    quizTimer = new QTimer(this);
    connect(quizTimer, &QTimer::timeout, this, &QuizTaker::updateTimer);
    quizTimer->start(1000);

    auto *btnRow = new QHBoxLayout;
    againButton = new QPushButton("Пройти снова", this);
    exitButton  = new QPushButton("Выход", this);
    btnRow->addWidget(againButton);
    btnRow->addWidget(exitButton);
    layout->addLayout(btnRow);

    againButton->hide();
    exitButton ->hide();

    connect(againButton, &QPushButton::clicked, this, &QuizTaker::restartQuiz);
    connect(exitButton , &QPushButton::clicked, this, &QWidget::close);
}

void QuizTaker::initScoreTable()
{
    scoreTable = new QTableWidget(this);
    scoreTable->setColumnCount(2);
    scoreTable->setHorizontalHeaderLabels({"ФИО", "Баллы"});
    scoreTable->horizontalHeader()->setStretchLastSection(true);
    scoreTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scoreTable->setSelectionMode(QAbstractItemView::NoSelection);
    scoreTable->setFixedHeight(200);
    scoreTable->hide();
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
        if (optionBoxes[i]->isChecked())
            selectedAnswers.insert(optionBoxes[i]->text());
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

void QuizTaker::updateTimer()
{
    remainingTime = remainingTime.addSecs(-1);
    timerLabel->setText(remainingTime.toString("mm:ss"));

    if (remainingTime == QTime(0, 0, 0))
        timeIsUp();
}

void QuizTaker::timeIsUp()
{
    quizTimer->stop();
    QMessageBox::information(this, "Время вышло", "Время вышло! Викторина завершена.");
    finishQuiz(true);
}

void QuizTaker::finishQuiz(bool)
{
    quizTimer->stop();
    QMessageBox::information(this, "Результат",
                             QString("Вы набрали %1 балл(ов).").arg(score));
    askForNameAndSaveScore();
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
        newRecord["quiz"] = quizFileName;

        scoresArray.append(newRecord);

        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(scoresArray);
            file.write(doc.toJson());
            file.close();
        }
    }
}

void QuizTaker::loadScoresToTable(const QString &filter)
{
    QFile file("scores.json");
    if (!file.open(QIODevice::ReadOnly)) {
        scoreTable->setRowCount(0);
        return;
    }

    QJsonArray scoresArray = QJsonDocument::fromJson(file.readAll()).array();
    QList<QJsonObject> records;

    for (const QJsonValue &val : scoresArray) {
        QJsonObject obj = val.toObject();
        if (filter == "Все викторины" || obj["quiz"].toString() == filter)
            records.append(obj);
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

void QuizTaker::showScoreTableOnly()
{
    questionLabel->hide();
    for (int i = 0; i < 4; ++i)
        optionBoxes[i]->hide();
    submitButton->hide();
    timerLabel->hide();

    loadScoresToTable("Все викторины");

    layout->addWidget(scoreTable);
    scoreTable->show();

    againButton->show();
    exitButton->show();

    if (filterLayout) {
        QLayoutItem *child;
        while ((child = filterLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        layout->removeItem(filterLayout);
        delete filterLayout;
        filterLayout = nullptr;
    }

    if (!filterAdded) {
        filterLayout = new QHBoxLayout;
        QLabel *filterLabel = new QLabel("Фильтр по викторине:", this);
        QComboBox *filterBox = new QComboBox(this);
        filterBox->addItem("Все викторины");

        QFile file("scores.json");
        if (file.open(QIODevice::ReadOnly)) {
            QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
            QSet<QString> quizSet;
            for (const QJsonValue &v : arr)
                quizSet.insert(v.toObject()["quiz"].toString());
            for (const QString &quiz : quizSet)
                filterBox->addItem(quiz);
        }

        connect(filterBox, &QComboBox::currentTextChanged, this, [=](const QString &quizName) {
            loadScoresToTable(quizName);
        });

        filterLayout->addWidget(filterLabel);
        filterLayout->addWidget(filterBox);
        layout->insertLayout(layout->indexOf(scoreTable), filterLayout);
        filterAdded = true;
    }
}

void QuizTaker::restartQuiz()
{
    filterAdded = false;
    currentQuestionIndex = 0;
    score = 0;

    if (filterLayout) {
        QLayoutItem *child;
        while ((child = filterLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        layout->removeItem(filterLayout);
        delete filterLayout;
        filterLayout = nullptr;
    }

    if (scoreTable && scoreTable->parent() == this) {
        layout->removeWidget(scoreTable);
        scoreTable->hide();
    }

    questionLabel->show();
    for (int i = 0; i < 4; ++i)
        optionBoxes[i]->show();

    submitButton->show();
    timerLabel->show();
    againButton->hide();
    exitButton->hide();

    int totalSeconds = 0;
    for (const QJsonValue &val : quizData) {
        int difficulty = val.toObject().value("difficulty").toInt(1);
        switch (difficulty) {
        case 1: totalSeconds += 20; break;
        case 2: totalSeconds += 35; break;
        case 3: totalSeconds += 90; break;
        default: totalSeconds += 35;
        }
    }
    remainingTime = QTime(0, 0).addSecs(totalSeconds);
    timerLabel->setText(remainingTime.toString("mm:ss"));
    quizTimer->start(1000);

    loadQuestion();
}
