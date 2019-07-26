/* mainwindow.h - the main window and business logic

Copyright 2016 Eric A. Cottrell <eric.c.boston@gmail.com>

This file is part of GREFwTool. Source code is available at https://github.com/LinuxSheeple-E/GREFwTool

    GREFwTool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GREFwTool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GREFwTool.  If not, see <http://www.gnu.org/licenses/>.
	
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>

#include <QMainWindow>

#include <QtSerialPort/QSerialPort>
#include <QTimer>

#include "greparser.h"

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Display;
class SettingsDialog;
class QProgressDialog;
class WebDownloader;
class GREFirmware;
class GREParser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();
    void processFirmwareDownload();
    void processDownloadComplete();
    void processDownloadError();
    void processFirmwareUpdate();
    void setTime();
    void writeData(const QByteArray &data);
    void readData();
    void commsTimeout(void );
    void dlTimeout();
    void processEOT(void );
    void processEnq(void );
    void processAck(void );
    void processDLE(void );
    void processNak(void );
    void processCan(void );
    void processCpuUpdateMode(void );
    void processPowerStatus(const bool &data);
    void processVersion(const GREParser::VersionVal &data );
    void processCCDump(const QString &data);

    void handleSerialError(QSerialPort::SerialPortError error);
    void handleApplySettings();

private:
    void displayProtocol(const QByteArray &data, bool txFlag);
    void scannerTypeConfig();

private:
    Ui::MainWindow *ui;

    enum {
        SCANNER_MODE_UNKNOWN,
        SCANNER_MODE_OFF,
        SCANNER_MODE_ON,
        SCANNER_MODE_CPU_UPDATE,
        SCANNER_MODE_UPDATE_IN_PROGRESS,
        SCANNER_MODE_UPDATE_DONE,
        SCANNER_MODE_UPDATE_ERROR
    } scannerMode;

    enum {
        DOWNLOAD_MODE_UNKNOWN,
        DOWNLOAD_MODE_VER_FILE,
        DOWNLOAD_MODE_CPU_FILE,
        DOWNLOAD_MODE_CPU_RELEASE,
        DOWNLOAD_MODE_CPU2_FILE,
        DOWNLOAD_MODE_CPU2_RELEASE,
        DOWNLOAD_MODE_DONE,
        DOWNLOAD_MODE_ERROR
    } downloadMode;

    GREParser *parser;
    WebDownloader *downloader;
    GREFirmware *firmware;
    Display *display;
    SettingsDialog *settings;
    QProgressDialog *progress;


    QString remoteFileDirectory;
    QString scannerFileDirectory;
    QString versionFileName;
    QString cpuFileName;
    QString cpu2FileName;
    QString cpuFileVersion;
    QString cpu2FileVersion;
    QString cpuReleaseName;
    QString cpu2ReleaseName;

    QSerialPort *serial;
    QByteArray updatePacket;
    int nakCount;
    QTimer *commsTimer;
    QTimer *dlTimer;
};

#endif // MAINWINDOW_H
