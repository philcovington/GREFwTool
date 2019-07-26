/* display.h - A simple message display class

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
#ifndef DISPLAY_H
#define DISPLAY_H

#include <QTextEdit>

class Display : public QTextEdit
{
    Q_OBJECT

public:
    explicit Display(QWidget *parent = 0);

    void putMessage(const QString &text);
    void putTxBytes(const QString &text);
    void putRxBytes(const QString &text);
    void putError(const QString &text);
    void putHtml(const QString &html);

};

#endif // DISPLAY_H
