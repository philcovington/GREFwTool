/* greparser.h - A simple GRE message generator/parser class

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
#ifndef GREPARSER_H
#define GREPARSER_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QMap>

class GREParser : public QObject
{
    Q_OBJECT
public:
    struct GetStatusVal {
        quint8 mode;
        quint8 flags;
        bool usbPower;
        quint16 battery;
        quint16 rssi;
        qint16 zeromatic;
        quint8 rLed;
        quint8 gLed;
        quint8 bLed;
        quint32 frequency;
        quint8 rxmode;
    };

    struct GetLCDVal {
        QByteArray lcd;
        QByteArray icons;
    };
    struct VersionVal {
        QString model;
        QString ver1;
        QString ver2;
        QString ver3;
        QString ver4;
    };

    explicit GREParser(QObject *parent = 0);
    ~GREParser();
    void initialize();
    void initializeWork();
    void setDateTime(const QDateTime &datetime);
    void receiveData(QByteArray &data);

signals:
    void sendData(const QByteArray &data);
    void updateEOT(void );
    void updateEnq(void );
    void updateAck(void );
    void updateDLE(void );
    void updateNak(void );
    void updateCan(void );
    void updateCpuUpdateMode(void );
    void updateStatus(const GREParser::GetStatusVal &data);
    void updateLCD(const GREParser::GetLCDVal &data);
    void updatePowerStatus(const bool &data);
    void updateVersion(const GREParser::VersionVal &data);
    void updateCCDump(const QString &data);

public slots:
    void setCCDump(bool enable);
    void getStatus(void );
    void getLcd(void );
    void getPowerStatus();
    void requestVersion(void );
    void clearPassword(void );
    void sendPacket(const QByteArray &data);
    void sendAck();
    void sendNak();

private:
    enum {
        MODE_WAIT_START = 0,
        MODE_RESPONSE_START,
        MODE_RESPONSE_DATA,
        MODE_RESPONSE_DATA_END,
        MODE_CCDUMP_START,
        MODE_CCDUMP_DATA,
    } mode;
    bool bootloaderActive; // CPU Application update mode
    void processCommand(const QByteArray &data);
    void processResponse(const QByteArray &data);
    QByteArray  commandData;
    QByteArray  responseData;
    QDateTime lastDate;
    GetStatusVal lastGetStatusVal;
    GetLCDVal lastGetLCDVal;
    bool lastPowerStatusVal;
    VersionVal lastVersionVal;
    QString lastCCDump;
};

#endif // GREPARSER_H
