
//===-- qlogo/console.cpp - Console class implementation -------*- C++ -*-===//
//
// This file is part of QLogo.
//
// QLogo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// QLogo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QLogo.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation of the Console class, which is the
/// text portion of the user interface.
///
//===----------------------------------------------------------------------===//

#include "console.h"
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QTextBlock>

#include CONTROLLER_HEADER

Console::Console(QWidget *parent) : QTextEdit(parent) {
  keyQueueHasChars = false;
  textFormat.setForeground(QBrush(QWidget::palette().color(QPalette::Text)));
  textFormat.setBackground(QBrush(QWidget::palette().color(QPalette::Base)));
}

Console::~Console() {}

void Console::keyPressEvent(QKeyEvent *event) {
  QString text = event->text();

  switch (inputMode) {
    case lineMode:
      processLineModeKeyPressEvent(event);
      break;
    case charMode:
      processCharModeKeyPressEvent(event);
      break;
    case inactiveMode:
      if (event->matches(QKeySequence::Quit)) {
          mainController()->addEventToQueue(toplevelEvent);
        } else if (event->matches(QKeySequence::Close)) {
          mainController()->addEventToQueue(pauseEvent);
        } else if (text != "") {
          keyQueue.push_back(text);
          keyQueueHasChars = true;
          mainController()->addEventToQueue(characterEvent);
        }
    }
}

void Console::mousePressEvent(QMouseEvent *e) {
  QTextEdit::mousePressEvent(e);
  checkCursor();
}

void Console::printString(const QString &text) {
  QTextCursor tc = textCursor();
  QStringList stringList = text.split(escapeChar);
  bool isEscaped = false;
  for (auto i = stringList.begin(); i != stringList.end(); ++i) {
    if (isEscaped) {
      switch ((*i)[0].toLatin1()) {
      case C_STANDOUT: {
        QBrush bg = textFormat.background();
        textFormat.setBackground(textFormat.foreground());
        textFormat.setForeground(bg);
        break;
      }
      case C_SET_TEXT_SIZE: {
        QStringRef sizeStringref = i->rightRef(i->size() - 1);
        double pointSize = sizeStringref.toDouble();
        if (pointSize > 0) {
          QFont f = textFormat.font();
          f.setPointSize(pointSize);
          textFormat.setFont(f);
        }
        break;
      }
      case C_SET_CURSOR_POS: {
        QStringRef rowcolStringref = i->rightRef(i->size() - 1);
        QVector<QStringRef> ary = rowcolStringref.split(C_DELIM);
        if (ary.size() == 2) {
          bool rowOK, colOK;
          int row = ary[0].toInt(&rowOK);
          int col = ary[1].toInt(&colOK);
          if (rowOK && colOK)
            moveCursorToPos(row, col);
        }
        break;
      }
      case C_SET_TEXT_COLOR: {
        QStringRef rowcolStringref = i->rightRef(i->size() - 1);
        QVector<QStringRef> ary = rowcolStringref.split(C_DELIM);
        if (ary.size() == 2) {
          textFormat.setForeground(QBrush(QColor(ary[0].toString())));
          textFormat.setBackground(QBrush(QColor(ary[1].toString())));
        }
        break;
      }
      case C_CLEAR_TEXT: {
        QTextEdit::clear();
        break;
      }
      case C_SET_FONT: {
        QString fontNameString = i->right(i->size() - 1);
        QFont f = textFormat.font();
        f.setFamily(fontNameString);
        textFormat.setFont(f);
        break;
      }
      default:
        break;
      }
    } else {
      tc.setCharFormat(textFormat);
      tc.insertText(*i);
    }
    isEscaped = !isEscaped;
  }
  ensureCursorVisible();
}

void Console::requestCharacter(void) {
  if (keyQueue.size() > 0) {
    QString key = keyQueue.left(1);
    keyQueue = keyQueue.right(keyQueue.size() - 1);
    if (keyQueue.size() == 0)
      keyQueueHasChars = false;
    mainController()->receiveString(key);
  } else {
    inputMode = charMode;
  }
}

void Console::requestLineWithPrompt(const QString &prompt) {
  moveCursor(QTextCursor::End);
  printString(prompt);
  inputMode = lineMode;
  beginningOfLine = textCursor().position();
  dumpNextLineFromQueue();
}

void Console::checkCursor() {
  QTextCursor tc = textCursor();
  if (tc.position() < beginningOfLine) {
    tc.setPosition(beginningOfLine);
    setTextCursor(tc);
  }
}

void Console::moveCursorToPos(int row, int col) {
  int countOfRows = document()->blockCount();
  while (countOfRows <= row) {
    moveCursor(QTextCursor::End);
    textCursor().insertBlock();
    ++countOfRows;
  }

  QTextBlock line = document()->findBlockByNumber(row);
  int countOfCols = line.length();

  QTextCursor tc = textCursor();
  if (countOfCols <= col) {
    tc.setPosition(line.position());
    tc.movePosition(QTextCursor::EndOfBlock);
    QString fill = QString(col - countOfCols + 1, QChar(' '));
    tc.insertText(fill);
    tc.movePosition(QTextCursor::EndOfBlock);
  } else {
    tc.setPosition(line.position() + col);
  }
  setTextCursor(tc);
}

void Console::getCursorPos(int &row, int &col) {
  QTextCursor tc = textCursor();
  row = tc.blockNumber();
  col = tc.positionInBlock();
}

void Console::processCharModeKeyPressEvent(QKeyEvent *event) {
    // If it's a paste event, pass it through so we can get the text
    if (event->matches(QKeySequence::Paste)) {
      QTextEdit::keyPressEvent(event);
      if (keyQueue.size() > 0) {
        QString text = keyQueue.left(1);
        keyQueue = keyQueue.right(keyQueue.size() - 1);
        keyQueueHasChars = (keyQueue.size() > 0);
        inputMode = inactiveMode;
        mainController()->receiveString(text);
        }
    } else if (event->matches(QKeySequence::Quit)) {
        mainController()->receiveString(toplevelString);
    } else if (event->matches(QKeySequence::Close)) {
        mainController()->receiveString(pauseString);
      } else {
        QString text = event->text();
        if (text != "") {
            inputMode = inactiveMode;
            mainController()->receiveString(text);
          }
      }
}

void Console::dumpNextLineFromQueue() {
  if (keyQueue.size() > 0) {
    int loc = 0;
    while ((loc < keyQueue.size()) && (keyQueue[loc] != '\n')) {
      ++loc;
    }

    moveCursor(QTextCursor::End);
    textCursor().insertText(keyQueue.left(loc));
    keyQueue = keyQueue.right(keyQueue.size() - loc);
    moveCursor(QTextCursor::End);
    ensureCursorVisible();
    if ((keyQueue.size() > 0) && (keyQueue[0] == '\n')) {
      inputMode = inactiveMode;
      QString field = toPlainText();
      QString line = field.right(field.size() - beginningOfLine);
      textCursor().insertBlock();
      keyQueue = keyQueue.right(keyQueue.size() - 1);
      keyQueueHasChars = (keyQueue.size() > 0);
      mainController()->receiveString(line);
    }
  }
}

void Console::processLineModeKeyPressEvent(QKeyEvent *event) {
  checkCursor();

  if (event->matches(QKeySequence::Quit)) {
      mainController()->receiveString(toplevelString);
      return;
    } else if (event->matches(QKeySequence::Close)) {
      mainController()->receiveString(pauseString);
      return;
    } else if (event->matches(QKeySequence::Paste)) {
      QTextEdit::keyPressEvent(event);
      dumpNextLineFromQueue();
      return;
    }
  int key = event->key();
  if ((key == Qt::Key_Return) || (key == Qt::Key_Enter)) {
    inputMode = inactiveMode;
    QString field = toPlainText();
    QString line = field.right(field.size() - beginningOfLine);
    moveCursor(QTextCursor::End);
    textCursor().insertBlock();
    mainController()->receiveString(line);
    return;
  }
  // Ignore delete if at beginning of line
  if ((key == Qt::Key_Backspace) &&
      (textCursor().position() <= beginningOfLine))
    return;

  QTextEdit::keyPressEvent(event);
  checkCursor();
}

bool Console::charsInQueue() { return keyQueueHasChars; }

void Console::insertFromMimeData(const QMimeData *source) {
  keyQueue += source->text();
}
