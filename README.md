What's this?
=============

A simple Time Tracker using QT, similar to KTimeTracker for KDE4. It even uses
the same file format so that You can view the old data. Plus it does
not depend on the whole KDE Frameworks thing, so that could be a plus for
somebody.

So far, lot of feature from KTimeTracker are missing, the most important
would be activity detection, idle detection and running in the system tray.

Installing
==========

Debian:

    dpkg-buildpackage
    sudo dpkg -i ../qttrack*.deb

Other:

    qmake .
    make
    sudo make install

