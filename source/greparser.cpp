/* greparser.cpp - A simple GRE message generator/parser class

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
#include "include/greparser.h"

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QTimer>
/* Constructor
*/
GREParser::GREParser(QObject *parent) : QObject(parent)
{
    commandData.clear();
    responseData.clear();
    mode = MODE_WAIT_START;
    bootloaderActive = false;
}
/* Destructor
*/
GREParser::~GREParser()
{

}
/* initialize - initalize data and start bootloader check timer
*/
void GREParser::initialize()
{
    bootloaderActive = false;
    commandData.clear();
    responseData.clear();
    mode = MODE_WAIT_START;
    QTimer::singleShot(2000, this, initializeWork); // give time for parser to detect if Bootloader is active

}
/* initializeWork - complete initialize function after delay to check for active bootloader
		Bootloader has a very limited command set and invalid commands can cause firmware erasure.
*/
void GREParser::initializeWork()
{
    if(bootloaderActive) 
    {
        requestVersion();
    }
    else
    {
        setCCDump(false);
        getPowerStatus();
        requestVersion();
    }
}
/* setCCDump - Command to enable/disable CCDump in the scanner
*/
void GREParser::setCCDump(bool enable)
{
    QByteArray command("C");
    command.append(static_cast<unsigned char>((enable)?1:0));
    processCommand(command);
}
/* getStatus - Command to get status information from the scanner
*/
void GREParser::getStatus(void )
{
    QByteArray command("A");
    processCommand(command);
}
/* getLed - command to get LCD display information from the scanner
*/
void GREParser::getLcd(void )
{
    QByteArray command("L");
    processCommand(command);

}
/* getPowerStatus - command to get power status from the scanner
*/
void GREParser::getPowerStatus(void )
{
    QByteArray command("P");
    processCommand(command);

}
/* requestVersion - command to request version information from the scanner
		The format of the command is slightly different when bootloader is active
*/
void GREParser::requestVersion(void )
{
    QByteArray command("V");
    if(bootloaderActive == false)
        command.append('\0');
    processCommand(command);

}
/* clearPassword - command to clear the scanner password
*/
void GREParser::clearPassword(void )
{
    QByteArray command("p");
    processCommand(command);

}
/* sendPacket - send a data packet to the scanner
*/
void GREParser::sendPacket(const QByteArray &data)
{
    processCommand(data);
}
/* sendAck - send a packet acknowledgement to the scanner
*/
void GREParser::sendAck(void )
{
    QByteArray ack(1, static_cast<char>(0x06) );
    emit sendData(ack);

}
/* sendNak - send a packet negative acknowledgement to the scanner
*/
void GREParser::sendNak(void )
{
    QByteArray nak(1,static_cast<char>(0x15));
    emit sendData(nak);

}
/* setDateTime - send the date and time to the scanner
*/
void GREParser::setDateTime(const QDateTime &datetime)
{
    unsigned char uc;
    QByteArray command("t");
    uc = datetime.time().second() & 0xff;
    command.append(uc);
    uc = (datetime.time().second() >> 8) & 0xff;
    command.append(uc);
    uc = datetime.time().minute() & 0xff;
    command.append(uc);
    uc = (datetime.time().minute() >> 8) & 0xff;
    command.append(uc);
    uc = datetime.time().hour() & 0xff;
    command.append(uc);
    uc = (datetime.time().hour() >> 8) & 0xff;
    command.append(uc);
    uc = datetime.date().day() & 0xff;
    command.append(uc);
    uc = (datetime.date().day() >> 8) & 0xff;
    command.append(uc);
    uc = (datetime.date().month() - 1) & 0xff;
    command.append(uc);
    uc = ((datetime.date().month() - 1) >> 8) & 0xff;
    command.append(uc);
    uc = (datetime.date().year() - 1900) & 0xff;
    command.append(uc);
    uc = ((datetime.date().year() - 1900) >> 8) & 0xff;
    command.append(uc);
    uc = datetime.date().dayOfWeek() & 0xff;
    command.append(uc);
    uc = (datetime.date().dayOfWeek() >> 8) & 0xff;
    command.append(uc);
    uc = datetime.date().dayOfYear() & 0xff;
    command.append(uc);
    uc = (datetime.date().dayOfYear() >> 8) & 0xff;
    command.append(uc);
    command.append(static_cast<unsigned char>(datetime.isDaylightTime())?1:0);
    uc = 0;
    command.append(uc);
    processCommand(command);
}
/* receiveData - receive data from the scanner
*/
void GREParser::receiveData(QByteArray &data)
{

    processResponse(data);
}
/* processCommand - packetize the data and send to the scanner
*/
void GREParser::processCommand(const QByteArray &data)
{
    unsigned char checksum;
    commandData.clear();
    if(data.isEmpty())
        return;
    commandData[0] = 0x02; // packet start
    checksum = 0x03;       // initialize to include data end byte
    for(QByteArray::const_iterator it = data.cbegin(); it != data.cend(); it++)
    {
        checksum += *it;
        commandData.append(*it);

    }
    commandData.append((unsigned char)0x03);
    commandData.append(checksum);
    emit sendData(commandData);

}
/* processResponse - process the scanner responseData
		This function tries to determine the type of data and act based on the type
*/
void GREParser::processResponse(const QByteArray &data)
{
    static int dataLength = -1;
    static unsigned char checksum = 0;
    static int updateFlagCount = 0;
    unsigned char uc;
    if(data.isEmpty())
        return;
    for(QByteArray::const_iterator it = data.cbegin(); it != data.cend(); it++)
    {
        switch(mode)
        {
        case MODE_WAIT_START:   // Not in a packet, so wait for start
            switch (*it)
            {
            case 0x02:                     // packet start
                updateFlagCount = 0;
                mode = MODE_RESPONSE_START;
                break;
            case 0x04:                     // Firmware update complete indication
                updateFlagCount = 0;
                emit updateEOT();
                break;
            case 0x05:                     // Firmware update wait indication
                updateFlagCount = 0;
                emit updateEnq();
                break;
            case 0x06:                     // Packet Acknowledgement indication
                updateFlagCount = 0;
                emit updateAck();
                break;
            case 0x10:                      // Firmware update start indication
                updateFlagCount = 0;
                emit updateDLE();
                break;
            case 0x15:                      // Packet Negative Acknowledgement indication
                updateFlagCount = 0;
                emit updateNak();
                break;
            case 0x18:                       // Firmware update cancel indication
                updateFlagCount = 0;
                emit updateCan();
                break;
            case 'C':                        // CPU update mode indication
                if((bootloaderActive == false) && (++updateFlagCount == 3))
                {
                    bootloaderActive = true;
                    emit updateCpuUpdateMode();
                }
                break;
            default:
                updateFlagCount = 0;
				// Simple check if scanner is doing CCDUMP
                if(data.contains(":"))
                {
                    lastCCDump.clear();
                    lastCCDump.append(*it);
                    mode = MODE_CCDUMP_START;
                }
                break;
            }
            break;
        case MODE_RESPONSE_START:   // Check command code at the beginning of the packet
            responseData.clear();
            mode = MODE_RESPONSE_DATA;
            responseData.append(*it);
            checksum = *it;
            switch (responseData.at(0))
            {
            case 'A': // Get Status
                dataLength = 17;
                break;
            case 'L': // Get LCD
                dataLength = 100;
                break;
            case 'P': // Get Power Status
                dataLength = 2;
                break;
            case 'V': // Version
                dataLength = 14;
                break;
            default:
                dataLength = -1;
                break;
            }
            break;
        case MODE_RESPONSE_DATA:       // in the data packet
            checksum += *it;
            // check the length for packets with a known length and for end of data indicatior
            if((dataLength == -1) && (*it == 0x03)) // Bootloader responses
            {
                mode = MODE_RESPONSE_DATA_END;
            }
            else if(responseData.length() == dataLength)
            {
                if(*it == 0x03)
                {
                    mode = MODE_RESPONSE_DATA_END;
                }
                else
                {
                    responseData.clear();
                    mode = MODE_WAIT_START;
                }
            }
            else
            {
                responseData.append(*it);
            }
            break;
        case MODE_RESPONSE_DATA_END:  // End of the packet
            uc = *it;
            // check the data and decode if good
            if(checksum == uc)
            {
                switch (responseData.at(0))
                {
                case 'A': // Get Status
                    lastGetStatusVal.mode = responseData.at(1);
                    lastGetStatusVal.flags = responseData.at(2);
                    lastGetStatusVal.usbPower = (responseData.at(4) & 0x80)?true:false;
                    lastGetStatusVal.battery = (responseData.at(3) | (responseData.at(4) << 8)) & 0x7FFF;
                    lastGetStatusVal.rssi = (responseData.at(5) | (responseData.at(6) << 8));
                    lastGetStatusVal.zeromatic = (responseData.at(7) | (responseData.at(8) << 8));
                    lastGetStatusVal.rLed = responseData.at(9);
                    lastGetStatusVal.gLed = responseData.at(10);
                    lastGetStatusVal.bLed = responseData.at(11);
                    lastGetStatusVal.frequency = responseData.at(12) | (responseData.at(13) << 8) ||
                                                (responseData.at(14) << 16) | (responseData.at(15) << 24);
                    lastGetStatusVal.rxmode =  responseData.at(17);
                    emit updateStatus(lastGetStatusVal);
                    break;
                case 'L': // Get LCD
                    lastGetLCDVal.lcd = responseData.mid(1,97);
                    lastGetLCDVal.icons = responseData.right(3);
                    emit updateLCD(lastGetLCDVal);
                    break;
                case 'P': // Get Power Status
                    lastPowerStatusVal = (responseData.at(1))?true:false;
                    emit updatePowerStatus(lastPowerStatusVal);
                    break;
                case 'V': // Version
                    lastVersionVal.model = responseData.mid(2, 8);
                    uc = responseData.at(10);
                    switch(uc)
                    {
                    case 0:
                        lastVersionVal.ver1 = QString("Boot N/A");
                        break;
                    case 255:
                        lastVersionVal.ver1 = QString("Boot Erased");
                        break;
                    default:
                        lastVersionVal.ver1 = QString("Boot %1.%2").arg(uc >> 4, 0, 16).arg(uc & 0xF, 0, 16);
                        break;
                    }
                    uc = responseData.at(11);
                    switch(uc)
                    {
                    case 0:
                        lastVersionVal.ver2 = QString("CPU N/A");
                        break;
                    case 255:
                        lastVersionVal.ver2 = QString("CPU Erased");
                        break;
                    default:
                        lastVersionVal.ver2 = QString("CPU %1.%2").arg(uc >> 4, 0, 16).arg(uc & 0xF, 0, 16);
                        break;
                    }
                    uc = responseData.at(12);
                    switch(uc)
                    {
                    case 0:
                        lastVersionVal.ver3 = QString("DSP N/A");
                        break;
                    case 255:
                        lastVersionVal.ver3 = QString("DSP Erased");
                        break;
                    default:
                        lastVersionVal.ver3 = QString("DSP %1.%2").arg(uc >> 4, 0, 16).arg(uc & 0xF, 0, 16);
                        break;
                    }
                    uc = responseData.at(13);
                    switch(uc)
                    {
                    case 0:
                        lastVersionVal.ver4 = QString("Voc N/A");
                        break;
                    case 255:
                        lastVersionVal.ver4 = QString("Voc Erased");
                        break;
                    default:
                        lastVersionVal.ver4 = QString("Voc %1.%2").arg(uc >> 4, 0, 16).arg(uc & 0xF, 0, 16);
                        break;
                    }
                    emit updateVersion(lastVersionVal);
                    break;
                default:
                    // bootloader only returns version information
                    if(bootloaderActive)
                    {
                        lastVersionVal.model.clear();
                        if((responseData.at(0) == QLatin1Char('F')) && (responseData.at(0) == responseData.at(1)))
                                lastVersionVal.ver1 = QString("Boot Erased"); // Impossible case
                        else
                                lastVersionVal.ver1 = QString("Boot %1.%2").arg(responseData.at(0)).arg(responseData.at(1));

                         if((responseData.at(2) == QLatin1Char('F')) && (responseData.at(2) == responseData.at(3)))
                                lastVersionVal.ver2 = QString("CPU Erased");
                         else
                                lastVersionVal.ver2 = QString("CPU %1.%2").arg(responseData.at(2)).arg(responseData.at(3));
                         lastVersionVal.ver3.clear();
                         lastVersionVal.ver4.clear();
                         emit updateVersion(lastVersionVal);
                         // bootloader expects ACK or NAK
                         sendAck();
                    }
                    break;
                }
            }
            else // bad checksum so clear
            {
                responseData.clear();
                // bootloader expects ACK or NAK
                if(bootloaderActive)
                {
                    sendNak();
                }
            }
            mode = MODE_WAIT_START;
            break;
        case MODE_CCDUMP_START:                 // Start of CCDUMP
            lastCCDump.append(*it);
            mode = MODE_CCDUMP_DATA;
            break;
        case MODE_CCDUMP_DATA:                  // CCDump continues
            lastCCDump.append(*it);
            if(*it == 0x0a)
            {
                emit updateCCDump(lastCCDump);
                mode = MODE_WAIT_START;
            }
            break;
        }

    }
}
