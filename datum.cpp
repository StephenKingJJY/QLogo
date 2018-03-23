
//===-- qlogo/datum.cpp - Datum class and subclasses implementation -------*-
// C++ -*-===//
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
/// This file contains the implementation of the Datum class and its subclasses,
/// Word, List, and Array, which are the data units of QLogo. This file also
/// contains the implementation of DatumP, a pointer to a Datum.
///
//===----------------------------------------------------------------------===//

#include "datum.h"
#include <qdebug.h>

int dc = 0;
int countOfNodes = 0;
int maxCountOfNodes = 0;

QList<void *> visited;
QList<void *> otherVisited;

DatumP nodes() {
  int a = countOfNodes;
  int b = maxCountOfNodes;

  maxCountOfNodes = countOfNodes;

  List *retval = new List;
  retval->append(DatumP(new Word(a)));
  retval->append(DatumP(new Word(b)));
  return DatumP(retval);
}

QChar rawToChar(const QChar &src) {
  const ushort rawToAsciiMap[] = {
      2,  58, 3,  32, 4,  9,  5,  10, 6,  40,  11, 63,  14, 43, 15, 126,
      16, 41, 17, 91, 18, 93, 19, 45, 20, 42,  21, 47,  22, 61, 23, 60,
      24, 62, 25, 34, 26, 92, 28, 59, 29, 124, 30, 123, 31, 125};
  ushort v = src.unicode();
  if (v >= 32)
    return src;
  for (const ushort *i = rawToAsciiMap; *i <= v; i += 2) {
    if (*i == v) {
      return QChar(*(i + 1));
    }
  }
  return src;
}

void rawToChar(QString &src) {
  for (int i = 0; i < src.size(); ++i) {
    QChar s = src[i];
    QChar d = rawToChar(s);
    if (s != d)
      src[i] = d;
  }
}

QChar charToRaw(const QChar &src) {
  const ushort asciiToRawMap[] = {
      126, 15, 125, 31, 124, 29, 123, 30, 93, 18, 92, 26, 91, 17, 63, 11,
      62,  24, 61,  22, 60,  23, 59,  28, 58, 2,  47, 21, 45, 19, 43, 14,
      42,  20, 41,  16, 40,  6,  34,  25, 32, 3,  10, 5,  9,  4,  0,  0};
  ushort v = src.unicode();
  for (const ushort *i = asciiToRawMap; *i >= v; i += 2) {
    if (*i == v) {
      return QChar(*(i + 1));
    }
  }
  return src;
}

Word trueWord("true", false, false);
Word falseWord("false", false, false);

Datum::Datum() {
  retainCount = 0;
  isMutable = true;
  ++countOfNodes;
  if (countOfNodes > maxCountOfNodes)
    maxCountOfNodes = countOfNodes;
}

Datum::~Datum() { --countOfNodes; }

void Datum::setImmutable() { isMutable = false; }

QString Datum::printValue(bool, int, int) { return name(); }

QString Datum::name(void) {
  static QString retval("nothing");
  return retval;
}

QString Datum::showValue(bool, int, int) { return name(); }

bool Datum::isEqual(DatumP other, bool) {
  return (other.isa() == Datum::noType);
}

DatumP Datum::first() {
  Q_ASSERT(false);
  return nothing;
}

DatumP Datum::last() {
  Q_ASSERT(false);
  return nothing;
}

DatumP Datum::butlast() {
  Q_ASSERT(false);
  return nothing;
}

DatumP Datum::butfirst() {
  Q_ASSERT(false);
  return nothing;
}

void Datum::setItem(int, DatumP) { Q_ASSERT(false); }

void Datum::setButfirstItem(DatumP) { Q_ASSERT(false); }

void Datum::setFirstItem(DatumP) { Q_ASSERT(false); }

bool Datum::containsDatum(DatumP, bool) {
  Q_ASSERT(false);
  return false;
}

bool Datum::isMember(DatumP, bool) {
  Q_ASSERT(false);
  return false;
}

DatumP Datum::fromMember(DatumP, bool) {
  Q_ASSERT(false);
  return nothing;
}

DatumP Datum::datumAtIndex(int) {
  Q_ASSERT(false);
  return nothing;
}

bool Datum::isIndexInRange(int) {
  Q_ASSERT(false);
  return false;
}

Datum &Datum::operator=(const Datum &) {
  Q_ASSERT(false);
  return *this;
}

Datum::DatumType Datum::isa() { return noType; }

int Datum::size() {
  Q_ASSERT(false);
  return 0;
}

/******************************************
 *
 * DatumP
 *
 ******************************************/

DatumP::DatumP() { d = &notADatum; }

DatumP::DatumP(Datum *other) {
  d = other;
  if (d) {
    d->retain();
  }
}

DatumP::DatumP(const DatumP &other) noexcept {
  d = other.d;
  if (d) {
    d->retain();
  }
}

DatumP::DatumP(bool b) { d = b ? &trueWord : &falseWord; }

void DatumP::destroy() {
  if (d != &notADatum) {
    d->release();
    if (d->shouldDelete()) {
      delete d;
    }
  }
}

DatumP::~DatumP() { destroy(); }

DatumP &DatumP::operator=(const DatumP &other) noexcept {
  if (&other != this) {
    destroy();
    d = other.d;
    if (d) {
      d->retain();
    }
  }
  return *this;
}

DatumP &DatumP::operator=(DatumP *other) noexcept {
  if (other != this) {
    destroy();
    d = other->d;
    d->retain();
  }
  return *this;
}

bool DatumP::operator==(DatumP *other) { return d == other->d; }

bool DatumP::operator==(const DatumP &other) { return d == other.d; }

bool DatumP::operator!=(DatumP *other) { return d != other->d; }

bool DatumP::operator!=(const DatumP &other) { return d != other.d; }

// This is true IFF EQUALP is true
bool DatumP::isEqual(DatumP other, bool ignoreCase) {
  if (d->isa() != other.isa())
    return false;
  if (d == other.d)
    return true;
  return d->isEqual(other, ignoreCase);
}

bool DatumP::isDotEqual(DatumP other) { return (d == other.d); }

bool DatumP::isASTNode() { return d->isa() == Datum::astnodeType; }

bool DatumP::isList() { return d->isa() == Datum::listType; }

bool DatumP::isArray() { return d->isa() == Datum::arrayType; }

bool DatumP::isWord() { return d->isa() == Datum::wordType; }

bool DatumP::isError() { return d->isa() == Datum::errorType; }

Word *DatumP::wordValue() {
  Q_ASSERT(d->isa() == Datum::wordType);
  return (Word *)d;
}

List *DatumP::listValue() {
  if (d->isa() != Datum::listType) {
    qDebug() << "Hello";
  }
  Q_ASSERT(d->isa() == Datum::listType);
  return (List *)d;
}

ListNode *DatumP::listNodeValue() {
  if (d->isa() != Datum::listNodeType) {
    qDebug() << "Hello";
  }
  Q_ASSERT(d->isa() == Datum::listNodeType);
  return (ListNode *)d;
}

Array *DatumP::arrayValue() {
  Q_ASSERT(d->isa() == Datum::arrayType);
  return (Array *)d;
}

Procedure *DatumP::procedureValue() {
  if (d->isa() != Datum::procedureType) {
    qDebug() << "Hello";
  }
  Q_ASSERT(d->isa() == Datum::procedureType);
  return (Procedure *)d;
}

ASTNode *DatumP::astnodeValue() {
  if (d->isa() != Datum::astnodeType) {
    qDebug() << "Error here";
  }
  return (ASTNode *)d;
}

Error *DatumP::errorValue() {
  Q_ASSERT(d->isa() == Datum::errorType);
  return (Error *)d;
}

Datum::DatumType DatumP::isa() { return d->isa(); }

QString DatumP::printValue(bool fullPrintp, int printDepthLimit,
                           int printWidthLimit) {
  return d->printValue(fullPrintp, printDepthLimit, printWidthLimit);
}

QString DatumP::showValue(bool fullPrintp, int printDepthLimit,
                          int printWidthLimit) {
  return d->showValue(fullPrintp, printDepthLimit, printWidthLimit);
}

/******************************************
 *
 * ASTNode
 *
 ******************************************/

void ASTNode::addChild(DatumP aChild) { children.push_back(aChild); }

int ASTNode::countOfChildren() { return (int)children.size(); }

DatumP ASTNode::childAtIndex(unsigned index) { return children.at(index); }

ASTNode::ASTNode(DatumP aNodeName) {
  nodeName = aNodeName;
}

ASTNode::ASTNode(const char *aNodeName) {
  nodeName = DatumP(new Word(aNodeName));
}

ASTNode::~ASTNode() {
}

Datum::DatumType ASTNode::isa() { return astnodeType; }

QString ASTNode::name(void) {
  static const QString retval("ASTNode");
  return retval;
}

QString ASTNode::printValue(bool, int, int) {
  QString retval = QString("( %1").arg(nodeName.showValue());
  for (auto &iter : children) {
    retval.append(QString(" %2").arg(iter.showValue()));
  }
  retval.append(" )");
  return retval;
}

QString ASTNode::showValue(bool, int, int) { return printValue(); }

DatumP ASTNode::first() { return nothing; }

/******************************************
 *
 * Word
 *
 ******************************************/

Word::Word() { dirtyFlag = stringIsDirty; }

Word::Word(const QString other, bool aIsForeverSpecial, bool canBeDestroyed) {
  dirtyFlag = stringIsDirty;
  isForeverSpecial = aIsForeverSpecial;
  isDestroyable = canBeDestroyed;

  rawString = other;
  printableString = rawString;
  for (int i = 0; i < printableString.size(); ++i) {
    QChar s = printableString[i];
    QChar d = rawToChar(s);
    if (s != d)
      printableString[i] = d;
  }
}

Word::Word(double other) {
  number = other;
  dirtyFlag = numberIsDirty;
}

Datum::DatumType Word::isa() { return wordType; }

QString Word::name() {
  static QString retval("Word");
  return retval;
}

Word::~Word() {}

QString Word::keyValue() {
  if (dirtyFlag == numberIsDirty)
    rawValue();
  if ((keyString.size() == 0) && (printableString.size() != 0)) {
    keyString = printableString;
    for (int i = 0; i < keyString.size(); ++i) {
      QChar s = keyString[i];
      QChar d = s.toUpper();
      if (s != d)
        keyString[i] = d;
    }
  }
  return keyString;
}

QString Word::rawValue() {
  if (dirtyFlag == numberIsDirty) {
    rawString.setNum(number);
    printableString = rawString;
    dirtyFlag = allClean;
  }
  return rawString;
}

double Word::numberValue() {
  if (dirtyFlag == stringIsDirty) {
    number = printableString.toDouble(&numberConversionSucceeded);
    if (numberConversionSucceeded)
      dirtyFlag = allClean;
  }
  return number;
}

bool Word::didNumberConversionSucceed() {
  return numberConversionSucceeded || (dirtyFlag != stringIsDirty);
}

QString Word::printValue(bool fullPrintp, int printDepthLimit,
                         int printWidthLimit) {
  if ((dirtyFlag == numberIsDirty) || (dirtyFlag == allClean))
    return rawValue();
  if (!fullPrintp && (printDepthLimit != 0) && (printWidthLimit < 0))
    return printableString;
  if (printDepthLimit == 0)
    return "...";
  QChar s, c;
  if (!fullPrintp) {
    if ((printWidthLimit >= 0) && (printWidthLimit <= 10))
      printWidthLimit = 10;
    if ((printWidthLimit >= 0) && (printableString.size() > printWidthLimit))
      return printableString.left(printWidthLimit) + "...";
    return printableString;
  }
  QString temp = rawString;
  QString retval;
  if (temp.size() == 0)
    return "||";
  bool shouldShowBars = false;
  for (int i = 0; i < temp.size(); ++i) {
    QChar t = temp[i];
    if (t < ' ') {
      shouldShowBars = true;
      break;
    }
  }
  for (int i = 0; i < temp.size(); ++i) {
    s = temp[i];
    if (shouldShowBars) {
      s = rawToChar(s);
    } else {
      c = charToRaw(s);
      if (s != c) {
        retval.append('\\');
      }
    }
    retval.append(s);
  }
  if (shouldShowBars) {
    retval.prepend('|');
    retval.append('|');
  }
  return retval;
}

QString Word::showValue(bool fullPrintp, int printDepthLimit,
                        int printWidthLimit) {
  return printValue(fullPrintp, printDepthLimit, printWidthLimit);
}

int Word::size() {
  rawValue();
  return rawString.size();
}

bool Word::isEqual(DatumP other, bool ignoreCase) {
  if (dirtyFlag != stringIsDirty) {
    bool answer = (number == other.wordValue()->numberValue());
    if (!other.wordValue()->didNumberConversionSucceed())
      return false;
    return answer;
  }
  if (other.wordValue()->dirtyFlag != stringIsDirty) {
    bool answer = (numberValue() == other.wordValue()->numberValue());
    if (!didNumberConversionSucceed())
      return false;
    return answer;
  }
  if (ignoreCase) {
    return rawValue().toUpper() == other.wordValue()->rawValue().toUpper();
  }
  return rawValue() == other.wordValue()->rawValue();
}

bool Word::isIndexInRange(int anIndex) {
  --anIndex;
  rawValue();
  return ((anIndex >= 0) && (anIndex < rawString.size()));
}

DatumP Word::datumAtIndex(int anIndex) {
  Q_ASSERT(isIndexInRange(anIndex));
  --anIndex;
  return DatumP(new Word(rawString.mid(anIndex, 1)));
}

bool Word::containsDatum(DatumP aDatum, bool ignoreCase) {
  if (!aDatum.isWord())
    return false;
  rawValue();
  Qt::CaseSensitivity cs = ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive;
  return rawString.contains(aDatum.wordValue()->rawValue(), cs);
}

bool Word::isMember(DatumP aDatum, bool ignoreCase) {
  return containsDatum(aDatum, ignoreCase);
}

DatumP Word::fromMember(DatumP aDatum, bool ignoreCase) {
  rawValue();
  Qt::CaseSensitivity cs = ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive;
  const QString &searchString = aDatum.wordValue()->rawValue();
  int pos = rawString.indexOf(searchString, 0, cs);
  QString retval;
  if (pos >= 0) {
    retval = rawString.right(rawString.size() - pos);
  }
  return DatumP(new Word(retval));
}

DatumP Word::first() {
  rawValue();
  Q_ASSERT(rawString.size() > 0);
  return DatumP(new Word(QString(rawString[0])));
}

DatumP Word::last() {
  rawValue();
  Q_ASSERT(rawString.size() > 0);
  return DatumP(new Word(QString(rawString[rawString.size() - 1])));
}

DatumP Word::butlast() {
  const QString &w = rawValue();
  Word *retval = new Word(w.left(w.size() - 1));
  return DatumP(retval);
}

DatumP Word::butfirst() {
  rawValue();
  Q_ASSERT(rawString.size() > 0);
  return DatumP(new Word(rawString.right(rawString.size() - 1)));
}

/******************************************
 *
 * List
 *
 ******************************************/

List::List() {
    astParseTimeStamp = 0;
    listSize = 0;
}

List::List(Array *source) {
  astParseTimeStamp = 0;
  listSize = source->size();
  auto aryIter = source->newIterator();
  DatumP prev;
  if (aryIter.elementExists()) {
      head = new ListNode;
      head.listNodeValue()->item = aryIter.element();
      prev = head;
  }
  while (aryIter.elementExists()) {
      ListNode *newNode = new ListNode;
      prev.listNodeValue()->next = newNode;
      newNode->item = aryIter.element();
      prev = newNode;
  }
}

List::~List() {}

List::List(List *source) {
  astParseTimeStamp = 0;
  head = source->head;
  listSize = source->size();
}

Datum::DatumType List::isa() { return listType; }

QString List::name() {
  static const QString retval("List");
  return retval;
}

QString List::printValue(bool fullPrintp, int printDepthLimit,
                         int printWidthLimit) {
  DatumP iter = head;
  if (iter == nothing) {
    return "";
  }
  if ((printDepthLimit == 0) || (printWidthLimit == 0)) {
    return "...";
  }
  int printWidth = printWidthLimit - 1;
  QString retval = iter.listNodeValue()->item.showValue(fullPrintp, printDepthLimit - 1, printWidthLimit);
  while (iter.listNodeValue()->next != nothing) {
      iter = iter.listNodeValue()->next;
    retval.append(QString(" "));
    if (printWidth == 0) {
      retval.append("...");
      break;
    }
    retval.append(
        iter.listNodeValue()->item.showValue(fullPrintp, printDepthLimit - 1, printWidthLimit));
    --printWidth;
  }
  return retval;
}

QString List::showValue(bool fullPrintp, int printDepthLimit,
                        int printWidthLimit) {
  if (!visited.contains(this)) {
    visited.push_back(this);
    QString retval = "[";
    retval.append(printValue(fullPrintp, printDepthLimit, printWidthLimit));
    retval.append("]");
    visited.removeOne(this);
    return retval;
  }
  return "...";
}

bool List::isEqual(DatumP other, bool ignoreCase) {
  ListIterator iter;
  ListIterator otherIter;
  List *o = other.listValue();
  int myIndex = visited.indexOf(this);
  int otherIndex = otherVisited.indexOf(o);
  if (myIndex != otherIndex)
    goto exit_false;

  if (myIndex > -1)
    return true;

  if (size() != o->size())
    goto exit_false;

  iter = newIterator();
  otherIter = o->newIterator();
  visited.push_back(this);
  otherVisited.push_back(o);

  while (iter.elementExists()) {
    DatumP value = iter.element();
    DatumP otherValue = otherIter.element();
    if (!value.isEqual(otherValue, ignoreCase))
      goto exit_false;
  }

  visited.pop_back();
  otherVisited.pop_back();
  return true;

exit_false:
  visited.clear();
  otherVisited.clear();
  return false;
}

DatumP List::first() {
  Q_ASSERT(head != nothing);
  return head.listNodeValue()->item;
}

bool List::isIndexInRange(int anIndex) {
    return (anIndex >= 1) && (anIndex <= listSize);
}

void List::setItem(int anIndex, DatumP aValue) {
    DatumP ptr = head;
    while (anIndex > 1) {
        --anIndex;
        ptr = ptr.listNodeValue()->next;
    }
  ptr.listNodeValue()->item = aValue;
  astParseTimeStamp = 0;
}

void List::setButfirstItem(DatumP aValue) {
  Q_ASSERT(head != nothing);
  Q_ASSERT(aValue.isList());
    head.listNodeValue()->next = aValue.listValue()->head;
    astParseTimeStamp = 0;
    listSize = aValue.listValue()->size() + 1;
}

void List::setFirstItem(DatumP aValue) {
    Q_ASSERT(head != nothing);
    head.listNodeValue()->item = aValue;
  astParseTimeStamp = 0;
}

bool List::containsDatum(DatumP aDatum, bool ignoreCase) {
    ListIterator iter = newIterator();
    while (iter.elementExists()) {
        DatumP e = iter.element();
        if (e == aDatum)
            return true;
        if (e.datumValue()->containsDatum(aDatum, ignoreCase))
          return true;
    }
  return false;
}

bool List::isMember(DatumP aDatum, bool ignoreCase) {
    ListIterator iter = newIterator();
    while (iter.elementExists()) {
        if (aDatum.isEqual(iter.element(), ignoreCase))
            return true;
    }
  return false;
}

DatumP List::fromMember(DatumP aDatum, bool ignoreCase) {
  List *retval = new List;
  DatumP ptr = head;
  while (ptr != nothing) {
      DatumP e = ptr.listNodeValue()->item;
      if (e.isEqual(aDatum, ignoreCase)) {
          retval->head = ptr;
          break;
      }
      ptr = ptr.listNodeValue()->next;
  }
  retval->setListSize();
  return DatumP(retval);
}

DatumP List::datumAtIndex(int anIndex) {
  Q_ASSERT(isIndexInRange(anIndex));
    DatumP ptr = head;
    while (anIndex > 1) {
        --anIndex;
        ptr = ptr.listNodeValue()->next;
    }
  return ptr.listNodeValue()->item;
}

DatumP List::butfirst() {
    Q_ASSERT(head != nothing);
  List *retval = new List;
    retval->head = head.listNodeValue()->next;
    retval->listSize = listSize - 1;
  return DatumP(retval);
}

void List::clear() {
  head = nothing;
  listSize = 0;
  astList.clear();
  astParseTimeStamp = 0;
}

// This should NOT be used in cases where a list may be shared
void List::append(DatumP element) {
    ListNode *newNode = new ListNode;
    ++listSize;
    newNode->item = element;
    if (head == nothing) {
        head = newNode;
        return;
    }
    DatumP ptr(head);
    while (ptr.listNodeValue()->next != nothing) {
        ptr = ptr.listNodeValue()->next;
    }
    ptr.listNodeValue()->next = newNode;
  astParseTimeStamp = 0;
}

DatumP List::last() {
    Q_ASSERT(head != nothing);
    DatumP retval = head;
    while (retval.listNodeValue()->next != nothing) {
        retval = retval.listNodeValue()->next;
    }
    return retval.listNodeValue()->item;
}

DatumP List::butlast() {
  List *retval = new List;
  retval->listSize = listSize - 1;
  if (head.listNodeValue()->next != nothing) {
      DatumP src = head;
      DatumP dest;
      while (src.listNodeValue()->next != nothing) {
          ListNode *newnode = new ListNode;
          newnode->item = src.listNodeValue()->item;
          if (retval->head == nothing) {
              retval->head = newnode;
          } else {
            dest.listNodeValue()->next = newnode;
          }
          dest = newnode;
          src = src.listNodeValue()->next;
      }
  }
  return DatumP(retval);
}

void List::prepend(DatumP element) {
    ListNode *newnode = new ListNode;
    newnode->item = element;
    newnode->next = head;
    head = newnode;
    ++listSize;
  astParseTimeStamp = 0;
}

DatumP List::fput(DatumP item)
{
    ListNode *newnode = new ListNode;
    List *retval = new List;
    newnode->item = item;
    newnode->next = head;
    retval->head = newnode;
    retval->listSize = listSize + 1;
    return retval;
}

void List::setListSize()
{
    listSize = 0;
    DatumP ptr = head;
    while (ptr != nothing) {
        ++listSize;
        ptr = ptr.listNodeValue()->next;
    }
}

ListIterator List::newIterator() { return ListIterator(head); }

/******************************************
 *
 * Array
 *
 ******************************************/

Array::Array(int aOrigin, int aSize) {
  origin = aOrigin;
  array.reserve(aSize);
  for (int i = 0; i < aSize; ++i) {
    array.append(DatumP(new List));
  }
}

Array::Array(int aOrigin, List *source) {
  origin = aOrigin;
  array.reserve(source->size());
  DatumP ptr = source->head;
  while (ptr != nothing) {
      array.push_back(ptr.listNodeValue()->item);
      ptr = ptr.listNodeValue()->next;
  }
}

Array::~Array() {}

Datum::DatumType Array::isa() { return Datum::arrayType; }

QString Array::name() {
  static const QString retval("Array");
  return retval;
}

QString Array::printValue(bool fullPrintp, int printDepthLimit,
                          int printWidthLimit) {
  QString retval = "";
  auto iter = array.begin();
  if (iter == array.end()) {
    return retval;
  }
  if ((printDepthLimit == 0) || (printWidthLimit == 0)) {
    return "...";
  }
  int printWidth = printWidthLimit - 1;
  retval = iter->showValue(fullPrintp, printDepthLimit - 1, printWidthLimit);
  while (++iter != array.end()) {
    retval.append(QString(" "));
    if (printWidth == 0) {
      retval.append("...");
      break;
    }
    retval.append(
        iter->showValue(fullPrintp, printDepthLimit - 1, printWidthLimit));
    --printWidth;
  }
  return retval;
}

QString Array::showValue(bool fullPrintp, int printDepthLimit,
                         int printWidthLimit) {
  if (!visited.contains(this)) {
    visited.push_back(this);
    QString retval = "{";
    retval.append(printValue(fullPrintp, printDepthLimit, printWidthLimit));
    retval.append("}");
    visited.removeOne(this);
    return retval;
  }
  return "...";
}

bool Array::isEqual(DatumP other, bool ignoreCase) {
  ArrayIterator iter;
  ArrayIterator otherIter;
  Array *o = other.arrayValue();
  int myIndex = visited.indexOf(this);
  int otherIndex = otherVisited.indexOf(o);
  if (myIndex != otherIndex)
    goto exit_false;

  if (myIndex > -1)
    return true;

  if (size() != o->size())
    goto exit_false;

  iter = newIterator();
  otherIter = o->newIterator();
  visited.push_back(this);
  otherVisited.push_back(o);

  while (iter.elementExists()) {
    DatumP value = iter.element();
    DatumP otherValue = otherIter.element();
    if (!value.isEqual(otherValue, ignoreCase))
      goto exit_false;
  }
  visited.pop_back();
  otherVisited.pop_back();
  return true;

exit_false:
  visited.clear();
  otherVisited.clear();
  return false;
}

int Array::size() { return array.size(); }

void Array::append(DatumP value) { array.append(value); }

bool Array::isIndexInRange(int anIndex) {
  int index = anIndex - origin;
  return ((index >= 0) && (index < array.size()));
}

void Array::setItem(int anIndex, DatumP aValue) {
  int index = anIndex - origin;
  array[index] = aValue;
}

void Array::setButfirstItem(DatumP aValue) {
  Q_ASSERT(array.size() > 0);
  auto estart = array.begin();
  ++estart;
  array.erase(estart, array.end());
  array.reserve(aValue.arrayValue()->size() + 1);
  array.append(aValue.arrayValue()->array);
}

void Array::setFirstItem(DatumP aValue) { array[0] = aValue; }

bool Array::containsDatum(DatumP aDatum, bool ignoreCase) {
  for (int i = 0; i < array.size(); ++i) {
    DatumP e = array[i];
    if (e == aDatum)
      return true;
    if (e.datumValue()->containsDatum(aDatum, ignoreCase))
      return true;
  }
  return false;
}

bool Array::isMember(DatumP aDatum, bool ignoreCase) {
  for (int i = 0; i < array.size(); ++i) {
    if (array[i].isEqual(aDatum, ignoreCase))
      return true;
  }
  return false;
}

DatumP Array::fromMember(DatumP aDatum, bool ignoreCase) {
  for (int i = 0; i < array.size(); ++i) {
    if (array[i].isEqual(aDatum, ignoreCase)) {
      Array *retval = new Array(origin, 0);
      retval->array.reserve(array.size() - i);
      while (i < array.size()) {
        retval->append(array[i]);
        ++i;
      }
      return DatumP(retval);
    }
  }
  return DatumP(new Array(origin, 0));
}

DatumP Array::datumAtIndex(int anIndex) {
  int index = anIndex - origin;
  Q_ASSERT((index >= 0) && (index < array.size()));
  return array[index];
}

DatumP Array::first() { return DatumP(new Word(origin)); }

DatumP Array::last() {
  Q_ASSERT(array.size() > 0);
  return array[array.size() - 1];
}

DatumP Array::butfirst() {
  Array *retval = new Array(origin, 0);
  retval->array = array.mid(1, array.size() - 1);
  return DatumP(retval);
}

DatumP Array::butlast() {
  Array *retval = new Array(origin, 0);
  retval->array = array.mid(0, array.size() - 1);
  return DatumP(retval);
}

ArrayIterator Array::newIterator() { return ArrayIterator(&array); }

/******************************************
 *
 * Iterator
 *
 ******************************************/

DatumP Iterator::element() {
  Q_ASSERT(false);
  return DatumP();
}

bool Iterator::elementExists() {
  Q_ASSERT(false);
  return false;
}

/******************************************
 *
 * ListIterator
 *
 ******************************************/

ListIterator::ListIterator() {}

ListIterator::ListIterator(DatumP head) {
    ptr = head;
}

DatumP ListIterator::element() {
    DatumP retval = ptr.listNodeValue()->item;
    ptr = ptr.listNodeValue()->next;
    return retval;
}

bool ListIterator::elementExists() { return (ptr != nothing); }

/******************************************
 *
 * ArrayIterator
 *
 ******************************************/

ArrayIterator::ArrayIterator() {}

ArrayIterator::ArrayIterator(QVector<DatumP> *aArray) {
  arrayIter = aArray->begin();
  end = aArray->end();
}

DatumP ArrayIterator::element() { return *arrayIter++; }

bool ArrayIterator::elementExists() { return (arrayIter != end); }

/******************************************
 *
 * WordIterator
 *
 ******************************************/

WordIterator::WordIterator() {}

WordIterator::WordIterator(Word *aWord) {
  charIter = aWord->rawString.begin();
  end = aWord->rawString.end();
}

DatumP WordIterator::element() {
  const QChar &c = *charIter++;
  return DatumP(new Word(c));
}

bool WordIterator::elementExists() { return (charIter != end); }

Datum notADatum;
DatumP nothing(&notADatum);
