#include "calendarmodel.h"
#include "calendartask.h"
#include "calendartimespan.h"
#include "libicalflusher.h"
#include <libical/ical.h>
#include <QFile>
#include <QTextStream>
#include <memory>
#include <QDebug>
#include <QHash>
#include <QtAlgorithms>
#include <QSaveFile>

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

CalendarModel::CalendarModel(QObject *parent):
    QObject(parent),
    mIcal(NULL)
{
}
QList<CalendarTask*> CalendarModel::rootTasks() const{
    return mRootTasks;
}
CalendarModel::~CalendarModel(){
    qDeleteAll(mAllTasks);
    qDebug() << "Deallocating model";
    icalcomponent_free(mIcal);
}
bool CalendarModel::load(const QString &path){
    mFileName = path;
    QFile f(path);
    if(!f.open(QFile::ReadOnly)){
        emitError(f.errorString());
        return false;
    }

    LibIcalFlusher _flusher;
    QTextStream in(&f);
    std::unique_ptr<icalparser, icalparser_deleter> parser(icalparser_new(), icalparser_free);
    icalparser_set_gen_data(parser.get(), &in);
    while(!in.atEnd()){
        QString line = in.readLine();
        mIcal = icalparser_add_line(parser.get(), line.toUtf8().data());
        if(mIcal != NULL){
            if(icalcomponent_count_errors(mIcal) != 0){
                reportIcalError(mIcal);
                return false;
            }else{
                return handleCalendar(mIcal);
            }
        }
    }

    emitError(tr("File does not contain valid calendar"));
    return false;
}
bool CalendarModel::reportIcalError(icalcomponent *c){
    // try to find ical errors in properties or sub-properties
    for(icalproperty* p = icalcomponent_get_first_property(c,ICAL_XLICERROR_PROPERTY);
        p != NULL;
        p = icalcomponent_get_next_property(c,ICAL_XLICERROR_PROPERTY))
    {
        emitError(icalproperty_get_xlicerror(p));
        return true;
    }

    for(icalcomponent* cc = icalcomponent_get_first_component(c, ICAL_ANY_COMPONENT);
        cc != NULL;
        cc = icalcomponent_get_next_component(cc, ICAL_ANY_COMPONENT)){
        if(reportIcalError(cc))
            return true;
    }

    return false;
}
void CalendarModel::emitError(const QString &problem){
    emit error(tr("iCal Calendar File"), problem);
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

    if(prodid != ICAL_PRODID_QTTRACK){
        emit readingThirdPartyFormat();
    }


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
icaltimetype CalendarModel::qtToIcal(const QDateTime & timeAny){
    QDateTime time = timeAny.toUTC();
    icaltimetype utc;
    memset(&utc, 0, sizeof(utc));
    utc.is_utc = 1;
    utc.zone = icaltimezone_get_utc_timezone();
    utc.day = time.date().day();
    utc.month = time.date().month();
    utc.year = time.date().year();
    utc.hour = time.time().hour();
    utc.minute = time.time().minute();
    utc.second = time.time().second();
    return utc;
}
void CalendarModel::deleteXProperty(icalcomponent *c, const QString& name){
    QList<icalproperty*> deleteList;
    for(icalproperty* p = icalcomponent_get_first_property(c, ICAL_X_PROPERTY);
        p != NULL;
        p = icalcomponent_get_next_property(c, ICAL_X_PROPERTY)){
        if(QString::fromUtf8(icalproperty_get_x(p)) == name){
            deleteList.append(p);
        }
    }
    for(icalproperty* p : deleteList){
        icalcomponent_remove_property(c, p);
        icalproperty_free(p);
    }
}

void CalendarModel::deleteProperty(icalcomponent* c, icalproperty_kind kind){
    QList<icalproperty*> deleteList;
    for(icalproperty* p = icalcomponent_get_first_property(c, kind);
        p != NULL;
        p = icalcomponent_get_next_property(c, kind)){
        deleteList.append(p);
    }
    for(icalproperty* p : deleteList){
        icalcomponent_remove_property(c, p);
        icalproperty_free(p);
    }
}
icalproperty* CalendarModel::updateProperty(icalcomponent *c, icalproperty_kind kind, icalproperty *value){
    deleteProperty(c, kind);
    icalcomponent_add_property(c, value);
    return value;
}
icalproperty* CalendarModel::updateXProperty(icalcomponent *c, const QString &name, const QString &value){
    for(icalproperty* p = icalcomponent_get_first_property(c, ICAL_X_PROPERTY);
        p != NULL;
        p = icalcomponent_get_next_property(c, ICAL_X_PROPERTY)){
        if(QString(icalproperty_get_x_name(p)) == name){
            icalproperty_set_x(p, value.toUtf8());
            return p;
        }
    }
    icalproperty* p = icalproperty_new_x(value.toUtf8());
    icalproperty_set_x_name(p, name.toUtf8());
    icalcomponent_add_property(c, p);
    return p;
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
    task->mBacking = c;
    task->mCreated = icalToQt(icalproperty_get_created(icalcomponent_get_first_property(c, ICAL_CREATED_PROPERTY)));
    task->mLastModified = icalToQt(icalproperty_get_lastmodified(icalcomponent_get_first_property(c, ICAL_LASTMODIFIED_PROPERTY)));
    task->mId = QString::fromUtf8(icalproperty_get_uid(icalcomponent_get_first_property(c, ICAL_UID_PROPERTY)));
    task->mSummary =
            QString::fromUtf8(icalproperty_get_summary(icalcomponent_get_first_property(c, ICAL_SUMMARY_PROPERTY)));
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
void CalendarModel::requestSave(){
    doSave(QDateTime::currentDateTimeUtc());
}
void CalendarModel::doSave(const QDateTime& now){
    LibIcalFlusher flusher;
    CalendarModel::updateProperty(mIcal, ICAL_PRODID_PROPERTY,
                                  icalproperty_new_prodid(ICAL_PRODID_QTTRACK));
    // save only the reachable tasks
    for(auto t: mRootTasks){
        t->save(now, mIcal);
    }

    QSaveFile file(mFileName);
    if(!file.open(QSaveFile::WriteOnly)){
        emitError(file.errorString());
        return;
    }
    const char* data = icalcomponent_as_ical_string_r(mIcal);
    file.write(data);
    if(!file.commit()){
        emitError(file.errorString());
    }
}
void CalendarModel::handleEvent(icalcomponent *c, const TaskMap& tasks){
    QScopedPointer<CalendarTimeSpan> span(new CalendarTimeSpan());

    QList<icalproperty_kind> required{ICAL_DTSTART_PROPERTY, ICAL_DTEND_PROPERTY, ICAL_UID_PROPERTY};
    if(!hasRequiredProperties(required,c ))
        return;

    span->mBacking = c;
    span->mId = QString::fromUtf8(icalproperty_get_uid(icalcomponent_get_first_property(c, ICAL_UID_PROPERTY)));
    span->mStart = icalToQt(icalproperty_get_created(icalcomponent_get_first_property(c, ICAL_DTSTART_PROPERTY)));
    span->mEnd = icalToQt(icalproperty_get_created(icalcomponent_get_first_property(c, ICAL_DTEND_PROPERTY)));

    icalproperty* durationProperty = icalcomponent_get_first_x_property(c, ICAL_XPROP_DURATION);
    if(durationProperty != NULL){
        bool ok;
        qint64 seconds = QString(icalproperty_get_x(durationProperty)).toLongLong(&ok);
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
CalendarTask* CalendarModel::addTask(CalendarTask *parent, const QString &name){
    QScopedPointer<CalendarTask> task(new CalendarTask());
    task->prepareNew();
    task->mSummary = name;
    task->mParent = parent;
    task->mModel = this;

    QList<CalendarTask*>& taskList = (parent == NULL) ? mRootTasks : parent->mSubtasks;
    emit taskAboutToBeAdded(parent, task.data(), taskList.length());
    taskList.append(task.data());
    CalendarTask* taskRaw = task.take();
    mAllTasks.append(taskRaw);
    emit taskAdded(parent, taskRaw, taskList.length() - 1);
    requestSave();
    return taskRaw;
}
void CalendarModel::removeTask(CalendarTask *task){
    QList<CalendarTask*>& taskList = (task->parent() == NULL) ? mRootTasks : task->parent()->mSubtasks;
    int index = taskList.indexOf(task);

    emit taskAboutToBeRemoved(task->parent(), task, index);
    QScopedPointer<CalendarTask> taskDeleter(task);
    mAllTasks.removeOne(task);
    taskList.removeOne(task);
    task->deleteBackings();
    emit taskRemoved(task->parent(), task, index);
    requestSave();
}
void CalendarModel::moveTask(CalendarTask *task, CalendarTask *newParent){
    if(task->parent() == newParent)
        return;
    CalendarTask* oldParent = task->parent();
    QList<CalendarTask*>& oldTaskList = (task->parent() == NULL) ? mRootTasks : task->parent()->mSubtasks;
    QList<CalendarTask*>& newTaskList = (newParent == NULL) ? mRootTasks : newParent->mSubtasks;
    int oldIndex = oldTaskList.indexOf(task);
    emit taskAboutToBeMoved(task, oldParent, newParent, oldIndex, newTaskList.length());
    task->mParent = newParent;
    oldTaskList.removeOne(task);
    newTaskList.append(task);
    emit taskMoved(task, oldParent, newParent, oldIndex, newTaskList.length() - 1);
}
void CalendarModel::informTimesChanged(CalendarTask* task){
    requestSave();
    emit timesChanged(task);
}
