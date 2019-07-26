/* mainwindow.cpp - the main window and business logic

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
#include "include/mainwindow.h"
#include "ui_mainwindow.h"
#include "include/display.h"
#include "include/settingsdialog.h"
#include "include/webdownloader.h"
#include "include/grefirmware.h"

#include <QMessageBox>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QtSerialPort/QSerialPort>
#include <QDateTime>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QFile>

/* Constructor
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    scannerMode(SCANNER_MODE_UNKNOWN),
    nakCount(0)
{
    ui->setupUi(this);

	// allocate the main classes
    display = new Display(this);
    setCentralWidget(display);

    serial = new QSerialPort(this);
    settings = new SettingsDialog(this);
    progress = nullptr;
    downloader = new WebDownloader(this);
    firmware = new GREFirmware(this);
    parser = new GREParser(this);

	// Connect the UI actions to the class slots
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionSettings, SIGNAL(triggered()), settings, SLOT(show()));
    connect(ui->actionClear, SIGNAL(triggered()), display, SLOT(clear()));
    connect(ui->actionDownloadFirmware, SIGNAL(triggered()), this, SLOT(processFirmwareDownload()));
    connect(ui->actionUpdateFirmware, SIGNAL(triggered()), this, SLOT(processFirmwareUpdate()));
    connect(ui->actionSetTime, SIGNAL(triggered()), this, SLOT(setTime()));
    connect(ui->actionClearPassword, SIGNAL(triggered(bool)), parser, SLOT(clearPassword()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// Setup the actions
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionSettings->setEnabled(true);
    ui->actionUpdateFirmware->setEnabled(false);
    ui->actionSetTime->setEnabled(false);
    ui->actionClearPassword->setEnabled(false);

	// Setup the scanner file directory and scanner type configuration
    scannerFileDirectory = QStandardPaths::locate(QStandardPaths::AppDataLocation, QString(), QStandardPaths::LocateDirectory);
    scannerTypeConfig();

	// Wire the class signals to the class slots
    connect(settings, SIGNAL(applySettings()), this, SLOT(handleApplySettings()));

    connect(downloader, SIGNAL(downloadComplete()), this, SLOT(processDownloadComplete()));
    connect(downloader, SIGNAL(downloadError()), this, SLOT(processDownloadError()));

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleSerialError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    connect(parser, SIGNAL(sendData(QByteArray)), this, SLOT(writeData(QByteArray)));
    connect(parser, SIGNAL(updateEOT(void)), this, SLOT(processEOT(void)));
    connect(parser, SIGNAL(updateEnq(void)), this, SLOT(processEnq(void)));
    connect(parser, SIGNAL(updateAck(void)), this, SLOT(processAck(void)));
    connect(parser, SIGNAL(updateDLE(void)), this, SLOT(processDLE(void)));
    connect(parser, SIGNAL(updateNak(void)), this, SLOT(processNak(void)));
    connect(parser, SIGNAL(updateCan(void)), this, SLOT(processCan(void)));
    connect(parser, SIGNAL(updateCpuUpdateMode(void)), this, SLOT(processCpuUpdateMode(void)));
    connect(parser, SIGNAL(updatePowerStatus(bool)), this, SLOT(processPowerStatus(bool)));
    connect(parser, SIGNAL(updateVersion(GREParser::VersionVal)), this, SLOT(processVersion(GREParser::VersionVal)));
    connect(parser, SIGNAL(updateCCDump(QString)), this, SLOT(processCCDump(QString)));

	// Setup the download timeout timer
    dlTimer = new QTimer(this);
    dlTimer->setInterval(30000);
    dlTimer->setSingleShot(true);
    connect(dlTimer, SIGNAL(timeout()), this, SLOT(dlTimeout()));

	// Setup the communications timeout timer
    commsTimer = new QTimer(this);
    commsTimer->setInterval(3000);
    commsTimer->setSingleShot(true);
    connect(commsTimer, SIGNAL(timeout()), this, SLOT(commsTimeout()));
}
/* Destructor
*/
MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}
/* openSerialPort - open the serial communications to the scanner
*/
void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->getCurrentSettings();
    // Close the port if it is open
    if (serial->isOpen())
    {
        serial->close();
    }
    serial->setPortName(p.serialPortName);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        parser->initialize();
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionSettings->setEnabled(false);
        ui->actionUpdateFirmware->setEnabled(false);
        ui->actionSetTime->setEnabled(true);
        ui->actionClearPassword->setEnabled(true);
        display->putMessage(tr("Connected to %1 " ).arg(p.serialPortName));
    } else {
        display->putError(tr("Open Serial Port Error: %1").arg(serial->errorString()));
        QMessageBox::critical(this, tr("Error"), serial->errorString());
    }
}
/* closeSerialPort - close the serial communications to the scanner
*/
void MainWindow::closeSerialPort()
{
    SettingsDialog::Settings p = settings->getCurrentSettings();
    scannerMode = SCANNER_MODE_UNKNOWN;
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionSettings->setEnabled(true);
    ui->actionUpdateFirmware->setEnabled(false);
    ui->actionSetTime->setEnabled(false);
    ui->actionClearPassword->setEnabled(false);
    if (serial->isOpen())
    {
        serial->close();
    }
    display->putMessage(tr("Disconnected from %1 " ).arg(p.serialPortName));
}
/* about - A simple about box
*/
void MainWindow::about()
{
    QMessageBox::about(this, tr("About GREFwTool"),
                       tr("<b>GREFwTool</b> Version ") + qApp->applicationVersion() + tr(" updates GRE scanner firmware using Qt." ));
}
/* writeData - write data to the scanner
*/
void MainWindow::writeData(const QByteArray &data)
{
    SettingsDialog::Settings p = settings->getCurrentSettings();
   if (serial->isOpen())
   {
        serial->write(data);
        if(p.protocolDebugEnabled)
            displayProtocol(data, true);
   }
}
/* readData - read data from the scanner
*/
void MainWindow::readData()
{
    SettingsDialog::Settings p = settings->getCurrentSettings();
    QByteArray data;
    if (serial->isOpen())
    {
        data = serial->readAll();
        if(p.protocolDebugEnabled)
            displayProtocol(data, false);
        parser->receiveData(data);
    }
}
/* dlTimeout - process the download timeout timer
*/
void MainWindow::dlTimeout()
{
    QString message("Timeout while downloading firmware file.");
    display->putError(message);
    QMessageBox::critical(this, tr("Timeout"), message);
    downloadMode = DOWNLOAD_MODE_ERROR;
    ui->actionDownloadFirmware->setEnabled(true);


}
/* commsTimeout - process the communications timeout timer
*/
void MainWindow::commsTimeout()
{
    QString message("Timeout while updating scanner. Please reset scanner and try again.");
    display->putError(message);
    progress->cancel();
    processCan(); // cancel and close serial port
}
/* displayProtocol - display bytes sent and received from scanner
		This is used for debugging purposes
*/
void MainWindow::displayProtocol(const QByteArray &data, bool txFlag)
{
    QString displayBytes;
    QString displayAscii;
    QString value;
    for(QByteArray::const_iterator it = data.cbegin(); it != data.cend(); it++)
    {
        unsigned char c = *it;
        value = QString("%1 ").arg(c, 2, 16, QLatin1Char('0'));
        displayBytes.append(value);
        if((c >= ' ' && c <= '~') )
        {
            displayAscii.append(c);
        }
        else
        {
            value = QString("<%1>").arg(c, 2, 16, QLatin1Char('0'));
            displayAscii.append(value);
        }

    }
    if(txFlag)
        display->putTxBytes(tr("Tx[Hex: %1 Ascii: %2] ").arg(displayBytes).arg(displayAscii));
    else
        display->putRxBytes(tr("Rx[Hex: %1 Ascii: %2] ").arg(displayBytes).arg(displayAscii));

}
/* processEOT - process the CPU Update complete character from scanner
*/
void MainWindow::processEOT(void )
{
    commsTimer->stop();
    QString message("CPU Update Complete. Reconnect after scanner reboots. ");
    closeSerialPort();  // Prevent error on Scanner Report
    updatePacket.clear();
    scannerMode = SCANNER_MODE_UPDATE_DONE;
    progress->reset();
    ui->actionUpdateFirmware->setEnabled(false);
    display->putMessage(message);
    nakCount = 0;
}
/* processEnq - process the CPU Update start character from scanner
*/
void MainWindow::processEnq(void )
{
    QString message("CPU is updating.");
    commsTimer->stop();
    if(scannerMode == SCANNER_MODE_CPU_UPDATE)
    {
        updatePacket = firmware->getNextPacket();
        if(!updatePacket.isEmpty())
        {
            parser->sendPacket(updatePacket);
            progress->setValue(firmware->getOffset());
        }
        commsTimer->start();
        scannerMode = SCANNER_MODE_UPDATE_IN_PROGRESS;
        display->putMessage(message);
    }
    nakCount = 0;
}
/* processAck - process the acknowledgement character from scanner
*/
void MainWindow::processAck(void )
{
    commsTimer->stop();
    if(scannerMode == SCANNER_MODE_UPDATE_IN_PROGRESS)
    {
        updatePacket = firmware->getNextPacket();
        if(!updatePacket.isEmpty())
        {
            parser->sendPacket(updatePacket);
            progress->setValue(firmware->getOffset());
        }
        commsTimer->start();
    }
    nakCount = 0;
}
/* processDLE - process the CPU Update wait character from scanner
*/
void MainWindow::processDLE(void )
{
    QString message("CPU Update Wait. ");
    commsTimer->start();
    nakCount = 0;
    display->putMessage(message);
}
/* processNak - process the negative acknowledgement character from scanner
*/
void MainWindow::processNak(void )
{
    QString message("CPU Update Rejected. ");
    commsTimer->stop();
    if((scannerMode == SCANNER_MODE_UPDATE_IN_PROGRESS) && !updatePacket.isEmpty())
    {
        if(++nakCount > 2)
        {
            scannerMode = SCANNER_MODE_UPDATE_ERROR;
            progress->cancel();
            display->putError(message);
            QMessageBox::critical(this, tr("Error"), message);
            return;
        }
        parser->sendPacket(updatePacket);
        commsTimer->start();
    }
}
/* processCan - process the CPU Update cancel character from scanner
*/
void MainWindow::processCan(void )
{
    QString message("CPU Update Error.");
    commsTimer->stop();
    updatePacket.clear();
    scannerMode = SCANNER_MODE_UPDATE_ERROR;
    progress->cancel();
    closeSerialPort();  // Prevent error on Scanner Report
    display->putError(message);
    QMessageBox::critical(this, tr("Error"), message);
}
/* processCpuUpdateMode - process the CPU Update Mode character from scanner
*/
void MainWindow::processCpuUpdateMode(void )
{
    QString message("Scanner is in CPU Update Mode. ");
    display->putMessage(message);
    ui->actionUpdateFirmware->setEnabled(true);
    scannerMode = SCANNER_MODE_CPU_UPDATE;
}
/* processPowerStatus - process the power status response from scanner
*/
void MainWindow::processPowerStatus(const bool &data )
{
    QString message = QString("Scanner is %1. ").arg((data)?"ON":"off");
    display->putMessage(message);
    ui->actionUpdateFirmware->setEnabled(false);
    scannerMode = (data)?SCANNER_MODE_ON:SCANNER_MODE_OFF;
}
/* processVersion - process the version response from scanner
*/
void MainWindow::processVersion(const GREParser::VersionVal &data )
{
    QString message = QString("Version %1  %2  %3  %4  %5 ").arg(data.model).arg(data.ver1).arg(data.ver2).arg(data.ver3).arg(data.ver4);
    display->putMessage(message);
}
/* processCCDump - Simple processing of CCDump response from scanner
*/
void MainWindow::processCCDump(const QString &data )
{
    QString message = QString("CCDump: %1 ").arg(data);
    display->putMessage(message);
}
/* handleSerialError - process Serial Port errors
*/
void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
{
    switch(error)
    {
    case QSerialPort::ResourceError:
        display->putError(tr("Serial Port Error: %1").arg(serial->errorString()));
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
        break;
    // Open errors are handled elsewhere
    case QSerialPort::NotOpenError:  // Qt 5.2 and above
    case QSerialPort::WriteError:
    case QSerialPort::ReadError:
    case QSerialPort::UnsupportedOperationError:
    case QSerialPort::TimeoutError:  // Qt 5.2 and above
    case QSerialPort::UnknownError:
        display->putError(tr("Serial Port Error: %1").arg(serial->errorString()));
        break;
    default:
        // do nothing
        break;
    }
}
/* handleApplySettings - process apply button in settings dialog being pressed
*/
void MainWindow::handleApplySettings()
{
    scannerTypeConfig();
}
/* scannerTypeConfig - configure remote directories and filenames based on firmware type
*/
void MainWindow::scannerTypeConfig()
{
    SettingsDialog::Settings s = settings->getCurrentSettings();
    switch(s.firmwareType)
    {
    case SettingsDialog::PSR800Scanner:
        remoteFileDirectory = QString::fromUtf8("http://902Public:Iiu893%40f@scanner-update.com/0602902/public/");
        versionFileName = QString::fromUtf8("versions.php");
        break;
    case SettingsDialog::PRO18Scanner:
        remoteFileDirectory = QString::fromUtf8("http://903Public:glew%25g93@scanner-update.com/0602903/public/");
        versionFileName = QString::fromUtf8("versions.php");
        break;
    case SettingsDialog::PRO668Scanner:
        remoteFileDirectory = QString::fromUtf8("http://pro668Public:;oeig(%40p06@scanner-update.com/pro668/public/");
        versionFileName = QString::fromUtf8("versions.php");
        break;
    case SettingsDialog::WS1080Scanner:
        remoteFileDirectory = QString::fromUtf8("http://ws1080Public:1Lus*eug84@scanner-update.com/ws1080/public/");
        versionFileName = QString::fromUtf8("versions.php");
        break;
    case SettingsDialog::PRO107Scanner:
        remoteFileDirectory = QString::fromUtf8("http://Public0602:lkjgie!853@scanner-update.com/0602/public/");
        versionFileName = QString::fromUtf8("versions.php");
        break;
    case SettingsDialog::PSR700Scanner:
        remoteFileDirectory = QString::fromUtf8("http://901Public:pw2+DR4i@scanner-update.com/0602-901/public/");
        versionFileName = QString::fromUtf8("versions.php");
        break;
    default:
        remoteFileDirectory.clear();
        remoteFileDirectory.clear();
        versionFileName.clear();
        break;

    }
}
/* processFirmwareDownload - start the process of downloading firmware from website
*/
void MainWindow::processFirmwareDownload()
{
    QUrl remote(remoteFileDirectory+versionFileName);
    QString local(scannerFileDirectory+versionFileName);
    QString message("Downloading Version File. ");
    ui->actionDownloadFirmware->setEnabled(false);
    display->clear();
    display->putMessage(message);
    downloadMode = DOWNLOAD_MODE_VER_FILE;
    dlTimer->start();
    downloader->doDownload(remote, local);
}
/* processDownloadComplete - process the file downloaded from website
		Multiple files are downloaded.
		The website has a version file that lists the filenames needed to be downloaded.
*/
void MainWindow::processDownloadComplete()
{
    QFile *file;
    QString message;
    QString html;
    // tokens
    QString cpu("CPU");
    QString cpuRelease("CPURelease");
    QString cpu2("CPU2");
    QString cpu2Release("CPU2Release");
    // Stop the timer
    dlTimer->stop();
	//	downloadMode is the state and indicates the file being downloaded.
    if(downloadMode == DOWNLOAD_MODE_VER_FILE)
    {   // process the version file, which should be the first one downloaded.
        message = QString("Version File Downloaded. ");
        display->putMessage(message);
        file = new QFile(scannerFileDirectory+versionFileName);
        cpuFileName.clear();
        cpu2FileName.clear();
        cpuFileVersion.clear();
        cpu2FileVersion.clear();
        cpuReleaseName.clear();
        cpu2ReleaseName.clear();
        if(file != nullptr)
        {
            if(file->open(QFile::ReadOnly))
            {   // extract the filename and versions
                QTextStream its(file);
                QString line, lastType;
                lastType.clear();
                while(its.readLineInto(&line))
                {
                    QStringList parms = line.split('\t');

                    if(parms.at(0).compare(cpu) == 0)
                    {
                        cpuFileVersion = parms.at(1).trimmed();
                        cpuFileName = parms.at(2).section(' ', 0, 0);
                    }
                    else if(parms.at(0).compare(cpuRelease) == 0)
                    {
                        cpuReleaseName = parms.at(2).trimmed();
                    }
                    else if(parms.at(0).compare(cpu2) == 0)
                    {
                        cpu2FileVersion = parms.at(1).trimmed();
                        cpu2FileName = parms.at(2).trimmed();
                    }
                    else if(parms.at(0).compare(cpu2Release) == 0)
                    {
                        cpu2ReleaseName = parms.at(2).trimmed();
                    }
                    // some files only use a ReleaseNotes tag
                    else if(parms.at(0).compare(QString("ReleaseNotes"))== 0)
                    {   // ReleaseNotes tag is used for multiple files so check the previous tag
                        if(lastType.compare(cpu) == 0)
                            cpuReleaseName = parms.at(2).trimmed();
                        else if (lastType.compare(cpu2) == 0)
                            cpu2ReleaseName = parms.at(2).trimmed();
                    }
                    lastType = parms.at(0).trimmed();
                }
                file->close();
            }
            delete file;
            file = nullptr;
        }
        // check CPU filename
        if(cpuFileName.isEmpty())
        {
            message = QString("Version File Parsing Error. ");
            display->putError(message);
            QMessageBox::critical(this, tr("Error"), message);
            downloadMode = DOWNLOAD_MODE_ERROR;
            ui->actionDownloadFirmware->setEnabled(true);
        }
        else
        {	// setup to downnload CPU firmware file
            QUrl remote(remoteFileDirectory+cpuFileName);
            QString local(scannerFileDirectory+cpuFileName);
            message = QString("Downloading Firmware Ver: %1 Filename: %2 ").arg(cpuFileVersion).arg(cpuFileName);
            display->putMessage(message);
            dlTimer->start();
            downloader->doDownload(remote, local);
            downloadMode = DOWNLOAD_MODE_CPU_FILE;
        }
    }
    else if (downloadMode == DOWNLOAD_MODE_CPU_FILE)
    {   // CPU firmware file done so check CPU release filename
        if(cpuReleaseName.isEmpty())
        {

            message = QString("Version File Parsing Error. ");
            display->putError(message);
            QMessageBox::critical(this, tr("Error"), message);
            downloadMode = DOWNLOAD_MODE_ERROR;
            ui->actionDownloadFirmware->setEnabled(true);
        }
        else
        {   // setup to download CPU release file
            QUrl remote(remoteFileDirectory+cpuReleaseName);
            QString local(scannerFileDirectory+cpuReleaseName);
            message = QString("Downloading Firmware Release Notes: %1 ").arg(cpuReleaseName);
            display->putMessage(message);
            dlTimer->start();
            downloader->doDownload(remote, local);
            downloadMode = DOWNLOAD_MODE_CPU_RELEASE;
        }
    }
    else if (downloadMode == DOWNLOAD_MODE_CPU_RELEASE)
    {   // CPU release file done so display it as HTML
        file = new QFile(scannerFileDirectory+cpuReleaseName);
        if(file != nullptr)
        {
            if(file->open(QFile::ReadOnly))
            {
                QTextStream rts(file);
                html = rts.readAll();
                display->putHtml(html);
                file->close();
            }
            delete file;
            file = nullptr;
        }
		// In current usage there is no CPU2 firmware file so finish
        if(cpu2FileName.isEmpty())
        {
            message = QString("Fimware Download Complete. ");
            display->putMessage(message);
            downloadMode = DOWNLOAD_MODE_DONE;
            ui->actionDownloadFirmware->setEnabled(true);
        }
        else
        {   // Future case to setup for downloading CPU2 firmware file
            QUrl remote(remoteFileDirectory+cpu2FileName);
            QString local(scannerFileDirectory+cpu2FileName);
            message = QString("Downloading Firmware2 Ver: %1 Filename: %2 ").arg(cpu2FileVersion).arg(cpu2FileName);
            display->putMessage(message);
            dlTimer->start();
            downloader->doDownload(remote, local);
            downloadMode = DOWNLOAD_MODE_CPU2_FILE;
        }
    }
    else if (downloadMode == DOWNLOAD_MODE_CPU2_FILE)
    {  // Future case: CPU2 firmware file done so check CPU2 release filename
        if(cpu2ReleaseName.isEmpty())
        {

            message = QString("Version File Parsing Error. ");
            display->putError(message);
            QMessageBox::critical(this, tr("Error"), message);
            downloadMode = DOWNLOAD_MODE_ERROR;
            ui->actionDownloadFirmware->setEnabled(true);
        }
        else
        {
            QUrl remote(remoteFileDirectory+cpu2ReleaseName);
            QString local(scannerFileDirectory+cpu2ReleaseName);
            message = QString("Downloading Firmware2 Release Notes: %1 ").arg(cpu2ReleaseName);
            display->putMessage(message);
            dlTimer->start();
            downloader->doDownload(remote, local);
            downloadMode = DOWNLOAD_MODE_CPU2_RELEASE;
        }
    }
    else if (downloadMode == DOWNLOAD_MODE_CPU2_RELEASE)
    {   // Future case: CPU2 release file done so display it as HTML
        file = new QFile(scannerFileDirectory+cpu2ReleaseName);
        if(file != nullptr)
        {
            if(file->open(QFile::ReadOnly))
            {
                QTextStream rts(file);
                html = rts.readAll();
                display->putHtml(html);
                file->close();
            }
            delete file;
            file = nullptr;
        }
        message = QString("Firmware Download Complete.  ");
        display->putMessage(message);
        downloadMode = DOWNLOAD_MODE_DONE;
        ui->actionDownloadFirmware->setEnabled(true);
    }
    else
    {   // Unhandled downloadMode value so show error
        QString message("Firmware Download mode error. ");
        display->putError(message);
        QMessageBox::critical(this, tr("Error"), message);
        downloadMode = DOWNLOAD_MODE_ERROR;
        ui->actionDownloadFirmware->setEnabled(true);
    }
}
/* processDownloadError - process error occuring during file download from website
*/
void MainWindow::processDownloadError()
{
    dlTimer->stop();
    QString message("Error while downloading file. ");
    display->putError(message);
    QMessageBox::critical(this, tr("Error"), message);
    downloadMode = DOWNLOAD_MODE_ERROR;
    ui->actionDownloadFirmware->setEnabled(true);

}
/* processFirmwareUpdate - start the CPU firmware update
*/
void MainWindow::processFirmwareUpdate()
{
    SettingsDialog::Settings s = settings->getCurrentSettings();
    if(firmware->openFile(scannerFileDirectory))
    {
        if(firmware->getPlatform() == s.firmwareType)
        {
            if((s.scannerType != s.firmwareType) && !firmware->transcode(s.scannerType))
			{   // if transcode is needed and not supported then error
                QString message("Transcode not supported for this scanner or version of firmware. ");
				display->putError(message);
				QMessageBox::critical(this, tr("Error"), message);
				return;
			}
            ui->actionUpdateFirmware->setEnabled(false);
            // Create progress dialog, if needed, without an abort button.
            if(progress == nullptr)
            {
                progress = new QProgressDialog("Updating Firmware", QString(), 0, firmware->getImageSize(), this);
            }
            else // update image size of existing dialog
            {
                progress->setMaximum(firmware->getImageSize());
            }
			// send the header
            updatePacket = firmware->getFirstPacket();
            parser->sendPacket(updatePacket);
            commsTimer->start();
        }
        else
        {
            QString message("Wrong firmware file for scanner. ");
            display->putError(message);
            QMessageBox::critical(this, tr("Error"), message);
        }
    }

}
/* setTime - set the date and time on scanner using current computer date and time
*/
void MainWindow::setTime()
{
    QString message("Date and Time sent to scanner. ");
    parser->setDateTime(QDateTime::currentDateTime());
    display->putMessage(message);
}
