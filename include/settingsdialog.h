/* settingsdialog.h - a simple settings dialog class

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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>


namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:

    enum Scanner {
        PSR800Scanner = 0xEE,
        PRO18Scanner = 0xEC,
        PRO668Scanner = 0xE4,
        WS1080Scanner = 0xE6,
        PRO107Scanner = 0xF0,
        PSR700Scanner = 0xEF,

    };

    struct Settings {
        Scanner scannerType;
        QString stringScannerType;
        Scanner firmwareType;
        QString stringFirmwareType;
        QString serialPortName;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool protocolDebugEnabled;
    };

    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    Settings getCurrentSettings() const;
signals:
    void applySettings();

private slots:
    void apply();
    void fillPortsInfo();
    void fillFirmwareBox(int idx);
    void showPortInfo(int idx);

private:
    void fillScannerParameters();
    void updateSettings();

private:
    Ui::SettingsDialog *ui;
    Settings currentSettings;
};

#endif // SETTINGSDIALOG_H
