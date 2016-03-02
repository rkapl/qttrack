#include "fixtimewidget.h"
#include <QStyle>
#include <QRegularExpressionValidator>
#include "timespan.h"
#include "calendartask.h"
#include <QDoubleValidator>

FixTimeWidget::FixTimeWidget():
    mTarget(NULL),
    mLayout(this),
    mEntry(this),
    mApply(this)
{
    connect(&mEntry, &QLineEdit::textChanged, this, &FixTimeWidget::textChanged);
    connect(&mEntry, &QLineEdit::returnPressed, this, &FixTimeWidget::applyFix);
    connect(&mApply, &QPushButton::pressed, this, &FixTimeWidget::applyFix);

    mLayout.addWidget(&mEntry);
    mLayout.addWidget(&mApply);
    mApply.setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    mLayout.setContentsMargins(4,4,4,4);
}
void FixTimeWidget::textChanged(const QString &str){
    auto match = TimeSpan::timeSpanRegex().match(str, 0, QRegularExpression::PartialPreferCompleteMatch);
    mApply.setEnabled(match.hasMatch());
    if(!match.hasPartialMatch() && !match.hasMatch() && !str.isEmpty()){
        mApply.setStyleSheet("background-color: red;");
    }else{
        mApply.setStyleSheet("");
    }
}
void FixTimeWidget::setTarget(CalendarTask *ctask){
    mTarget = ctask;
    mEntry.setText("+ 0s");
}
CalendarTask* FixTimeWidget::target() const{
    return mTarget;
}
void FixTimeWidget::applyFix(){
    if(applyFixInternal()){
        mTarget = NULL;
        emit done();
    }
}
bool FixTimeWidget::applyFixInternal(){
    if(target() == NULL)
        return false;

    bool ok;
    auto span = TimeSpan::parse(mEntry.text(), &ok);
    if(!ok)
        return false;

    mTarget->addFix(QDateTime::currentDateTimeUtc(), span);


    return true;
}
void FixTimeWidget::applyFixKeepTarget(){
    if(applyFixInternal())
        emit done();
}
