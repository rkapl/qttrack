#include "calendarmodel.h"
#include "calendartask.h"
#include "calendartimespan.h"
#include <libical/ical.h>
#include <QFile>
#include <QTextStream>
#include <memory>
#include <QDebug>
#include <QHash>
#include <QtAlgorithms>

typedef void (*icalparser_deleter)(icalparser*);
typedef void (*icalcomponent_deleter)(icalcomponent*);

static icalproperty* icalcomponent_get_first_x_property(icalcomponent* c, const char* name){
    icalproperty* p = icalcomponent_get_first_property(c, ICAL_X_PROPERTY);
    while(p != NULL){
        if(strcmp(name, icalproperty_get_x_name(p)) == 0)
            return p;
    }
    return NULL;
}

CalendarModel::CalendarModel(QObject *parent): QObject(parent)
{

}
QList<CalendarTask*> CalendarModel::rootTasks() const{
    return mRootTasks;
}
CalendarModel::~CalendarModel(){
    qDeleteAll(mAllTasks);
}
bool CalendarModel::load(const QString &path){
    QFile f(path);
    if(!f.open(QFile::ReadOnly))
        return false;
    QTextStream in(&f);


    std::unique_ptr<icalparser, icalparser_deleter> parser(icalparser_new(), icalparser_free);
    icalparser_set_gen_data(parser.get(), &in);
    while(!in.atEnd()){
        QString line = in.readLine();
        std::unique_ptr<icalcomponent, icalcomponent_deleter>
                c(icalparser_add_line(parser.get(), line.toUtf8().data()), icalcomponent_free);
        if(c != NULL){
            return handleCalendar(c.get());
        }
    }
    return true;
}
/**
 * @brief Handles one iCal calendar and stores all its TODOs and EVENTs into the internal structures
 */
bool CalendarModel::handleCalendar(icalcomponent *c){
    QString prodid = "unknown";
    icalproperty* prodidProperty = icalcomponent_get_first_property(c, ICAL_PRODID_PROPERTY);
    if(prodidProperty != NULL){
        prodid = icalproperty_get_prodid(prodidProperty);
    }
    qDebug() << "Reading calendar produced by: " << prodid;

    icalcomponent* sub = icalcomponent_get_first_component(c, ICAL_VTODO_COMPONENT);
    TaskMap tasks;
    while(sub != NULL){
        handleTodo(sub, tasks);
        sub = icalcomponent_get_next_component(c, ICAL_VTODO_COMPONENT);
    }

    makeTaskTree(tasks);

    mUnassigned = new CalendarTask();
    mUnassigned->prepareNew();
    mUnassigned->mSummary = "Unassigned work";
    mUnassignedAdded = false;
    mAllTasks.append(mUnassigned);

    sub = icalcomponent_get_first_component(c, ICAL_VEVENT_COMPONENT);
    while(sub != NULL){
        handleEvent(sub, tasks);
        sub = icalcomponent_get_next_component(c, ICAL_VEVENT_COMPONENT);
    }

    for(CalendarTask* t: mAllTasks){
        sortSpans(t->mTimeSpans);
    }

    return true;
}
void CalendarModel::makeTaskTree(const TaskMap& tasks){

    // go throught all tasks and append them to parent tasks
    for(auto task: mAllTasks){
        if(task->mParentId.isEmpty()){
            mRootTasks.append(task);
        }else if(!tasks.contains(task->mParentId)){
            qWarning() << "Task " << task->mId << " has parent "<< task->mParentId << " but it does not exist";
            mRootTasks.append(task);
        }else{
            task->mParent = tasks[task->mParentId];
            task->mParent->mSubtasks.append(task);
        }
    }
}
bool CalendarModel::hasRequiredProperties(const QList<icalproperty_kind>& required, icalcomponent* c){
    for(icalproperty_kind k: required){
        icalproperty* p = icalcomponent_get_first_property(c, k);
        if(p == NULL){
            qWarning() << "Component " << icalcomponent_get_uid(c) << " is missing property " << icalproperty_kind_to_string(k);
            return false;
        }
    }
    return true;
}
QDateTime CalendarModel::icalToQt(const icaltimetype& time){
    icaltimetype utc = icaltime_convert_to_zone(time, icaltimezone_get_utc_timezone());
    QDate d(utc.year, utc.month, utc.day);
    QTime t(utc.hour, utc.minute, utc.second);
    QDateTime date(d, t, Qt::UTC);
    return date.toLocalTime();
}
void CalendarModel::sortSpans(QList<CalendarTimeSpan*>& spans){
    qSort(spans.begin(), spans.end(), [](CalendarTimeSpan* a, CalendarTimeSpan* b){
       return a->start() < b->start();
    });
}

void CalendarModel::handleTodo(icalcomponent *c, TaskMap& tasks){
    QScopedPointer<CalendarTask> task(new CalendarTask());

    QList<icalproperty_kind> required{ICAL_CREATED_PROPERTY, ICAL_LASTMODIFIED_PROPERTY, ICAL_UID_PROPERTY,
                                     ICAL_SUMMARY_PROPERTY};
    if(!hasRequiredProperties(required,c ))
        return;

    // TODO: make created and last modified optional ?
    task->mCreated = icalToQt(icalproperty_get_created(icalcomponent_get_first_property(c, ICAL_CREATED_PROPERTY)));
    task->mLastModified = icalToQt(icalproperty_get_lastmodified(icalcomponent_get_first_property(c, ICAL_LASTMODIFIED_PROPERTY)));
    task->mId = icalproperty_get_uid(icalcomponent_get_first_property(c, ICAL_UID_PROPERTY));
    task->mSummary = icalproperty_get_summary(icalcomponent_get_first_property(c, ICAL_SUMMARY_PROPERTY));
    task->mParent = NULL;
    task->mModel = this;

    icalproperty* relatedProperty = icalcomponent_get_first_property(c, ICAL_RELATEDTO_PROPERTY);
    if(relatedProperty != NULL){
        task->mParentId = icalproperty_get_relatedto(relatedProperty);
    }

    if(tasks.contains(task->mId))
        qWarning() << "Duplicate task uid " << task->mId;
    tasks[task->mId] = task.data();
    mAllTasks.append(task.take());

}
void CalendarModel::handleEvent(icalcomponent *c, const TaskMap& tasks){
    QScopedPointer<CalendarTimeSpan> span(new CalendarTimeSpan());

    QList<icalproperty_kind> required{ICAL_DTSTART_PROPERTY, ICAL_DTEND_PROPERTY};
    if(!hasRequiredProperties(required,c ))
        return;

    span->mStart = icalToQt(icalproperty_get_created(icalcomponent_get_first_property(c, ICAL_DTSTART_PROPERTY)));
    span->mEnd = icalToQt(icalproperty_get_created(icalcomponent_get_first_property(c, ICAL_DTEND_PROPERTY)));

    icalproperty* durationProperty = icalcomponent_get_first_x_property(c, ICAL_XPROP_DURATION);
    if(durationProperty != NULL){
        bool ok;
        int seconds = QString(icalproperty_get_x(durationProperty)).toInt(&ok);
        if(ok){
            span->mIsFix = true;
            span->mFixDuration = TimeSpan(seconds*1000);
        }else{
            qWarning() << "Can not parse" << ICAL_XPROP_DURATION << " = " << icalproperty_get_x(durationProperty);
        }
    }

    icalproperty* relatedProperty = icalcomponent_get_first_property(c, ICAL_RELATEDTO_PROPERTY);
    if(relatedProperty != NULL){
        QString parentId = icalproperty_get_relatedto(relatedProperty);
        if(tasks.contains(parentId)){
            span->mTask = tasks[parentId];
        }
    }

    if(span->mTask == NULL){
        const char* uid = icalcomponent_get_uid(c);
        qWarning() << "Event (CalendarTimeSpan) " << (uid ? uid : "#unknown") << " can not be paired to any parent";
        span->mTask = mUnassigned;
        if(!mUnassignedAdded){
            mRootTasks.append(mUnassigned);
            mUnassignedAdded = true;
        }
    }
    CalendarTask* task = span->task();
    // qDebug() << "Event (part of " << task->summary() << ") takes " << span->duration().description()
    //          << "(" << span->duration().msec << " ms)";
    task->mTimeSpans.append(span.take());

}

