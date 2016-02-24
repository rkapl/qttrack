#ifndef TIMETABLEWINDOW_H
#define TIMETABLEWINDOW_H

#include <QMainWindow>


namespace Ui {
class TimeTableWindow;
}

class TreeCalendarModel;
class CalendarModel;

class TimeTableWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TimeTableWindow(QWidget *parent = 0);
    ~TimeTableWindow();
public slots:
    void openFileWithDialog();
    void openFile(const QString& fileName);
private:
    Ui::TimeTableWindow *ui;
    CalendarModel* mModel;
    TreeCalendarModel* mTreeModel;
};

#endif // TIMETABLEWINDOW_H
