/* display.cpp - A simple message display class based on QTextEdit

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
#include "include/display.h"

#include <QScrollBar>

Display::Display(QWidget *parent)
    : QTextEdit(parent)
{
    setAutoFormatting(QTextEdit::AutoNone);
    setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
    setFocusPolicy(Qt::NoFocus);
    setLineWrapMode(QTextEdit::WidgetWidth);
    document()->setMaximumBlockCount(400);

}
/* putMessage - Display a normal message and move cursor to end
*/
void Display::putMessage(const QString &text)
{
    QScrollBar *bar = verticalScrollBar();
    moveCursor (QTextCursor::End);
    setTextBackgroundColor(QColor(Qt::white));
    setTextColor(QColor(Qt::black));
    setFontWeight(QFont::Normal);
    append(text);
    moveCursor (QTextCursor::End);
    bar->setValue(bar->maximum());
}
/* putTxBytes - Display serial transmit bytes with blue on white text and move cursor to end
*/
void Display::putTxBytes(const QString &text)
{
    QScrollBar *bar = verticalScrollBar();
    moveCursor (QTextCursor::End);
    setTextBackgroundColor(QColor(Qt::white));
    setTextColor(QColor(Qt::blue));
    setFontWeight(QFont::Normal);
    append(text);
    moveCursor (QTextCursor::End);
    bar->setValue(bar->maximum());
}
/* putTxBytes - Display serial receive bytes with green on white text and move cursor to end
*/
void Display::putRxBytes(const QString &text)
{
    QScrollBar *bar = verticalScrollBar();
    moveCursor (QTextCursor::End);
    setTextBackgroundColor(QColor(Qt::white));
    setTextColor(QColor(Qt::green));
    setFontWeight(QFont::Normal);
    append(text);
    moveCursor (QTextCursor::End);
    bar->setValue(bar->maximum());
}
/* putError - Display error message with red on black text and move cursor to end
*/
void Display::putError(const QString &text)
{
    QScrollBar *bar = verticalScrollBar();
    moveCursor (QTextCursor::End);
    setTextBackgroundColor(QColor(Qt::black));
    setTextColor(QColor(Qt::red));
    setFontWeight(QFont::Bold);
    append(text);
    moveCursor (QTextCursor::End);
    bar->setValue(bar->maximum());
}
/* putError - Display HTML and move cursor to end
*/
void Display::putHtml(const QString &html)
{
    moveCursor (QTextCursor::End);
    setTextBackgroundColor(QColor(Qt::white));
    setTextColor(QColor(Qt::black));
    setFontWeight(QFont::Normal);
    insertHtml(html);
    moveCursor (QTextCursor::End);
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());

}
