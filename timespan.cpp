#include "timespan.h"
#include <QDateTime>
#include <QStringList>
#include <QDebug>


TimeSpan::TimeSpan():
    msec(0)
{

}
TimeSpan::TimeSpan(qint64 msec):
    msec(msec){

}
#define wpart(name, var) \
    if(var != 0 || noskip){ \
        noskip = true; \
        text.append(QString("%1 " name).arg(var)); \
    }
QString TimeSpan::description() const{
    qint64 seconds = msec / 1000;
    qint64 minutes = seconds / 60;
    seconds -= minutes * 60;
    qint64 hours = minutes / 60;
    minutes -= hours * 60;
    qint64 days = hours / 24;
    hours -= days * 24;

    QStringList text;

    bool noskip = false;
    wpart("d", days);
    wpart("h", hours);
    wpart("m", minutes);
    noskip = true;
    wpart("s", seconds);
    return text.join(" ");
}
TimeSpan operator-(const QDateTime& a, const QDateTime& b){
    return TimeSpan(b.msecsTo(a));
}
TimeSpan& TimeSpan::operator+=(const TimeSpan& add){
    msec += add.msec;
    return *this;
}
TimeSpan TimeSpan::operator+(const TimeSpan& add) const{
    return TimeSpan(msec + add.msec);
}
