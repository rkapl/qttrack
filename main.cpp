#include "timetablewindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("q-time-tracker");
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser cmdline;
    cmdline.addHelpOption();
    cmdline.addPositionalArgument("calendar-file", "Calendar file to open (in KTimeTracker format)", "[calendar-file]");
    cmdline.process(a);

    TimeTableWindow w;
    w.show();

    if(cmdline.positionalArguments().length() == 1){
        w.openFile(cmdline.positionalArguments()[0]);
    }else if(cmdline.positionalArguments().length() > 1){
        fprintf(stderr, "Error: More than two arguments given.");
        cmdline.showHelp(1);
    }

    return a.exec();
}
