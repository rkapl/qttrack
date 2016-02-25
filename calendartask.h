#ifndef CALENDARTASK_H
#define CALENDARTASK_H

#include <QList>
#include <QDateTime>
#include "timespan.h"

class CalendarModel;
class CalendarTimeSpan;

class CalendarTask
{
    friend class CalendarModel;
public:
    CalendarTask();
    CalendarModel* model() const;
    CalendarTask* parent() const;
    QList<CalendarTask*> subtasks() const;
    QString summary() const;

    /**
     * Time spans will be sorted by their start date
     */
    QList<CalendarTimeSpan*> timeSpans(bool recursive = false) const;
    TimeSpan duration(bool recursive) const;
    CalendarTimeSpan* startLogging(const QDateTime& startDate);
    CalendarTimeSpan* stopLogging(const QDateTime &endDate);
    CalendarTimeSpan* addFix(const QDateTime& startDate, const TimeSpan& duration);
    bool isLogging() const;

    ~CalendarTask();
private:
    /**
     * @brief Prepare a new task with default field values.
     * Fills in creation, modification and generates ID.
     */
    void prepareNew();
    void invalidateTimes();

    mutable bool mDurationCacheValid;
    mutable TimeSpan mDuration;
    mutable TimeSpan mDurationRecursive;

    CalendarModel* mModel;
    QDateTime mCreated;
    QDateTime mLastModified;
    QString mId;
    QString mParentId;
    CalendarTask* mParent;
    QString mSummary;
    QList<CalendarTask*> mSubtasks;
    QList<CalendarTimeSpan*> mTimeSpans;
    CalendarTimeSpan* mCurrentlyLogging;
};

#endif // CALENDARTASK_H
