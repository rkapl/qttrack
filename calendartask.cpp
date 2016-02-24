#include "calendartask.h"
#include "calendartimespan.h"
#include <QUuid>

CalendarTask::CalendarTask():
    mDurationCacheValid(false),
    mModel(NULL),
    mParent(NULL)
{

}
void CalendarTask::prepareNew(){
    mCreated = QDateTime::currentDateTimeUtc();
    mLastModified = QDateTime::currentDateTimeUtc(),
    mId = QUuid::createUuid().toString();
}

CalendarModel* CalendarTask::model() const{
    return mModel;
}
CalendarTask* CalendarTask::parent() const{
    return mParent;
}
QList<CalendarTask*> CalendarTask::subtasks() const{
    return mSubtasks;
}
QString CalendarTask::summary() const{
    return mSummary;
}
QList<CalendarTimeSpan*> CalendarTask::timeSpans() const{
    return mTimeSpans;
}
TimeSpan CalendarTask::duration(bool recursive) const{
    if(!mDurationCacheValid){
        mDuration = TimeSpan();
        for(CalendarTimeSpan* span: timeSpans()){
            mDuration += span->duration();
        }

        mDurationRecursive = mDuration;
        for(CalendarTask* task: subtasks()){
            mDurationRecursive+= task->duration(true);
        }
        mDurationCacheValid = true;
    }
    return recursive ? mDurationRecursive : mDuration;
}
CalendarTask::~CalendarTask(){
    qDeleteAll(mTimeSpans);
}
