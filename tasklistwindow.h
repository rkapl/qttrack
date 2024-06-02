#ifndef TIMETABLEWINDOW_H
#define TIMETABLEWINDOW_H

#include <QMainWindow>
#include <QItemSelection>
#include <QTimer>
#include <QMenu>
#include <QWidgetAction>
#include <QSettings>
#include <QComboBox>
#include "fixtimewidget.h"
#include "timeformat.h"

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
    void openFile(const QString& fileName, bool showInCaption = false);
    void createFile(const QString& fileName, bool showInCaption = false);
    void toggleTask();
    void clearModel();
    void showError(const QString& from, const QString& text);
    void addNewTask();
    void removeSelectedTask();
    void openDefault();
    void checkForImport();
private slots:
    void taskSelectionChanged(const QItemSelection&, const QItemSelection& );
    void timeDetailsForCurrentSelection();
    void timeDetailsForSelection(const QModelIndex& selection);
    void timeDetailsFor(CalendarTask* task);
    void fixTimeFor(CalendarTask* task);
    void modelExistenceChanged();
    void timeFormatSelected(int index);
private:
    static constexpr int LOGGING_SAVE_FREQUENCY = 60*1000;
    static constexpr qint64 DELETE_WARNING_TRESHOLD_MSEC = 5 * 60 * 1000;

    /**
     * @brief Updates the green bar at bottom of the window.
     */
    void updateActiveTaskUi();
    void updateActiveTaskTicker();
    void updateCaption(const QString& fileName, bool showInCaption);
    void clearCaption();
    void setupTimeFormats();
    void doImport();
    void connectNewModel();
    bool createDataDir();
    CalendarTask* selectedTask() const;

    Ui::TimeTableWindow *ui;
    CalendarModel* mModel;
    TreeCalendarModel* mTreeModel;

    QMenu mFixTimeMenu;
    FixTimeWidget mFixTimeMenuWidget;
    QComboBox* mTimeFormatSelector;
    std::vector<TimeFormat> mAvailableTimeFormats;
    TimeFormat* mSelectedTimeFormat;

    CalendarTask* mActiveTask;
    CalendarTimeSpan* mActiveTimeSpan;
    QTimer mActiveTaskTimeUpdater;
    /**
     * @brief Active during logging.
     */
    QTimer mPeriodicSave;
    QIcon mPlay;
    QIcon mStop;
    QSettings mSettings;

    QString mCaptionBase;
};

#endif // TIMETABLEWINDOW_H
