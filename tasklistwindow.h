#ifndef TIMETABLEWINDOW_H
#define TIMETABLEWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QTimer>

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
private slots:
    void taskSelectionChanged(const QItemSelection&, const QItemSelection& );
    void timeDetailsForCurrentSelection();
    void timeDetailsForSelection(const QModelIndex& selection);
    void timeDetailsFor(CalendarTask* task);
private:
    /**
     * @brief Updates the green bar at bottom of the window.
     */
    void updateActiveTaskUi();
    void updateActiveTaskTicker();
    CalendarTask* selectedTask() const;

    Ui::TimeTableWindow *ui;
    CalendarModel* mModel;
    TreeCalendarModel* mTreeModel;

    CalendarTask* mActiveTask;
    CalendarTimeSpan* mActiveTimeSpan;
    QTimer mActiveTaskTimeUpdater;
};

#endif // TIMETABLEWINDOW_H
