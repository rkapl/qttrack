#ifndef CALENDARMODEL_H
#define CALENDARMODEL_H

#include <QList>
#include <libical/ical.h>
#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include <QList>
#include <QTimer>
#include <QDateTime>

class CalendarTask;
class CalendarTimeSpan;

/**
 * @brief Represents the task hierarchy and in which time spans has the user worked on them.
 *
 * The calendar is represented as an iCal file on disk. The file format is compatible with
 * that of KTimeTracker. Each CalendarTask corresponds to a TODO item and each
 * CalendarTimeSpan record is a calendar EVENT.
 *
 * The iCal file is parsed using an iCal library. To minimize file damage (e.g. preserve extra
 * properties not recognized by QTimeTracker), each task and time span keeps reference to its
 * original libical component. The libical components are kept in sync with the memory objects lazily
 * (this is an implementation detail). They are kept in sync by:
 *
 * - updating them before the libical root component is written to file, optionally creating
 *   components for tasks or time spans that were created after the last save operation
 * - removing the backing libical component from the libical root component when task or time
 *   span is deleted
 *
 */
class CalendarModel: public QObject
{
    friend class CalendarTask;
    Q_OBJECT
public:
    static constexpr const char* ICAL_XPROP_DURATION = "X-KDE-ktimetracker-duration";
    static constexpr const char* ICAL_PRODID_QTTRACK = "-//Roman Kapl//QTimeTracker//EN";

    // class proper
    CalendarModel(QObject* parent = NULL);
    /**
     * @brief May not be called multiple times.
     */
    bool load(const QString& path);
    QList<CalendarTask*> rootTasks() const;
    /**
     * @param now Explicit time for testing determinism.
     */
    void doSave(const QDateTime& now);
    CalendarTask* addTask(CalendarTask* parent, const QString& name);
    void moveTask(CalendarTask* task, CalendarTask* newParent);
    void removeTask(CalendarTask* task);

    // helpers
    static void sortSpans(QList<CalendarTimeSpan *> &spans);
    static bool hasRequiredProperties(const QList<icalproperty_kind>& required, icalcomponent *c);
    static void deleteProperty(icalcomponent* c, icalproperty_kind kind);
    static void deleteXProperty(icalcomponent* c, const QString &name);
    static icalproperty* updateProperty(icalcomponent* c, icalproperty_kind kind, icalproperty* value);
    static icalproperty* updateXProperty(icalcomponent* c, const QString& name, const QString& value);

    static icaltimetype qtToIcal(const QDateTime& );
    static QDateTime icalToQt(const icaltimetype &timw);

    ~CalendarModel();
public slots:
    /**
     * @brief Request save may delay and coalesce save.
     * Currently it does not do it and just calls doSave. Also it supplies the current
     * system date as current date.
     */
    void requestSave();
signals:
    void error(const QString& source, const QString& description);
    void readingThirdPartyFormat();
    /**
     * @brief Time accounting has changed for a given task and consequently for its parents.
     */
    void timesChanged(CalendarTask* task);
    /**
     * @brief Task details (such as summary) have changed
     */
    void taskChanged(CalendarTask* task);
    void taskAboutToBeAdded(CalendarTask* parent, CalendarTask* task, int position);
    void taskAdded(CalendarTask* parent, CalendarTask* task, int position);
    void taskAboutToBeRemoved(CalendarTask *parent, CalendarTask* task, int position);
    void taskRemoved(CalendarTask *parent, CalendarTask* task, int position);
    void taskAboutToBeMoved(CalendarTask* task, CalendarTask* oldParent, CalendarTask* newParent,
                            int oldPosition, int newPosition);
    void taskMoved(CalendarTask* task, CalendarTask* oldParent, CalendarTask* newParent,
                   int oldPosition, int newPosition);
private:
    using TaskMap = QHash<QString, CalendarTask*>;

    QString mFileName;
    icalcomponent* mIcal;
    QList<CalendarTask*> mRootTasks;
    QList<CalendarTask*> mAllTasks;
    /**
     * @brief mUnassigned is used to track all CalendarTimeSpan for which
     * no suitable parent was found. It is not added as a visible task,
     * unless there is some CalendarTimeSpan wihtout parent. In that case,
     * mUnassignedAdded is true.
     */
    bool mUnassignedAdded;
    CalendarTask* mUnassigned;

    void emitError(const QString& problem);

    bool reportIcalError(icalcomponent* c);
    bool handleCalendar(icalcomponent* c);
    void handleEvent(icalcomponent* c, const TaskMap& tasks);
    void handleTodo(icalcomponent* c, TaskMap& tasks);
    void makeTaskTree(const TaskMap& tasks);
    void informTimesChanged(CalendarTask *task);
    void informTaskChanged(CalendarTask *task);
};




#endif // CALENDARMODEL_H
