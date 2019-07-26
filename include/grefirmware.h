/* grefirmware.h - A simple GRE firmware container class

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
#ifndef GREFIRMWARE_H
#define GREFIRMWARE_H

#include <QObject>

class GREFirmware : public QObject
{
    Q_OBJECT
public:
    explicit GREFirmware(QObject *parent = 0);
    ~GREFirmware();
    bool openFile(QString path);
    quint8 getPlatform() { return header.platform; }
    qint32 getImageSize() { return header.imageSize; }
    qint32 getOffset() { return offset; }
    bool transcode(quint8 newPlatform);
    QByteArray &getFirstPacket();
    QByteArray &getNextPacket();

private:
    struct
    {
        quint8  platform;
        qint32  imageSize;
    } header;
    QByteArray imageData;
    QString pathInfo;
    qint32 offset;
    QByteArray headerPacket;
    QByteArray dataPacket;

};

#endif // GREFIRMWARE_H
