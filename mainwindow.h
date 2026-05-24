#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTimer>
#include <QLabel>
#include <QSpinBox>
#include <QSoundEffect>
#include <QVector>

struct TaskData
{
    QString text;
    QString date;
    bool checked;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void showHomePage();
    void showWelcomePage();
    void showHistoryPage();
    void showTimeUpPage();
    void addTask();
    void updateProgress();
    void startTimer(int minutes);

private:
    QStackedWidget *stackedWidget;

    QWidget *welcomePage;
    QWidget *homePage;
    QWidget *historyPage;
    QWidget *timeUpPage;

    QLineEdit *taskInput;
    QPushButton *addButton;

    QPushButton *btn15;
    QPushButton *btn30;
    QPushButton *btn60;
    QPushButton *customTimerBtn;

    QSpinBox *customMinuteInput;

    QVBoxLayout *taskLayout;
    QVBoxLayout *historyLayout;

    QProgressBar *progressBar;
    QProgressBar *dailyGoalBar;

    QLabel *timerDisplay;
    QLabel *dailyGoalText;
    QLabel *taskInfo;

    QTimer *timer;
    QTimer *dailyResetChecker;
    QSoundEffect *alarmSound;

    QVector<TaskData> tasks;

    int remainingSeconds = 0;
    int dailyGoal = 5;

    QString todayDate;

    void createWelcomePage();
    void createHomePage();
    void createHistoryPage();
    void createTimeUpPage();

    void renderTodayTasks();
    void renderHistory();
    void saveTasks();
    void loadTasks();
    void clearLayout(QLayout *layout);
};

#endif