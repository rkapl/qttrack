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
private:
    QDateTime mStart;
    QDateTime mEnd;
    CalendarTask* mTask;
};

#endif // CALENDARTIMESPAN_H
