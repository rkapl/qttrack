#include "calendartask.h"
#include "calendartimespan.h"
#include <QUuid>
#include <QDebug>

CalendarTask::CalendarTask():
    mDurationCacheValid(false),
    mModel(NULL),
    mParent(NULL),
    mCurrentlyLogging(false)
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
QList<CalendarTimeSpan*> CalendarTask::timeSpans(bool recursive) const{
    if(recursive){
        QList<CalendarTimeSpan*> spans(timeSpans());
        for(CalendarTask* subtask: subtasks()){
            spans += subtask->timeSpans(true);
        }
        return spans;
    }else{
        return mTimeSpans;
    }
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
CalendarTimeSpan* CalendarTask::startLogging(const QDateTime& startDate){
    Q_ASSERT(!isLogging());
    mCurrentlyLogging = new CalendarTimeSpan();
    mCurrentlyLogging->mTask = this;
    mCurrentlyLogging->mStart = startDate;
    return mCurrentlyLogging;
}
CalendarTimeSpan* CalendarTask::stopLogging(const QDateTime& endDate){
    Q_ASSERT(isLogging());
    mCurrentlyLogging->mEnd = endDate;
    mTimeSpans.append(mCurrentlyLogging);
    mCurrentlyLogging = NULL;
    invalidateTimes();
    return mTimeSpans.back();
}
CalendarTimeSpan* CalendarTask::addFix(const QDateTime &startDate, const TimeSpan& duration){
    CalendarTimeSpan* span = new CalendarTimeSpan();
    span->mStart = startDate;
    span->mIsFix = true;
    span->mFixDuration = duration;
    span->mTask = this;
    mTimeSpans.append(span);
    invalidateTimes();
    return span;
}
void CalendarTask::invalidateTimes(){
    mDurationCacheValid = false;
    if(parent() != NULL)
        parent()->invalidateTimes();
}
bool CalendarTask::isLogging() const{
    return mCurrentlyLogging != NULL;
}
CalendarTask::~CalendarTask(){
    qDeleteAll(mTimeSpans);
    delete mCurrentlyLogging;
}
