#ifndef TIMELISTDIALOG_H
#define TIMELISTDIALOG_H

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class TimeListDialog;
}

class TimeSpan;
class CalendarTask;
class CalendarTimeSpan;
class TimeListTimeWalker;

class TimeListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeListDialog(QWidget *parent = 0);
    void setContent(CalendarTask* task);
    ~TimeListDialog();

private:
    /**
     * @brief If the start day in `walker` is different than in `span`, create a new
     * widget item for the new day (and possible also year and month).
     */
    void updateWalker(TimeListTimeWalker& walker, CalendarTimeSpan* span);
    void updateItemWithDuration(QTreeWidgetItem* item, const TimeSpan& duration);
    Ui::TimeListDialog *ui;
};

#endif // TIMELISTDIALOG_H
