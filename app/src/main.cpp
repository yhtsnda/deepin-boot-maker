/*
main.cpp from UNetbootin <http://unetbootin.sourceforge.net>
Copyright (C) 2007-2008 Geza Kovacs <geza0kovacs@gmail.com>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License at <http://www.gnu.org/licenses/> for more details.
*/
#include "bootmaker.h"
#include "ui/dwindowui.h"

#include <QtGui>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QtGlobal>
#include <QtWidgets/QApplication>
#include <QMessageBox>

#ifdef Q_OS_UNIX
QString checkforgraphicalsu(QString graphicalsu) {
    QProcess whereiscommand;
    whereiscommand.start(QString("which %1").arg(graphicalsu));
    whereiscommand.waitForFinished(-1);
    QString commandbinpath = QString(whereiscommand.readAll()).trimmed();

    if(!commandbinpath.isEmpty() && QFile::exists(commandbinpath))
        return commandbinpath;
    else
        return "REQCNOTFOUND";
}

bool SwitchToRoot(QApplication &app) {
    QStringList allappargs = app.arguments();
    QProcess whoamip;
    whoamip.start("whoami");
    whoamip.waitForFinished();

    if(QString(whoamip.readAll()).remove("\r").remove("\n") != "root") {
        QString argsconc = "";
        QString argsconcSingleQuote = "";

        for(int i = 1; i < allappargs.size(); ++i) {
            argsconc += QString("\"%1\" ").arg(allappargs.at(i));
            argsconcSingleQuote += QString("'%1' ").arg(allappargs.at(i));
        }

        argsconc += "\"rootcheck=no\"";
        argsconcSingleQuote += "'rootcheck=no'";
#ifdef Q_OS_LINUX
        QString gksulocation = checkforgraphicalsu("gksu");

        if(gksulocation != "REQCNOTFOUND") {
            QProcess::startDetached(QString("%1 %2 %3").arg(gksulocation).arg(app.applicationFilePath()).arg(argsconc));
            return true;
        }

        QString kdesulocation = checkforgraphicalsu("kdesu");

        if(kdesulocation != "REQCNOTFOUND") {
            QProcess::startDetached(QString("%1 %2 %3").arg(kdesulocation).arg(app.applicationFilePath()).arg(argsconc));
            return true;
        }

        QString gnomesulocation = checkforgraphicalsu("gnomesu");

        if(gnomesulocation != "REQCNOTFOUND") {
            QProcess::startDetached(QString("%1 %2 %3").arg(gnomesulocation).arg(app.applicationFilePath()).arg(argsconc));
            return true;
        }

        QString kdesudolocation = checkforgraphicalsu("kdesudo");

        if(kdesudolocation != "REQCNOTFOUND") {
            QProcess::startDetached(QString("%1 %2 %3").arg(kdesudolocation).arg(app.applicationFilePath()).arg(argsconc));
            return true;
        }

        QMessageBox rootmsgb;
        rootmsgb.setIcon(QMessageBox::Warning);
        rootmsgb.setWindowTitle(QObject::tr("Must run as root"));
        rootmsgb.setTextFormat(Qt::RichText);
        rootmsgb.setText(QObject::tr("%2 must be run as root. Close it, and re-run using either:<br/><b>sudo %1</b><br/>").arg(app.applicationFilePath()));
        rootmsgb.setStandardButtons(QMessageBox::Ok);

        switch(rootmsgb.exec()) {
        case QMessageBox::Ok:
            break;

        default:
            break;
        }

#endif
#ifdef Q_OS_MAC
        /*
        QProcess osascriptProc;
        osascriptProc.start("osascript");
        osascriptProc.write(QString("do shell script \""+app.applicationFilePath()+"\" with administrator privileges\n").toAscii().data());
        osascriptProc.closeWriteChannel();
        osascriptProc.waitForFinished(-1);
        */
        //qDebug() << QString("osascript -e 'do shell script \"%1 %2\" with administrator privileges'").arg(app.applicationFilePath()).arg(argsconc);
        //QProcess::startDetached(QString("osascript -e 'do shell script \"%1 %2\" with administrator privileges'").arg(app.applicationFilePath()).arg(argsconc));
        QProcess::startDetached("osascript", QStringList() << "-e" << QString("do shell script \"'%1' %2\" with administrator privileges").arg(app.applicationFilePath()).arg(argsconcSingleQuote));
        return true;
#endif
    }

    return false;
}
#endif

QString g_LogPath;
QString g_LogDir;
#include <iostream>

void crashMessageOutput(QtMsgType type, const QMessageLogContext &c, const QString & str) {
    QString txt;
    switch(type) {
    case QtDebugMsg:
        txt = QString("Debug: %1:%2 %3").arg(c.function).arg(c.line).arg(str);
        break;

    case QtWarningMsg:
        txt = QString("Warning: %1:%2 %3").arg(c.function).arg(c.line).arg(str);
        break;

    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(str);
        break;

    case QtFatalMsg:
        txt = QString("Fatal: %").arg(str);
        abort();
    }

    QFile outFile(g_LogPath);
    outFile.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;

    std::wcout<<txt.toStdWString()<<std::endl;
}

void installLogHandler() {
    QDir tmpDir(QStandardPaths::standardLocations(QStandardPaths::TempLocation).first());
    tmpDir.mkdir("deepin-boot-maker");
    g_LogDir = tmpDir.path() + "/deepin-boot-maker";
    g_LogPath = QDir::toNativeSeparators(g_LogDir + "/deepin-boot-maker.log");
    qDebug() << "Install Log to " << g_LogPath;
    qInstallMessageHandler(crashMessageOutput);
}

void loadTranslate(QApplication& app) {
    QTranslator *translator = new QTranslator();
    QString tnapplang;
    QString tnappcoun;
    QString clangcode = "";
    QStringList allappargs = app.arguments();
    QList<QPair<QString, QString> > oppairs;

    for(QList<QString>::const_iterator i = allappargs.constBegin(); i < allappargs.constEnd(); ++i) {
        if(i->count('=') == 1) {
            oppairs.append(QPair<QString, QString> (i->section('=', 0, 0).simplified(), i->section('=', 1, 1).simplified()));
        }
    }

    for(QList<QPair<QString, QString> >::const_iterator i = oppairs.constBegin(); i < oppairs.constEnd(); ++i) {
        if(i->first.contains("lang", Qt::CaseInsensitive)) {
            clangcode = i->second;
            tnapplang = clangcode.left(2);

            if(clangcode.contains('_') && clangcode.size() == 5) {
                tnappcoun = clangcode.section('_', -1, -1);
            }

            break;
        }
    }

    if(clangcode.isEmpty()) {
        clangcode = QLocale::system().name();
        tnapplang = clangcode.left(2);

        if(clangcode.contains('_') && clangcode.size() == 5) {
            tnappcoun = clangcode.section('_', -1, -1);
        }
    }

    QString tranlateUrl;

    if (!tnappcoun.isEmpty()) {
        tranlateUrl = QString(":/po/%1_%2.qm").arg(tnapplang).arg(tnappcoun);
    }

    if (!QFile::exists(tranlateUrl)) {
        tranlateUrl = QString(":/po/%1.qm").arg(tnapplang);
    }


    if(!QFile::exists(tranlateUrl)) {
        tranlateUrl = ":/en_US.qm";
    }

    qDebug()<<&app<<"locate: "<<clangcode<<"\nload translate file: "<<tranlateUrl;

    if(translator->load(tranlateUrl)) {
        app.installTranslator(translator);
    }
}

/*select the best font*/
#include <QFontDatabase>
void loadFonts() {
    QFontDatabase database;
    QStringList fontlist = database.families();

    QStringList preferList;
    preferList.append("Microsoft YaHei");
    preferList.append("微软雅黑");
    preferList.append("SimHei");
    preferList.append("黑体");

    foreach (QString font, preferList) {
        if (fontlist.contains(font)) {
            QFont newFont = QFont(font);
            QApplication::setFont(newFont);
            return;
        }
    }
}

#include <XSys>

int main(int argc, char **argv) {
    QApplication app(argc, argv, true);
    installLogHandler();
    loadTranslate(app);

#ifdef Q_OS_WIN32
    loadFonts();
#endif

#ifdef Q_OS_UNIX
    if(SwitchToRoot(app))
        exit(0);
#endif
    qDebug()<<app.font();

    QIcon icon;
    icon.addFile(":/ui/images/deepin-boot-maker.png");

    DWindowUI mainWindow;
    mainWindow.setWindowIcon(icon);
    mainWindow.setWindowTitle(AppTitle());
    mainWindow.show();

//    QDirIterator it(":", QDirIterator::Subdirectories);
//    while (it.hasNext()) {
//        qDebug() << it.next();
//    }

    app.exec();
    exit(0);
}


