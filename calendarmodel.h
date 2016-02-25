#ifndef CALENDARMODEL_H
#define CALENDARMODEL_H

#include <QList>
#include <libical/ical.h>
#include <QDateTime>
#include <QObject>
#include <QScopedPointer>

class CalendarTask;
class CalendarTimeSpan;

class CalendarModel: public QObject
{
    Q_OBJECT
public:
    CalendarModel(QObject* parent = NULL);
    /**
     * @brief May not be called multiple times.
     */
    bool load(const QString& path);
    QList<CalendarTask*> rootTasks() const;
    static void sortSpans(QList<CalendarTimeSpan *> &spans);
    ~CalendarModel();
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
private:
    static constexpr const char* ICAL_XPROP_DURATION = "X-KDE-ktimetracker-duration";
    static constexpr const char* ICAL_PRODID_QTTRACK = "-//Roman Kapl//QTimeTracker//EN";
    typedef QHash<QString, CalendarTask*> TaskMap;

    void emitError(const QString& problem);

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

    static bool hasRequiredProperties(const QList<icalproperty_kind>& required, icalcomponent *c);
    static QDateTime icalToQt(const icaltimetype &timw);

    bool reportIcalError(icalcomponent* c);
    bool handleCalendar(icalcomponent* c);
    void handleEvent(icalcomponent* c, const TaskMap& tasks);
    void handleTodo(icalcomponent* c, TaskMap& tasks);
    void makeTaskTree(const TaskMap& tasks);
};

#endif // CALENDARMODEL_H
