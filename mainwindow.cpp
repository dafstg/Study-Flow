#include "mainwindow.h"

#include <QHBoxLayout>
#include <QFrame>
#include <QCheckBox>
#include <QScrollArea>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDate>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(1400, 850);

    todayDate = QDate::currentDate().toString("yyyy-MM-dd");

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    timer = new QTimer(this);

    dailyResetChecker = new QTimer(this);
    dailyResetChecker->start(60000);

    alarmSound = new QSoundEffect(this);
    alarmSound->setSource(QUrl::fromLocalFile("alarm.wav"));
    alarmSound->setVolume(0.9);

    createWelcomePage();
    createHomePage();
    createHistoryPage();
    createTimeUpPage();

    stackedWidget->addWidget(welcomePage);
    stackedWidget->addWidget(homePage);
    stackedWidget->addWidget(historyPage);
    stackedWidget->addWidget(timeUpPage);

    loadTasks();
    renderTodayTasks();
    renderHistory();
    updateProgress();

    stackedWidget->setCurrentWidget(welcomePage);

    connect(dailyResetChecker, &QTimer::timeout, this, [=]()
            {
                QString nowDate = QDate::currentDate().toString("yyyy-MM-dd");

                if (nowDate != todayDate)
                {
                    todayDate = nowDate;
                    renderTodayTasks();
                    renderHistory();
                    updateProgress();
                    saveTasks();
                }
            });
}

void MainWindow::createWelcomePage()
{
    welcomePage = new QWidget;

    welcomePage->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #ffe1f0, stop:0.5 #dbeafe, stop:1 #f3e8ff);"
        );

    QVBoxLayout *layout = new QVBoxLayout(welcomePage);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(22);

    QLabel *welcome = new QLabel("W E L C O M E   T O");
    welcome->setAlignment(Qt::AlignCenter);
    welcome->setStyleSheet("color:#7c3aed; font-size:22px; letter-spacing:8px;");

    QLabel *title = new QLabel("STUDY FLOW");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size:82px;"
        "font-weight:900;"
        "color:#4f46e5;"
        "letter-spacing:4px;"
        );

    QLabel *by = new QLabel("by Cognitive Error");
    by->setAlignment(Qt::AlignCenter);
    by->setStyleSheet("color:#64748b; font-size:20px;");

    QPushButton *playButton = new QPushButton("▶");
    playButton->setFixedSize(110, 110);
    playButton->setCursor(Qt::PointingHandCursor);
    playButton->setStyleSheet(
        "QPushButton{background:#7c3aed; color:white; border:none;"
        "border-radius:55px; font-size:45px; padding-left:8px;}"
        "QPushButton:hover{background:#9333ea;}"
        );

    QLabel *tap = new QLabel("TAP TO BEGIN");
    tap->setAlignment(Qt::AlignCenter);
    tap->setStyleSheet("color:#64748b; font-size:18px; letter-spacing:4px;");

    layout->addWidget(welcome);
    layout->addWidget(title);
    layout->addWidget(by);
    layout->addWidget(playButton, 0, Qt::AlignCenter);
    layout->addWidget(tap);

    connect(playButton, &QPushButton::clicked,
            this, &MainWindow::showHomePage);
}

void MainWindow::createHomePage()
{
    homePage = new QWidget;

    homePage->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #fff7ed, stop:0.45 #e0f2fe, stop:1 #f5d0fe);"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(homePage);
    mainLayout->setContentsMargins(280, 45, 280, 45);
    mainLayout->setSpacing(16);

    QHBoxLayout *topLayout = new QHBoxLayout;

    QPushButton *back = new QPushButton("< Back");
    back->setCursor(Qt::PointingHandCursor);
    back->setStyleSheet(
        "QPushButton{background:transparent; border:none; color:#475569;"
        "font-size:20px; text-align:left;}"
        "QPushButton:hover{color:#7c3aed;}"
        );

    QPushButton *historyBtn = new QPushButton("History");
    historyBtn->setCursor(Qt::PointingHandCursor);
    historyBtn->setStyleSheet(
        "QPushButton{background:#06b6d4; color:white; border:none;"
        "border-radius:15px; font-size:16px; font-weight:bold; padding:10px 18px;}"
        "QPushButton:hover{background:#0891b2;}"
        );

    topLayout->addWidget(back);
    topLayout->addStretch();
    topLayout->addWidget(historyBtn);

    QLabel *title = new QLabel("STUDY FLOW");
    title->setStyleSheet(
        "font-size:46px;"
        "font-weight:800;"
        "letter-spacing:4px;"
        "color:#4f46e5;"
        );

    QLabel *progressText = new QLabel("PROGRESS");
    progressText->setStyleSheet("color:#334155; font-size:18px; letter-spacing:2px;");

    progressBar = new QProgressBar;
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(
        "QProgressBar{background:white; border-radius:10px; height:22px;"
        "color:#334155; font-weight:bold;}"
        "QProgressBar::chunk{background:#8b5cf6; border-radius:10px;}"
        );

    taskInfo = new QLabel("0 of 0 tasks completed");
    taskInfo->setStyleSheet("color:#475569; font-size:16px;");

    QLabel *dailyTitle = new QLabel("DAILY GOAL");
    dailyTitle->setStyleSheet("color:#334155; font-size:18px; letter-spacing:2px;");

    dailyGoalBar = new QProgressBar;
    dailyGoalBar->setValue(0);
    dailyGoalBar->setTextVisible(true);
    dailyGoalBar->setStyleSheet(
        "QProgressBar{background:white; border-radius:10px; height:22px;"
        "color:#334155; font-weight:bold;}"
        "QProgressBar::chunk{background:#06b6d4; border-radius:10px;}"
        );

    dailyGoalText = new QLabel("0 / 5 tugas hari ini");
    dailyGoalText->setStyleSheet("color:#475569; font-size:16px;");

    QHBoxLayout *inputLayout = new QHBoxLayout;

    taskInput = new QLineEdit;
    taskInput->setPlaceholderText("Tambah tugas baru...");
    taskInput->setStyleSheet(
        "QLineEdit{background:white; border:2px solid #c4b5fd;"
        "border-radius:20px; padding:18px; font-size:22px; color:#1e293b;}"
        );

    addButton = new QPushButton("+");
    addButton->setFixedSize(78, 78);
    addButton->setCursor(Qt::PointingHandCursor);
    addButton->setStyleSheet(
        "QPushButton{background:#8b5cf6; color:white; border:none;"
        "border-radius:20px; font-size:40px;}"
        "QPushButton:hover{background:#7c3aed;}"
        );

    inputLayout->addWidget(taskInput);
    inputLayout->addWidget(addButton);

    QHBoxLayout *timerLayout = new QHBoxLayout;

    QLabel *timerLabel = new QLabel("⏱ Timer:");
    timerLabel->setStyleSheet("color:#334155; font-size:18px;");

    btn15 = new QPushButton("15m");
    btn30 = new QPushButton("30m");
    btn60 = new QPushButton("60m");

    QList<QPushButton*> timerButtons = {btn15, btn30, btn60};

    for (auto btn : timerButtons)
    {
        btn->setFixedSize(90, 48);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton{background:white; border:2px solid #a78bfa;"
            "border-radius:15px; color:#6d28d9; font-size:18px; font-weight:bold;}"
            "QPushButton:hover{background:#ede9fe;}"
            );

        timerLayout->addWidget(btn);
    }

    customMinuteInput = new QSpinBox;
    customMinuteInput->setRange(1, 180);
    customMinuteInput->setValue(10);
    customMinuteInput->setSuffix(" menit");
    customMinuteInput->setStyleSheet(
        "QSpinBox{background:white; border:2px solid #a78bfa;"
        "border-radius:15px; padding:10px; font-size:16px; color:#1e293b;}"
        );

    customTimerBtn = new QPushButton("Start");
    customTimerBtn->setFixedHeight(48);
    customTimerBtn->setCursor(Qt::PointingHandCursor);
    customTimerBtn->setStyleSheet(
        "QPushButton{background:#06b6d4; color:white; border:none;"
        "border-radius:15px; font-size:16px; font-weight:bold; padding:10px 22px;}"
        "QPushButton:hover{background:#0891b2;}"
        );

    timerLayout->insertWidget(0, timerLabel);
    timerLayout->addWidget(customMinuteInput);
    timerLayout->addWidget(customTimerBtn);
    timerLayout->addStretch();

    timerDisplay = new QLabel("00:00");
    timerDisplay->setStyleSheet("color:#7c3aed; font-size:36px; font-weight:bold;");

    QWidget *taskContainer = new QWidget;
    taskLayout = new QVBoxLayout(taskContainer);
    taskLayout->setSpacing(16);
    taskLayout->addStretch();

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidget(taskContainer);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea{border:none; background:transparent;}"
        "QScrollArea > QWidget > QWidget{background:transparent;}"
        );

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(title);
    mainLayout->addWidget(progressText);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(taskInfo);
    mainLayout->addWidget(dailyTitle);
    mainLayout->addWidget(dailyGoalBar);
    mainLayout->addWidget(dailyGoalText);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(timerLayout);
    mainLayout->addWidget(timerDisplay);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(scroll);

    connect(back, &QPushButton::clicked,
            this, &MainWindow::showWelcomePage);

    connect(historyBtn, &QPushButton::clicked,
            this, &MainWindow::showHistoryPage);

    connect(addButton, &QPushButton::clicked,
            this, &MainWindow::addTask);

    connect(btn15, &QPushButton::clicked, this, [=]() {
        startTimer(15);
    });

    connect(btn30, &QPushButton::clicked, this, [=]() {
        startTimer(30);
    });

    connect(btn60, &QPushButton::clicked, this, [=]() {
        startTimer(60);
    });

    connect(customTimerBtn, &QPushButton::clicked, this, [=]() {
        startTimer(customMinuteInput->value());
    });

    connect(timer, &QTimer::timeout, this, [=]() {
        remainingSeconds--;

        int minutes = remainingSeconds / 60;
        int seconds = remainingSeconds % 60;

        timerDisplay->setText(
            QString("%1:%2")
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'))
            );

        if (remainingSeconds <= 0)
        {
            timer->stop();

            if (alarmSound->isLoaded())
                alarmSound->play();

            showTimeUpPage();
        }
    });
}

void MainWindow::createHistoryPage()
{
    historyPage = new QWidget;

    historyPage->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #ecfeff, stop:0.5 #eef2ff, stop:1 #fae8ff);"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(historyPage);
    mainLayout->setContentsMargins(260, 50, 260, 50);
    mainLayout->setSpacing(18);

    QPushButton *backHome = new QPushButton("< Kembali");
    backHome->setCursor(Qt::PointingHandCursor);
    backHome->setStyleSheet(
        "QPushButton{background:transparent; border:none; color:#475569;"
        "font-size:20px; text-align:left;}"
        "QPushButton:hover{color:#7c3aed;}"
        );

    QLabel *title = new QLabel("HISTORY TASK");
    title->setStyleSheet(
        "font-size:44px; font-weight:900; color:#4f46e5; letter-spacing:3px;"
        );

    QLabel *desc = new QLabel("Riwayat tugas hari ini dan hari sebelumnya. Kamu bisa mengerjakan ulang tugas lama.");
    desc->setStyleSheet("font-size:17px; color:#475569;");

    QWidget *historyContainer = new QWidget;
    historyLayout = new QVBoxLayout(historyContainer);
    historyLayout->setSpacing(14);
    historyLayout->addStretch();

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidget(historyContainer);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet(
        "QScrollArea{border:none; background:transparent;}"
        "QScrollArea > QWidget > QWidget{background:transparent;}"
        );

    mainLayout->addWidget(backHome);
    mainLayout->addWidget(title);
    mainLayout->addWidget(desc);
    mainLayout->addWidget(scroll);

    connect(backHome, &QPushButton::clicked,
            this, &MainWindow::showHomePage);
}

void MainWindow::createTimeUpPage()
{
    timeUpPage = new QWidget;

    timeUpPage->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #fef3c7, stop:0.5 #fecaca, stop:1 #ddd6fe);"
        );

    QVBoxLayout *layout = new QVBoxLayout(timeUpPage);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(25);

    QLabel *emoji = new QLabel("⏰");
    emoji->setAlignment(Qt::AlignCenter);
    emoji->setStyleSheet("font-size:90px;");

    QLabel *title = new QLabel("WAKTU SUDAH HABIS!");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size:54px; font-weight:900; color:#dc2626; letter-spacing:2px;"
        );

    QLabel *desc = new QLabel("Istirahat sebentar, lalu lanjutkan lagi fokusmu.");
    desc->setAlignment(Qt::AlignCenter);
    desc->setStyleSheet("font-size:22px; color:#475569;");

    QPushButton *backBtn = new QPushButton("Kembali ke Study Flow");
    backBtn->setFixedHeight(60);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton{background:#7c3aed; color:white; border:none;"
        "border-radius:20px; font-size:20px; font-weight:bold; padding:12px 25px;}"
        "QPushButton:hover{background:#6d28d9;}"
        );

    layout->addWidget(emoji);
    layout->addWidget(title);
    layout->addWidget(desc);
    layout->addWidget(backBtn, 0, Qt::AlignCenter);

    connect(backBtn, &QPushButton::clicked, this, [=]() {
        alarmSound->stop();
        timerDisplay->setText("00:00");
        stackedWidget->setCurrentWidget(homePage);
    });
}

void MainWindow::showHomePage()
{
    renderTodayTasks();
    updateProgress();
    stackedWidget->setCurrentWidget(homePage);
}

void MainWindow::showWelcomePage()
{
    stackedWidget->setCurrentWidget(welcomePage);
}

void MainWindow::showHistoryPage()
{
    renderHistory();
    stackedWidget->setCurrentWidget(historyPage);
}

void MainWindow::showTimeUpPage()
{
    stackedWidget->setCurrentWidget(timeUpPage);
}

void MainWindow::startTimer(int minutes)
{
    remainingSeconds = minutes * 60;

    timerDisplay->setText(
        QString("%1:00").arg(minutes, 2, 10, QChar('0'))
        );

    timer->start(1000);
}

void MainWindow::addTask()
{
    QString text = taskInput->text().trimmed();

    if (text.isEmpty())
        return;

    TaskData task;
    task.text = text;
    task.date = todayDate;
    task.checked = false;

    tasks.append(task);

    taskInput->clear();

    saveTasks();
    renderTodayTasks();
    renderHistory();
    updateProgress();
}

void MainWindow::renderTodayTasks()
{
    clearLayout(taskLayout);

    for (int i = 0; i < tasks.size(); i++)
    {
        if (tasks[i].date != todayDate)
            continue;

        QFrame *card = new QFrame;
        card->setMinimumHeight(85);
        card->setStyleSheet(
            "background:white;"
            "border:2px solid #c4b5fd;"
            "border-radius:22px;"
            );

        QHBoxLayout *layout = new QHBoxLayout(card);

        QCheckBox *check = new QCheckBox;
        check->setChecked(tasks[i].checked);

        check->setStyleSheet(
            "QCheckBox::indicator{width:28px; height:28px; border-radius:14px;"
            "border:2px solid #8b5cf6; background:white;}"
            "QCheckBox::indicator:checked{background:#8b5cf6;}"
            );

        QLabel *label = new QLabel(tasks[i].text);
        label->setStyleSheet("color:#1e293b; font-size:24px; font-weight:500;");

        layout->addWidget(check);
        layout->addWidget(label);
        layout->addStretch();

        taskLayout->addWidget(card);

        connect(check, &QCheckBox::checkStateChanged, this, [=](Qt::CheckState) {
            tasks[i].checked = check->isChecked();
            saveTasks();
            renderHistory();
            updateProgress();
        });
    }

    taskLayout->addStretch();
}

void MainWindow::renderHistory()
{
    clearLayout(historyLayout);

    if (tasks.isEmpty())
    {
        QLabel *empty = new QLabel("Belum ada history tugas.");
        empty->setStyleSheet("font-size:20px; color:#64748b;");
        historyLayout->addWidget(empty);
        historyLayout->addStretch();
        return;
    }

    for (int i = tasks.size() - 1; i >= 0; i--)
    {
        QFrame *card = new QFrame;
        card->setMinimumHeight(90);
        card->setStyleSheet(
            "background:white;"
            "border:2px solid #bae6fd;"
            "border-radius:20px;"
            );

        QHBoxLayout *layout = new QHBoxLayout(card);

        QString status = tasks[i].checked ? "Selesai" : "Belum selesai";

        QLabel *text = new QLabel(
            tasks[i].text + "\nTanggal: " + tasks[i].date + " | " + status
            );

        text->setStyleSheet("font-size:18px; color:#1e293b;");

        QPushButton *redoBtn = new QPushButton("Kerjakan Ulang");
        redoBtn->setCursor(Qt::PointingHandCursor);
        redoBtn->setStyleSheet(
            "QPushButton{background:#8b5cf6; color:white; border:none;"
            "border-radius:14px; font-size:15px; font-weight:bold; padding:10px 16px;}"
            "QPushButton:hover{background:#7c3aed;}"
            );

        layout->addWidget(text);
        layout->addStretch();
        layout->addWidget(redoBtn);

        historyLayout->addWidget(card);

        connect(redoBtn, &QPushButton::clicked, this, [=]() {
            TaskData repeatedTask;
            repeatedTask.text = tasks[i].text + " (ulang)";
            repeatedTask.date = todayDate;
            repeatedTask.checked = false;

            tasks.append(repeatedTask);

            saveTasks();
            renderTodayTasks();
            renderHistory();
            updateProgress();

            stackedWidget->setCurrentWidget(homePage);
        });
    }

    historyLayout->addStretch();
}

void MainWindow::updateProgress()
{
    int totalToday = 0;
    int completedToday = 0;

    for (const TaskData &task : tasks)
    {
        if (task.date == todayDate)
        {
            totalToday++;

            if (task.checked)
                completedToday++;
        }
    }

    int progress = 0;

    if (totalToday > 0)
        progress = (completedToday * 100) / totalToday;

    progressBar->setValue(progress);

    taskInfo->setText(
        QString("%1 of %2 tasks completed")
            .arg(completedToday)
            .arg(totalToday)
        );

    int dailyPercent = 0;

    if (dailyGoal > 0)
        dailyPercent = qMin(100, (completedToday * 100) / dailyGoal);

    dailyGoalBar->setValue(dailyPercent);

    dailyGoalText->setText(
        QString("%1 / %2 tugas hari ini")
            .arg(completedToday)
            .arg(dailyGoal)
        );
}

void MainWindow::saveTasks()
{
    QJsonArray array;

    for (const TaskData &task : tasks)
    {
        QJsonObject obj;
        obj["text"] = task.text;
        obj["date"] = task.date;
        obj["checked"] = task.checked;

        array.append(obj);
    }

    QJsonDocument doc(array);

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(path);

    QFile file(path + "/studyflow_tasks.json");

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::loadTasks()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFile file(path + "/studyflow_tasks.json");

    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray array = doc.array();

    tasks.clear();

    for (const QJsonValue &value : array)
    {
        QJsonObject obj = value.toObject();

        TaskData task;
        task.text = obj["text"].toString();
        task.date = obj["date"].toString();
        task.checked = obj["checked"].toBool();

        tasks.append(task);
    }
}

void MainWindow::clearLayout(QLayout *layout)
{
    if (!layout)
        return;

    while (QLayoutItem *item = layout->takeAt(0))
    {
        if (item->widget())
        {
            item->widget()->deleteLater();
        }

        if (item->layout())
        {
            clearLayout(item->layout());
        }

        delete item;
    }
}