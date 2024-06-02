#ifndef TIMESPAN_H
#define TIMESPAN_H

#include <QObject>
#include <QRegularExpression>

class QDateTime;

class TimeSpan
{
public:
    TimeSpan();
    explicit TimeSpan(qint64 msec);
    TimeSpan& operator+=(const TimeSpan& add);
    TimeSpan operator+(const TimeSpan& b) const;

    static TimeSpan fromDiff(const QDateTime& a, const QDateTime& b);

    static TimeSpan parse(const QString& str, bool* ok);
    static const QRegularExpression &timeSpanRegex();
    static constexpr int MAX_DETAIL = 4;

    qint64 msec;

private:
    static const QRegularExpression mRegExp;
};

#endif // TIMESPAN_H
