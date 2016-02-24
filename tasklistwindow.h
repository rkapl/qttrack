#ifndef TIMETABLEWINDOW_H
#define TIMETABLEWINDOW_H

#include <QMainWindow>
#include <QItemSelection>

namespace Ui {
class TimeTableWindow;
}

class TreeCalendarModel;
class CalendarModel;
class CalendarTask;

class TaskListWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TaskListWindow(QWidget *parent = 0);
    ~TaskListWindow();
public slots:
    void openFileWithDialog();
    void openFile(const QString& fileName);
private slots:
    void taskSelectionChanged(const QItemSelection& selected, const QItemSelection& );
    void timeDetailsForSelection();
    void timeDetailsFor(const QModelIndex& selection);
private:
    Ui::TimeTableWindow *ui;
    CalendarModel* mModel;
    TreeCalendarModel* mTreeModel;
};

#endif // TIMETABLEWINDOW_H
