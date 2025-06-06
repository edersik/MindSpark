#include "quizviewer.h"
#include "quiztaker.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QVBoxLayout>
#include <QPushButton>

QuizViewer::QuizViewer(const QString &fileName, QWidget *mainWindow)
    : QWidget(nullptr), mainWindowPtr(mainWindow) {

    this->setStyleSheet(R"(
        QWidget {
            background-color: #ffe4f0;
        }
        QListWidget {
            background-color: #fff0f8;
            border: 1px solid #ffaad4;
            border-radius: 6px;
        }
        QPushButton {
            background-color: #ffaad4;
            border: 2px solid white;
            border-radius: 10px;
            color: white;
            font-weight: bold;
            padding: 8px;
        }
        QPushButton:hover {
            background-color: #ff8fb6;
        }
    )");

    listWidget = new QListWidget(this);
    startButton = new QPushButton("Начать викторину", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
    layout->addWidget(startButton);

    connect(startButton, &QPushButton::clicked, this, &QuizViewer::startQuiz);

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray quizArray = doc.array();

        for (const QJsonValue &value : quizArray) {
            QJsonObject obj = value.toObject();
            QString question = obj["question"].toString();
            QString answer = obj["correct"].toString();
            listWidget->addItem(QString("❓ %1\n  ➤ %2").arg(question, answer));
        }
        file.close();
    }
}

void QuizViewer::startQuiz() {
    auto *quizTaker = new QuizTaker(windowTitle(), mainWindowPtr);
    quizTaker->show();
    this->close();
}
