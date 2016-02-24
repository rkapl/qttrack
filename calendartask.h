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
    /**
     * Time spans will be sorted by their start date
     */
    QList<CalendarTimeSpan*> timeSpans() const;
    QString summary() const;
    TimeSpan duration(bool recursive) const;
    ~CalendarTask();
private:
    /**
     * @brief Prepare a new task with default field values.
     * Fills in creation, modification and generates ID.
     */
    void prepareNew();

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
};

#endif // CALENDARTASK_H
