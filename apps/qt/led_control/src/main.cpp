/* Copyright 2019 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowState(Qt::WindowFullScreen);
    w.show();

    // This will not return until main window close.
    return a.exec();
}
