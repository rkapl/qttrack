#include "calendartask.h"
#include "calendartimespan.h"
#include "calendarmodel.h"
#include <QtAlgorithms>
#include <QUuid>
#include <QDebug>

CalendarTask::CalendarTask():
    mDurationCacheValid(false),
    mModel(NULL),
    mParent(NULL),
    mCurrentlyLogging(NULL),
    mBacking(NULL)
{

}
void CalendarTask::prepareNew(){
    mCreated = QDateTime::currentDateTimeUtc();
    mLastModified = QDateTime::currentDateTimeUtc(),
    mId = QUuid::createUuid().toString();
}
void CalendarTask::save(const QDateTime& now, icalcomponent *root){
    for(auto task: subtasks())
        task->save(now, root);

    for(auto span: mTimeSpans)
        span->save(root);

    if(mCurrentlyLogging){
        mCurrentlyLogging->mEnd = now;
        mCurrentlyLogging->save(root);
    }

    if(mBacking == NULL){
        mBacking = icalcomponent_new_vtodo();
        icalcomponent_add_component(root, mBacking);
    }
    CalendarModel::updateProperty(mBacking, ICAL_CREATED_PROPERTY,
                                  icalproperty_new_created(CalendarModel::qtToIcal(mCreated)));
    CalendarModel::updateProperty(mBacking, ICAL_LASTMODIFIED_PROPERTY,
                                  icalproperty_new_lastmodified(CalendarModel::qtToIcal(mLastModified)));
    CalendarModel::updateProperty(mBacking, ICAL_UID_PROPERTY,
                                  icalproperty_new_uid(mId.toUtf8().data()));
    if(parent()){
        CalendarModel::updateProperty(mBacking, ICAL_RELATEDTO_PROPERTY,
                                      icalproperty_new_relatedto(parent()->mId.toUtf8().data()));
    }else{
        CalendarModel::deleteProperty(mBacking, ICAL_RELATEDTO_PROPERTY);
    }
    CalendarModel::updateProperty(mBacking, ICAL_SUMMARY_PROPERTY,
                                  icalproperty_new_summary(summary().toUtf8().data()));
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
        CalendarModel::sortSpans(spans);
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
    mCurrentlyLogging->prepreNew();
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
    Q_ASSERT(!isLogging());
    CalendarTimeSpan* span = new CalendarTimeSpan();
    span->prepreNew();
    span->mStart = startDate;
    span->mIsFix = true;
    span->mFixDuration = duration;
    span->mTask = this;
    mTimeSpans.append(span);
    invalidateTimes();
    return span;
}
void CalendarTask::invalidateTimes(){
    CalendarTask* current = this;
    while(current){
        current->mDurationCacheValid = false;
        current = current->parent();
    }
    model()->informTimesChanged(this);
}
bool CalendarTask::isLogging() const{
    return mCurrentlyLogging != NULL;
}
CalendarTask::~CalendarTask(){
    qDeleteAll(mTimeSpans);
    delete mCurrentlyLogging;
}
