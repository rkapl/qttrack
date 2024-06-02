#ifndef TIMEFORMAT_H
#define TIMEFORMAT_H

#include <QString>

class TimeSpan;

class TimeFormat
{
public:
    enum class Part {
        DAY, HOUR, MINUTE, SECOND
    };

    TimeFormat(const QString& name, Part min, Part max, int decimalPlaces)
        :mName(name), mMin(min), mMax(max), mDecimalPlaces(decimalPlaces) {
        Q_ASSERT(mMin <= mMax);
    }

    QString format(const TimeSpan& time) const;
    const QString& name() const;

    static const TimeFormat defaultFormat;

private:
    /* Format part (day, hour...), having msec_per_unit, substracting the formatted amount from acc.
     *
     * Decimal places are used if this is the last part.
     */
    void formatPart(const char* name, Part part, qint64 msecPerUnit, qint64& acc, bool& first, QStringList& dst) const;

    QString mName;
    Part mMin, mMax;
    int mDecimalPlaces;
};

#endif // TIMEFORMAT_H
