#ifndef TIMETABLEWINDOW_H
#define TIMETABLEWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QTimer>
#include <QMenu>
#include <QWidgetAction>
#include "fixtimewidget.h"

namespace Ui {
class TimeTableWindow;
}

class TreeCalendarModel;
class CalendarModel;
class CalendarTask;
class CalendarTimeSpan;

class TaskListWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TaskListWindow(QWidget *parent = 0);
    ~TaskListWindow();
public slots:
    void openFileWithDialog();
    void openFile(const QString& fileName);
    void toggleTask();
    void clearModel();
    void showError(const QString& from, const QString& text);
private slots:
    void taskSelectionChanged(const QItemSelection&, const QItemSelection& );
    void timeDetailsForCurrentSelection();
    void timeDetailsForSelection(const QModelIndex& selection);
    void timeDetailsFor(CalendarTask* task);
    void fixTimeFor(CalendarTask* task);
    void modelExistenceChanged();
private:
    static constexpr int LOGGING_SAVE_FREQUENCY = 60*1000;

    /**
     * @brief Updates the green bar at bottom of the window.
     */
    void updateActiveTaskUi();
    void updateActiveTaskTicker();
    CalendarTask* selectedTask() const;

    Ui::TimeTableWindow *ui;
    CalendarModel* mModel;
    TreeCalendarModel* mTreeModel;

    QMenu mFixTimeMenu;
    FixTimeWidget mFixTimeMenuWidget;


    CalendarTask* mActiveTask;
    CalendarTimeSpan* mActiveTimeSpan;
    QTimer mActiveTaskTimeUpdater;
    /**
     * @brief Active during logging.
     */
    QTimer mPeriodicSave;
    QIcon mPlay;
    QIcon mStop;
};

#endif // TIMETABLEWINDOW_H
