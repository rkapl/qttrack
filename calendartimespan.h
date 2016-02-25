#ifndef CALENDARTIMESPAN_H
#define CALENDARTIMESPAN_H


#include <QDateTime>
#include "timespan.h"

class CalendarTask;
class CalendarModel;

class CalendarTimeSpan
{
    friend class CalendarModel;
public:
    CalendarTimeSpan();
    CalendarTask* task() const;
    QDateTime start() const;
    QDateTime end() const;
    TimeSpan duration() const;
    /**
     * @brief Fix CalendarTimeSpan is a special record marking that user has modified task's duration
     *
     * The user has the option to explicitely modify (fix) the task's duration, by adding
     * or substracting a certain amount of time from it. This is done by appending a special
     * CalendarTimeSpan, possibly with negative duration.
     * @return true if this is a TimeSpan entered by user for modifying the tasks' duration
     */
    bool isFix() const;
private:
    bool mIsFix;
    TimeSpan mFixDuration;
    QDateTime mStart;
    QDateTime mEnd;
    CalendarTask* mTask;
};

#endif // CALENDARTIMESPAN_H
