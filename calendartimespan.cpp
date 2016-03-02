#include "calendartimespan.h"
#include "calendarmodel.h"
#include "calendartask.h"
#include <QUuid>
#include <QDebug>

CalendarTimeSpan::CalendarTimeSpan():
    mIsFix(false),
    mTask(NULL),
    mBacking(NULL)
{

}
CalendarTimeSpan::~CalendarTimeSpan(){
    task()->mTimeSpans.removeOne(this);
    if(mBacking){
        icalcomponent_free(mBacking);
    }
}
void CalendarTimeSpan::prepreNew(){
    mId = QUuid::createUuid().toString();
}
void CalendarTimeSpan::save(icalcomponent *root){
    if(mBacking == NULL){
        mBacking = icalcomponent_new(ICAL_VEVENT_COMPONENT);
        icalcomponent_add_component(root, mBacking);
    }
    CalendarModel::updateProperty(mBacking, ICAL_UID_PROPERTY,
                                  icalproperty_new_uid(mId.toUtf8().data()));
    CalendarModel::updateProperty(mBacking, ICAL_SUMMARY_PROPERTY,
                                  icalproperty_new_summary(task()->summary().toUtf8().data()));
    CalendarModel::updateProperty(mBacking, ICAL_DTSTART_PROPERTY,
                                  icalproperty_new_dtstart(CalendarModel::qtToIcal(mStart)));
    CalendarModel::updateProperty(mBacking, ICAL_RELATEDTO_PROPERTY,
                                  icalproperty_new_relatedto(task()->mId.toUtf8().data()));

    if(isFix()){
        QDateTime end = (mFixDuration.msec > 0) ? (start().addMSecs(mFixDuration.msec)) : start().addSecs(1);
        CalendarModel::updateProperty(mBacking, ICAL_DTEND_PROPERTY,
                                      icalproperty_new_dtend(CalendarModel::qtToIcal(end)));
        CalendarModel::updateXProperty(mBacking, CalendarModel::ICAL_XPROP_DURATION,
                                       QString::number(mFixDuration.msec/1000).toUtf8().data());
    }else{
        CalendarModel::deleteXProperty(mBacking, CalendarModel::ICAL_XPROP_DURATION);
        CalendarModel::updateProperty(mBacking, ICAL_DTEND_PROPERTY,
                                      icalproperty_new_dtend(CalendarModel::qtToIcal(mEnd)));
    }
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
