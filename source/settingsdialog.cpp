/* settingsdialog.cpp - a simple settings dialog class

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
#include "include/settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QtSerialPort/QSerialPortInfo>

// Static not available string
static const char naString[] = "N/A";

// Static settings strings
static const char settingsString[] = "Settings";
static const char scannerIndexString[] = "ScannerIndex";
static const char firmwareIndexString[] = "FirmwareIndex";
static const char portIndexString[] = "PortIndex";

/* Constructor
*/
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
	
    // connect ui to class slots
    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->serialPortInfoListBox, SIGNAL(currentIndexChanged(int)), this, SLOT(showPortInfo(int)));
    connect(ui->rescanButton, SIGNAL(clicked(bool)), this, SLOT(fillPortsInfo()));
    connect(ui->scannerTypeListBox, SIGNAL(currentIndexChanged(int)), this, SLOT(fillFirmwareBox(int)));

    // setup ui elements
    fillScannerParameters();
    fillPortsInfo();

	// update settings values
    updateSettings();
}
/* Destructor
*/
SettingsDialog::~SettingsDialog()
{
    delete ui;
}
/* getCurrentSettings - returns a structure with the current settings values
*/
SettingsDialog::Settings SettingsDialog::getCurrentSettings() const
{
    return currentSettings;
}
/* apply - process the apply button and save settings to persistent storage
*/
void SettingsDialog::apply()
{
    QSettings config;
    int idx;
    updateSettings();
    config.beginGroup(settingsString);
    idx = ui->scannerTypeListBox->currentIndex();
    if(idx < 0)
        idx = 0;
    config.setValue(scannerIndexString, idx);
    idx = ui->firmwareListBox->currentIndex();
    if(idx < 0)
        idx = 0;
    config.setValue(firmwareIndexString, idx);
    idx = ui->serialPortInfoListBox->currentIndex();
    if(idx < 0)
        idx = 0;
    config.setValue(portIndexString, idx);
    config.endGroup();
    hide();
    emit applySettings();
}
/* showPortInfo - display serial port information of currently selected serial port
*/
void SettingsDialog::showPortInfo(int idx)
{
    if(idx < 0)
        return;

    QStringList portList = ui->serialPortInfoListBox->itemData(idx).toStringList();
    ui->descriptionLabel->setText(tr("Description: %1").arg(portList.count() > 1 ? portList.at(1) : tr(naString)));
    ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(portList.count() > 2 ? portList.at(2) : tr(naString)));
    ui->serialNumberLabel->setText(tr("Serial Number: %1").arg(portList.count() > 3 ? portList.at(3) : tr(naString)));
    ui->locationLabel->setText(tr("Location: %1").arg(portList.count() > 4 ? portList.at(4) : tr(naString)));
    ui->vidLabel->setText(tr("Vendor Id: %1").arg(portList.count() > 5 ? portList.at(5) : tr(naString)));
    ui->pidLabel->setText(tr("Product Id: %1").arg(portList.count() > 6 ? portList.at(6) : tr(naString)));
}
/* fillPortsInfo - fill the serial port list box with serial port information from computer
*/
void SettingsDialog::fillPortsInfo()
{
    QSettings config;
    int idx;
    QString description;
    QString manufacturer;
    QString serialNumber;
    QString vendorIdentifier;
    QString productIdentifier;

    config.beginGroup(settingsString);
    ui->serialPortInfoListBox->clear();
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QStringList portList;
        if(!info.isBusy())
        {
            description  = (!info.description().isEmpty()) ? info.description() : naString;
            manufacturer = (!info.manufacturer().isEmpty()) ? info.manufacturer() : naString;
            serialNumber = (!info.serialNumber().isEmpty()) ? info.serialNumber() : naString;
            vendorIdentifier = (info.hasVendorIdentifier()) ? QString("0x%1").arg(info.vendorIdentifier(), 4, 16, QLatin1Char('0')) : naString;
            productIdentifier = (info.hasProductIdentifier()) ? QString("0x%1").arg(info.productIdentifier(), 4, 16, QLatin1Char('0')) : naString;
            portList << info.portName()
                     << description
                     << manufacturer
                     << serialNumber
                     << info.systemLocation()
                     << vendorIdentifier
                     << productIdentifier;
            ui->serialPortInfoListBox->addItem(portList.first(), portList);
        }
    }
    config.endGroup();
    idx = config.value(portIndexString).toInt();
    if((idx < 0) || (idx >= ui->serialPortInfoListBox->count()))
        idx = 0;
    showPortInfo(idx);
    ui->serialPortInfoListBox->setCurrentIndex(idx);
}
/* fillScannerParameters - initialize the scanner list boxes
*/
void SettingsDialog::fillScannerParameters()
{
    QSettings config;
    config.beginGroup(settingsString);
    ui->scannerTypeListBox->addItem(QStringLiteral("PSR-800"), SettingsDialog::PSR800Scanner);
    ui->scannerTypeListBox->addItem(QStringLiteral("Pro-18"), SettingsDialog::PRO18Scanner);
    ui->scannerTypeListBox->addItem(QStringLiteral("Pro-668"), SettingsDialog::PRO668Scanner);
    ui->scannerTypeListBox->addItem(QStringLiteral("WS-1080"), SettingsDialog::WS1080Scanner);
    ui->scannerTypeListBox->addItem(QStringLiteral("Pro-107"), SettingsDialog::PRO107Scanner);
    ui->scannerTypeListBox->addItem(QStringLiteral("PSR-700"), SettingsDialog::PSR700Scanner);
    ui->scannerTypeListBox->setCurrentIndex(config.value(scannerIndexString, 0).toInt());
    fillFirmwareBox(ui->scannerTypeListBox->currentIndex());
    ui->firmwareListBox->setCurrentIndex(config.value(firmwareIndexString, 0).toInt());
    config.endGroup();

}
/* fillFirmwareBox - initialize the firmware list box
		This is a simple routine for now
*/
void SettingsDialog::fillFirmwareBox(int idx)
{
    ui->firmwareListBox->clear();
    ui->firmwareListBox->addItem(ui->scannerTypeListBox->itemText(idx), ui->scannerTypeListBox->itemData(idx));
    switch (ui->scannerTypeListBox->itemData(idx).toInt())
    {
    case SettingsDialog::PSR800Scanner:
    case SettingsDialog::PRO18Scanner:
    case SettingsDialog::PRO668Scanner:
        ui->firmwareListBox->addItem(QStringLiteral("WS-1080"), SettingsDialog::WS1080Scanner);
        break;
    default:
        break;
    }
    ui->firmwareListBox->setCurrentIndex(0);
}
/* updateSettings - the dialog control contents are stored to the current settings structure
*/
void SettingsDialog::updateSettings()
{
    currentSettings.scannerType = static_cast<SettingsDialog::Scanner>(ui->scannerTypeListBox->itemData(ui->scannerTypeListBox->currentIndex()).toInt());
    currentSettings.stringScannerType = ui->scannerTypeListBox->currentText();
    currentSettings.firmwareType = static_cast<SettingsDialog::Scanner>(ui->firmwareListBox->itemData(ui->firmwareListBox->currentIndex()).toInt());
    currentSettings.stringFirmwareType = ui->firmwareListBox->currentText();
    currentSettings.serialPortName = ui->serialPortInfoListBox->currentText();
    currentSettings.baudRate = QSerialPort::Baud115200;
    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = QSerialPort::Data8;
    currentSettings.stringDataBits = QString::number(currentSettings.dataBits);

    currentSettings.parity = QSerialPort::NoParity;
    currentSettings.stringParity = tr("None");

    currentSettings.stopBits = QSerialPort::OneStop;
    currentSettings.stringStopBits = QString::number(currentSettings.stopBits);

    currentSettings.flowControl = QSerialPort::NoFlowControl;
    currentSettings.stringFlowControl = tr("None");

    currentSettings.protocolDebugEnabled = ui->protocolDebugCheckBox->isChecked();
}
