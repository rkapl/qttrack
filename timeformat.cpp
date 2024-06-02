#include "timeformat.h"
#include "timespan.h"

const TimeFormat TimeFormat::defaultFormat("Hours, minutes", Part::HOUR, Part::MINUTE, 0);

const QString& TimeFormat::name() const {
    return mName;
}

void TimeFormat::formatPart(
    const char* name, Part part, qint64 msecPerUnit, qint64& acc, bool& first, QStringList &dst) const
{
    if (part > mMax || part < mMin) {
        return;
    } else if (mMax == part) {
        auto amount_str = QString::number(acc / static_cast<double>(msecPerUnit), 'f', mDecimalPlaces);
        dst.append(QString("%1 %2").arg(amount_str).arg(name));
    } else {
        qint64 amount = acc / msecPerUnit;
        acc %= msecPerUnit;
        if (acc || !first) {
            dst.append(QString("%1 %2").arg(QString::number(amount)).arg(name));
            first = false;
        }
    }
}

QString TimeFormat::format(const TimeSpan& time) const {

    /**
     * Writes a part of TimeSpan description (such as minutes, hours ...)
     * into a string if it is appropriate: leading zero parts and parts
     * exceeding the wanted detail are emitted.
     */

    qint64 msec = time.msec;
    QStringList text;
    if(msec < 0){
        msec = -msec;
        text.append("-");
    }
    bool first = true;

    formatPart("d", Part::DAY, 1000*60*60*24, msec, first, text);
    formatPart("h", Part::HOUR, 1000*60*60, msec, first, text);
    formatPart("m", Part::MINUTE, 1000*60, msec, first, text);
    formatPart("s", Part::SECOND, 1000, msec, first, text);

    return text.join(" ");

}
