#include "calendartimespan.h"

CalendarTimeSpan::CalendarTimeSpan():
    mTask(NULL)
{

}

CalendarTask* CalendarTimeSpan::task() const{
    return mTask;
}
QDateTime CalendarTimeSpan::start() const{
    return mStart;
}
QDateTime CalendarTimeSpan::end() const{
    return mEnd;
}
TimeSpan CalendarTimeSpan::duration() const{
    return end()-start();
}
