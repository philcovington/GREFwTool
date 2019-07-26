/* webdownloader.cpp - a basic website file download class
	        This is a simple interface wrapper around QNetworkAccessManager

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
#include "include/webdownloader.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QFile>

/* Constructor
*/
WebDownloader::WebDownloader(QObject *parent) :
    QObject(parent)
{
    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyDone(QNetworkReply*)));

}
/* Destructor
*/
WebDownloader::~WebDownloader()
{
    delete manager;

}
/* doDownload - start download of the remoteFile that is saved to localFile
*/
void WebDownloader::doDownload(QUrl &remoteFile, QString &localFile)
{
    QNetworkRequest request(remoteFile);
    if (manager == nullptr)
        return;
    localFileName = localFile;
    manager->get(request);
}
/* replyDone - process a completed reply and save it to the local file
*/
void WebDownloader::replyDone(QNetworkReply *reply)
{
    if(reply->error())
    {
        emit downloadError();
    }
    else
    {
        QFile *file = new QFile(localFileName);
        if(file != nullptr)
        {
            if(file->open(QFile::WriteOnly))
            {
                file->write(reply->readAll());
                file->flush();
                file->close();
            }
            delete file;
        }
        emit downloadComplete();
    }
    reply->deleteLater();
}
