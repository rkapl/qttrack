#include "calendartimespan.h"

CalendarTimeSpan::CalendarTimeSpan():
    mIsFix(false),
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
bool CalendarTimeSpan::isFix() const{
    return mIsFix;
}
TimeSpan CalendarTimeSpan::duration() const{
    if(mIsFix){
        return mFixDuration;
    }else{
        return end()-start();
    }
}
