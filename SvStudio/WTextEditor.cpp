﻿#include "SvConfig.h"
#include "WTextEditor.h"
#include "Highlighter.h"
#include "WMain.h"

#include <QKeyEvent>
#include <QPainter>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QClipboard>
#include <QGuiApplication>

#include <QDebug>

void ScrollBarWithMarkers::paintEvent(QPaintEvent *event)
  {
  QScrollBar::paintEvent((event));
  QPainter painter(this);
  int scrollBarWidth = width();
  int x = scrollBarWidth / 5;
  int w = scrollBarWidth - x*2;
  //здесь scrollBarWidth также определяет высоту кнопок вверх и вниз (они квадратные)
  int rangeSize = height()-2*scrollBarWidth;
  auto i=0;
  foreach (auto position, mHighlightedLines) {
    double y = scrollBarWidth + position.first*rangeSize;
    double h = qMax(position.second*rangeSize, 1.0);
    if (i == mSelected)
      painter.fillRect(x, y, w, h, mSelectedMarkerColor);
    else
      painter.fillRect(x, y, w, h, mMarkerColor);
    ++i;
    }
  }

ScrollBarWithMarkers::ScrollBarWithMarkers(QWidget *parent):
  QScrollBar(parent)
  {
  mMarkerColor = QColor(Qt::green).darker(160);
  mMarkerColor.setAlpha(150);
  mSelectedMarkerColor = QColor(Qt::yellow).darker(160);
  mSelectedMarkerColor.setAlpha(150);
  }

void ScrollBarWithMarkers::setHighlights(const QList<QPair<double, double>> &positions, int selected)
  {
  mHighlightedLines = positions;
  mSelected = selected;
  update();
  }


//корень графа локальных блоков
BlockNode* mRootBlock;
//все локальные блоки для облегчения поиска
QList<BlockNode*> mAllBlocks;

ScrollBarWithMarkers* mScrollBarMarker;

int             mAutoIndentSpacesCount;   //!< количество пробелов для автоотступа

bool            mLastPressedReturn;
bool            mAutoCompleteParenthesis;

int             mDebugLine;          //!< Номер строки, в которой произведена остановка
int             mBreakLine;          //!< Номер строки, в которой ожидается остановка
QDateTime       mFileTime;           //!< Возраст файла
QListWidget    *mAutoComplete;       //!< Окно списка автозавершения
int             mLeftViewPortMargin; //!< Отступ слева для номеров строк
bool            mIsUndoAvailable;
bool            mIsRedoAvailable;
bool            mIsSelectPresent;

bool            mControlPress;       //!< Флаг нажатия кнопки Control
QString         mLink;
QTextBlock      mLinkBlock;
QString         mOverWord;           //!< Идентификатор, над которым находится курсор

QPlainTextEdit *mHelpPopUp;          //!< Окно всплывающей помощи
QTimer          mHelpTime;           //!< Таймер, обеспечивающий появление помощи к идентификатору


QList<QTextEdit::ExtraSelection> mSearchHighlight; //!< подстветка поиска

QList<QTextEdit::ExtraSelection> mParenthesisHighlight; //!< подсветка скобок

QTextEdit::ExtraSelection        mCurrentLineHighlight; //!< подстветка текущей строки

TextEditor::TextEditor(QWidget *parent) :
  QPlainTextEdit(parent),
  mRootBlock(nullptr),
  mScrollBarMarker(nullptr),
  mAutoIndentSpacesCount(2),
  mLastPressedReturn(false),
  mAutoCompleteParenthesis(true),
  mDebugLine(-1),
  mBreakLine(-1),
  mAutoComplete(nullptr),
  mIsUndoAvailable(false),
  mIsRedoAvailable(false),
  mIsSelectPresent(false),
  mControlPress(false),
  mHelpPopUp(nullptr)
  {

  mScrollBarMarker = new ScrollBarWithMarkers(nullptr);
  setVerticalScrollBar(mScrollBarMarker);

  mRootBlock = nullptr;

  //Создать окно автозавершения
  mAutoComplete = new QListWidget(this);
  mAutoComplete->hide();
  //Установить размер автозавершателя
  mAutoComplete->resize( 250, 150 );

  //Создать окно помощи
  mHelpPopUp = new QPlainTextEdit( this );
  mHelpPopUp->hide();
  //Установить размер окна помощи
  mHelpPopUp->resize( 450, 50 );

  connect( this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()) );

  setLineWrapMode(QPlainTextEdit::NoWrap);

  lineNumberArea = new LineNumberArea(this);
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
  //  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
  // connect(this, SIGNAL(textChanged()), this, SLOT(updateBlockGraph()));

  //Состояние редактора
  connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(onUndoAvailable(bool)) );
  connect(this, SIGNAL(redoAvailable(bool)), this, SLOT(onRedoAvailable(bool)) );
  connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(onSelection(bool)) );

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();

  setMouseTracking(true);
  }




TextEditor::~TextEditor()
  {
  if (mRootBlock != nullptr)
    mRootBlock->deleteLater();
  }






void TextEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);


  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int) blockBoundingRect(block).height();

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.drawText(0 - 4, top, lineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
      }

    block = block.next();
    top = bottom;
    bottom = top + (int) blockBoundingRect(block).height();
    ++blockNumber;
    }
  }






int TextEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
    }

  int space = 5 + 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

  return space;
  }





//Получить строку под курсором
QString cursorLine( QTextCursor c, int *posPtr ) {
  int pos = c.position();
  c.movePosition( QTextCursor::StartOfBlock );
  int linePos = c.position();
  c.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );
  //Вернуть позицию курсора относительно начала строки
  if( posPtr )
    *posPtr = pos - linePos;
  return c.selectedText();
  }





QChar cursorChar( QTextCursor c, int offset ) {
  int pos;
  QString line = cursorLine( c, &pos );
  return line[ pos + offset ];
  }





QString TextEditor::getWordCursorOver()
  {
  return getWordCursorOver( textCursor() );
  }




QString TextEditor::getWordCursorOver(QTextCursor cursor)
  {
  //Текущая строка
  int pos;
  QString line = cursorLine( cursor, &pos );

  //Обнаружить начало слова
  while( pos >= 1 && (line.at(pos-1).isLetterOrNumber() || line.at(pos-1) == QChar('.') || line.at(pos-1) == QChar('_')) ) pos--;

  //Первый знак должен быть буквой
  if( pos >= 0 && pos < line.length() && line.at(pos).isLetter() ) {
    //Собрать слово
    int len = 1;
    while( pos + len < line.length() && (line.at(pos + len).isLetterOrNumber() || line.at(pos + len) == QChar('.') || line.at(pos + len) == QChar('_')) )
      len++;
    return line.mid( pos, len );
    }
  return QString();
  }







QString TextEditor::getIncludeOver()
  {
  //Текущая строка
  int pos;
  QString line = cursorLine( textCursor(), &pos );

  //Обнаружить начало имени файла
  int start = line.indexOf( QChar('<') );
  int stop;
  if( start < 0 ) {
    start = line.indexOf( QChar('"') );
    if( start < 0 ) return QString();
    stop = line.indexOf( QChar('"'), start+1 );
    }
  else stop = line.indexOf( QChar('>') );
  if( stop < 0 ) return QString();

  //Между индексами start и stop находится имя файла
  return line.mid( start+1, stop - start - 1 );
  }








//Получить директорий файла
QString TextEditor::getFileDirectory()
  {
  QFileInfo info(mFilePath);
  return info.absolutePath();
  }




int TextEditor::autoIndentSpaceCount() const
  {
  return mAutoIndentSpacesCount;
  }




void TextEditor::setAutoIndentSpaceCount(int count)
  {
  mAutoIndentSpacesCount = count;
  }




bool TextEditor::autoCompleteParenthesis() const
  {
  return mAutoCompleteParenthesis;
  }




void TextEditor::setAutoCompleteParenthesis(bool autoComplete)
  {
  mAutoCompleteParenthesis = autoComplete;
  }




void TextEditor::updateHighlights()
  {
  QList<QTextEdit::ExtraSelection> extraSelections;

  extraSelections.append(mCurrentLineHighlight);

  extraSelections.append(mParenthesisHighlight);
  extraSelections.append(mSearchHighlight);

  setExtraSelections(extraSelections);
  }




int TextEditor::getLeadingSpaceCount(const QString &text)
  {
  int i =0;
  for(i=0; i<text.length(); ++i){
    if (text[i] != ' ')
      return i;
    }
  return i;
  }




BlockNode *TextEditor::getBlockForLine(int lineNum) const
  {
  auto result = mRootBlock;
  auto minRange = INT_MAX;
  foreach (auto pBlock, mAllBlocks) {
    if (pBlock->containsLine(lineNum)){
      auto blockRange = pBlock->mEndLineNumber - pBlock->mStartLineNumber;
      if (blockRange < minRange){
        minRange = blockRange;
        result = pBlock;
        }
      }
    }
  return result;
  }




BlockNode *TextEditor::getBlockForIndex(int index) const
  {
  auto result = mRootBlock;
  auto minRange = INT_MAX;
  foreach (auto pBlock, mAllBlocks) {
    if (pBlock->containsIndex(index)){
      auto blockRange = pBlock->mEndIndex - pBlock->mStartIndex;
      if (blockRange < minRange){
        minRange = blockRange;
        result = pBlock;
        }
      }
    }
  return result;
  }










void TextEditor::autoIndent()
  {
  auto currentLine = textCursor().blockNumber();
  if( mLastPressedReturn ) {
    if( getBlockForLine(currentLine)->mSingle ) {
      updateBlockGraph();
      autoIndentLine(currentLine, false);
      }
    else{
      auto prevLine =  currentLine - 1;
      if (prevLine < 0)
        prevLine = 0;
      auto prevBlock = document()->findBlockByLineNumber(prevLine);
      indentLine(currentLine, getLeadingSpaceCount(prevBlock.text()), false);
      }
    }
  else{
    updateBlockGraph();
    autoIndentLine(currentLine, false);
    }
  }




bool TextEditor::completeParenthesis()
  {
  QTextCursor tc = textCursor();
  //Весь редактируемый текст
  auto text = toPlainText();

  //Текст до точки вставки
  auto prevText = text.left( tc.position() );

  //Текст после точки вставки
  auto nextText = text.right(text.length() - tc.position());

  //Количество открытых и не закрытых скобок в предыдущем тексте
  auto prevTextOpenedParenthesis = prevText.count('{') - prevText.count('}');

  //Количество закрытых и не открытых скобок в следующем тексте
  auto nextTextClosedParenthesis = nextText.count('}') - nextText.count('{');

  //Количество вставляемых скобок
  auto insertedParenthesis = prevTextOpenedParenthesis - nextTextClosedParenthesis;

  if (insertedParenthesis > 0) {
    //insertPlainText( "\n  " );
    autoIndent();
//    insertPlainText( QString( mAutoIndentSpacesCount, QChar(' ')) );
    insertPlainText( "\n" );
    mLastPressedReturn = false;
    autoIndent();
    insertPlainText( "}" );
    QTextCursor c = textCursor();
    c.movePosition( QTextCursor::Up );
    c.movePosition( QTextCursor::EndOfLine );
    setTextCursor( c );
    }
  /* for (auto i=0; i<insertedParenthesis; ++i){
        if (i > 0)
            insertPlainText("\n");
        insertPlainText("}");
    }*/
  return insertedParenthesis > 0;
  }






void TextEditor::autoComplete()
  {
  //Проверить наличие текущего выбора
  if( mAutoComplete->currentItem() ) {
    //Вставить текущий выбор из автозавершения
    QTextCursor c = textCursor();
    QString word = getWordLeft();
    c.movePosition( QTextCursor::Left, QTextCursor::KeepAnchor, word.length() );
    c.removeSelectedText();
    c.insertText( mAutoComplete->currentItem()->text() );
    //Текущая строка
    int pos;
    QString line = cursorLine( textCursor(), &pos );
    //Проверить специальный случай ввода файла include
    if( line.simplified().startsWith(QString("#include <")) )
      c.insertText( QString(">") );
    else if( line.simplified().startsWith(QString("#include \"")) )
      c.insertText( QString("\"") );
    setTextCursor( c );
    mAutoComplete->hide();
    mHelpPopUp->hide();
    }
  }






void TextEditor::refillAutoComplete()
  {
  QStringList srcList;
  bool checkLen = true;

  //Текущая строка
  int pos;
  QString line = cursorLine( textCursor(), &pos );
  //Проверить специальный случай ввода файла include
  if( line.simplified().startsWith(QString("#include <")) || line.simplified().startsWith(QString("#include \"")) ) {
    //Показываем список файлов из директория include
    QDir dir( getFileDirectory() );
    QStringList filtr;
    filtr << QString("*.h") << QString("*.c") << QString("*.cpp") << QString("*" EXTENSION_SCRIPT );
    mAutoComplete->clear();
    srcList = QStringList( dir.entryList( filtr, QDir::Files ) );
    checkLen = false;
    }
  else {
    //Обычный идентификатор
    if( Highlighter::mCompiler ) {
      //Types
      srcList.append( Highlighter::mCompiler->mTypeList.keys() );
      //Variables
      srcList.append( Highlighter::mCompiler->mVarGlobal.mHash.keys() );
      //Functions
      srcList.append( Highlighter::mCompiler->mFunGlobal.mHash.keys() );
      //Macro names
      srcList.append( Highlighter::mCompiler->mMacroTable.keys() );
      //Key words
      srcList.append( Highlighter::mCompiler->mKeyWords.keys() );
      }
    srcList.append( QString("#define") );
    srcList.append( QString("#ifdef") );
    srcList.append( QString("#ifndef") );
    srcList.append( QString("#else") );
    srcList.append( QString("#endif") );
    srcList.append( QString("#include") );
    }

  //Проверить наличие предварительно определенного идентификатора
  QTextCursor cr = textCursor();
  cr.movePosition( QTextCursor::StartOfWord, QTextCursor::KeepAnchor );
  QString word = getWordLeft();
  QPoint p = cursorRect( cr ).bottomLeft();
  if( p.x() + 250 > width() ) {
    //Окно не влезает по ширине, выравниваем по правому краю
    p.rx() -= 250;
    }
  if( p.y() + 150 > height() ) {
    //Окно не влезает по высоте, направляем его на верх
    p.ry() = cursorRect( cr ).topLeft().y() - 155;
    }
  qDebug() << "refill" << word;

  if( word.length() < 3 && checkLen ) mAutoComplete->hide();
  else {
    QString pat("^");
    pat.append( word );
    //Перезаполняем список автозавершения
    mAutoComplete->clear();
    srcList = srcList.filter( QRegularExpression(pat, QRegularExpression::CaseInsensitiveOption) );
    if( !srcList.isEmpty() ) {
      p.rx() += mLeftViewPortMargin;
      mAutoComplete->addItems( srcList );
      mAutoComplete->setCurrentRow(0);
      //Вынести окно автозавершения в позицию начала слова
      mAutoComplete->move( p );
      mAutoComplete->show();
      }
    }
  }




QString TextEditor::getWordLeft()
  {
  //Текущая строка
  int pos;
  QString line = cursorLine( textCursor(), &pos );
  int epos = pos;

  if( pos <= 0 ) return QString();

  //Обнаружить начало слова
  while( pos >= 1 && (line.at(pos-1).isLetterOrNumber() || line.at(pos-1) == QChar('_') || line.at(pos-1) == QChar('#')) ) pos--;

  //Проверить, что это слово
  if( pos < line.length() && (line.at(pos).isLetter() || line.at(pos) == QChar('_') || line.at(pos) == QChar('#')) ) {
    //Это слово, вернуть
    return line.mid( pos, epos - pos );
    }
  return QString();
  }





//Переключить комментарии в одной строке
void TextEditor::toggleRemarkLine(QTextCursor tc)
  {
  //Получить содержимое блока, соответствующее данному курсору
  int pos;
  QString line = cursorLine( tc, &pos );
  tc.movePosition( QTextCursor::StartOfBlock );
  if( line.startsWith( QString("//") ) ) {
    //Удаляем два символа из начала строки
    tc.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, 2 );
    tc.removeSelectedText();
    }
  else {
    //Вставляем два символа // в начало строки
    tc.insertText( QString("//") );
    }
  }





//Переключить комментарий
void TextEditor::toggleRemark()
  {
  //Если есть выделение, то вставка-удаление коментирования для каждой строки
  QTextCursor tc = textCursor();
  if( tc.hasSelection() ) {
    //Получить начало и конец выделения
    int start = tc.selectionStart();
    int end = tc.selectionEnd();
    //Снять выделение
    tc.clearSelection();
    //Обеспечить, чтобы начало было меньше конца
    if( end < start ) {
      int sw = start;
      start = end;
      end = sw;
      }
    //Едем к началу выделения
    tc.setPosition( start );
    //Начало строки
    tc.movePosition( QTextCursor::StartOfBlock );

    QTextCursor tcs(tc);
    tcs.setPosition( start );
    start = tcs.blockNumber();
    tcs.setPosition( end );
    end = tcs.blockNumber();
    //Переключаем для всех строк, пока не достигнем конца выделения
    for( int i = start; i <= end; i++ ) {
      toggleRemarkLine( tc );
      tc.movePosition( QTextCursor::NextBlock );
      }
    }
  else
    //Вставка-удаление коментария только для текущей строки
    toggleRemarkLine( tc );
  }






//Реакция на нажатие F2
void TextEditor::toggleF2()
  {
  //При нажатии клавиши F2 над включаемым файлом осуществляется открытие этого файла
  //При нажатии над идентификатором осуществлять переключение к месту определения-объявления
  QString str = getIncludeOver();
  if( !str.isEmpty() ) {
    QString fname = getFileDirectory() + QString("/") + str;
    emit fileOpen( fname );
    }
  else {
    //Возможно, стоим над идентификатором
    QString ident = getWordCursorOver();
    if( !ident.isEmpty() && Highlighter::mCompiler ) {
      QString fname;
      int line;

      if( Highlighter::mCompiler->mVarGlobal.isPresent(ident) ) {
        //Выполнить переход к определению символа
        SvCompiler6::SvVariablePtr vp = Highlighter::mCompiler->mVarGlobal.getVariable(ident);
        fname = Highlighter::mCompiler->mFileTable.value(vp->mMarkDefine.mFile);
        line = vp->mMarkDefine.mLine;
//        if( fname.compare( mFilePath, Qt::CaseInsensitive ) == 0 && line - 1 == textCursor().blockNumber() ) {
//          fname = Highlighter::mCompiler->mFileTable.value(vp->mMarkFirstDeclare.mFile);
//          line = vp->mMarkFirstDeclare.mLine;
//          }
        emit jump( fname, line );
        }
      else if( Highlighter::mCompiler->mFunGlobal.isPresent(ident) ) {
        SvCompiler6::SvFunctionPtr fp = Highlighter::mCompiler->mFunGlobal.getFunction(ident);
        fname = Highlighter::mCompiler->mFileTable.value(fp->mMarkDefine.mFile);
        line = fp->mMarkDefine.mLine;
        emit jump( fname, line );
        }
      else if( Highlighter::mCompiler->mTypeList.isPresent(ident) ) {
        SvCompiler6::SvStruct *sp = Highlighter::mCompiler->mTypeList.getType(ident)->toStruct();
        if( sp ) {
          fname = Highlighter::mCompiler->mFileTable.value(sp->mDefine.mFile);
          line = sp->mDefine.mLine;
          emit jump( fname, line );
          }
        }
      }
    }
  }




const QMap<QChar, QChar> cParenthesisPairs(
{
      {'(',')'},
      {'[',']'},
      {'{','}'},
    });


QPair<int, int> TextEditor::getParenthesisBlock(const QString &text, int pos) const
  {
  if (pos < 0 || pos >= text.length())
    return QPair<int,int>(-1,-1);
  auto start = pos;
  auto end = pos;
  QChar startParenthesis;
  QMap<QChar, int> numClosed;

  QChar c;
  if (pos > 0){
    auto c = text[pos - 1];
    if (c == ')' || c == ']' || c == '}'){
      pos--;
      }
    }
  for (start = pos; start >=0; start--){
    c = text[start];
    if ((c == ')' || c == ']' || c == '}') && start != pos){
      numClosed[c]++;
      }
    if (c == '(' || c == '[' || c == '{'){
      if (numClosed[cParenthesisPairs[c]] == 0){
        startParenthesis = c;
        break;
        }
      numClosed[cParenthesisPairs[c]]--;
      }
    }

  if (start < 0){
    return QPair<int, int>(-1,-1);
    }
  else{
    QMap<QChar, int> numOpened;
    for (end = pos; end <text.count(); end++){
      c = text[end];
      if ((c == '(' || c == '[' || c == '{') && end != pos){
        numOpened[cParenthesisPairs[c]]++;
        }

      if (c == ')' || c == ']' || c == '}'){
        if (c == cParenthesisPairs[startParenthesis] && numOpened[c] == 0)
          break;
        numOpened[c]--;
        }
      }
    if (end < text.count())
      return QPair<int, int>(start, end);
    else
      return QPair<int, int>(-1,-1);
    }
  }





void TextEditor::updateLineNumberAreaWidth(int newBlockCount) {
  Q_UNUSED(newBlockCount);
  setViewportMargins( mLeftViewPortMargin = lineNumberAreaWidth(), 0, 0, 0);
  }





//подсветка текущей строки
void TextEditor::highlightCurrentLine() {
  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    QColor lineColor = QColor(Qt::yellow).lighter(160);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    mCurrentLineHighlight = selection;
    }
  else
    mCurrentLineHighlight = QTextEdit::ExtraSelection();

  // updateHighlights();
  }




//подсветка текущего блока
void TextEditor::highlightCurrentBlock()
  {
  auto text = toPlainText();
  auto cursorPosition = textCursor().position();
  auto parenthBlock = getParenthesisBlock(text, cursorPosition);

  mParenthesisHighlight.clear();
  //если нашли блок
  if (parenthBlock.first != -1){
    if (!isReadOnly()) {

      QTextEdit::ExtraSelection selection;
      QColor color = QColor(Qt::cyan);
      selection.format.setBackground(color);
      auto cursor = textCursor();
      //выбрать открывающую скобку
      cursor.setPosition(parenthBlock.first);
      cursor.setPosition(parenthBlock.first+1, QTextCursor::KeepAnchor);
      selection.cursor = cursor;

      mParenthesisHighlight.append(selection);

      color = QColor(Qt::cyan).lighter(160);
      selection.format.setBackground(color);
      cursor = textCursor();
      //выбрать закрывающую скобку
      cursor.setPosition(parenthBlock.second);
      cursor.setPosition(parenthBlock.second+1, QTextCursor::KeepAnchor);
      selection.cursor = cursor;

      mParenthesisHighlight.append(selection);
      }
    }
  //  updateHighlights();
  }






void TextEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
  }






void
TextEditor::keyPressEvent(QKeyEvent *e)
  {
  mControlPress = (e->modifiers() & Qt::ControlModifier) != 0;

  mHelpPopUp->hide();

  bool returnPressed = false;

  //При нажатии клавиши F2 над включаемым файлом осуществляется открытие этого файла
  //TODO при нажатии над идентификатором осуществлять переключение к месту определения-объявления
  if( e->key() == Qt::Key_F2 ) toggleF2();




  //Return (Enter)
  else if( e->key() == Qt::Key_Return ) {
    if( mAutoComplete->isHidden() ){
      //после стандартной обработки нажатия Enter сделать автоотступ
      returnPressed = true;
      QPlainTextEdit::keyPressEvent(e);
      }
    else
      autoComplete();
    //emit contensChanged();
    // e->setAccepted(false);
    }



  //При вставке скобки выполняем вставку парной скобки,
  //при наличии выделения заключаем его в скобки
  else if( e->text() == QString("(") ) {
    mAutoComplete->hide();
    //Если было выделение, то округлить его в парные скобки
    QTextCursor tc = textCursor();
    if( tc.hasSelection() ) {
      int open = tc.selectionStart();
      int close = tc.selectionEnd();
      tc.clearSelection();
      //Едем курсором в начало выделения
      tc.setPosition( open );
      tc.insertText( QString("(") );
      //Теперь едем в конец выделения
      tc.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, close - open );
      tc.insertText( QString(")") );
      setTextCursor( tc );
      }
    else {
      //Вставим парные скобки
      insertPlainText( QString("()") );
      QTextCursor c = textCursor();
      c.movePosition( QTextCursor::Left );
      setTextCursor( c );
      }
    }



  //При вставке закрывающей скобки и наличии справа закрывающей скобки - просто сдвигаем курсор
  else if( e->text() == QString(")") ) {
    mAutoComplete->hide();
    //Если справа закрывающая скобка, то просто сдвигаем курсор
    if( cursorChar(textCursor(),0) == QChar(')') )
      moveCursor( QTextCursor::Right );
    else
      QPlainTextEdit::keyPressEvent(e);
    }


  //При нажатии ctrl+/ выполнить переключение комментария
  else if( e->text() == QString("/") && mControlPress ) toggleRemark();


  //Вставляем парные ковычки или если есть выделение заключаем его в ковычки
  else if( e->text() == QString("\"") ) {
    mAutoComplete->hide();
    //Если было выделение, то округлить его в ковычки
    QTextCursor tc = textCursor();
    if( tc.hasSelection() ) {
      int open = tc.selectionStart();
      int close = tc.selectionEnd();
      tc.clearSelection();
      //Едем курсором в начало выделения
      tc.setPosition( open );
      tc.insertText( QString("\"") );
      //Теперь едем в конец выделения
      tc.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, close - open );
      tc.insertText( QString("\"") );
      setTextCursor( tc );
      }
    else {
      int pos;
      QString line = cursorLine( textCursor(), &pos );
      //Проверить специальный случай ввода файла include
      if( line.simplified().startsWith(QString("#include")) ) {
        //Ввести знак
        QPlainTextEdit::keyPressEvent(e);
        //refillAutoComplete();
        // emit contensChanged();
        return;
        }
      else {
        //Если справа ковычка, то просто сдвигаем курсор
        if( cursorChar(textCursor(),0) == QChar('\"') )
          moveCursor( QTextCursor::Right );
        else {
          //Вставим парные скобки
          insertPlainText( QString("\"\"") );
          QTextCursor c = textCursor();
          c.movePosition( QTextCursor::Left );
          setTextCursor( c );
          }
        }
      }
    }

  //Клавиши вверх-вниз в списке завершателей
  else if( (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) && !mAutoComplete->isHidden() ) {
    //Изменить выбор в автозавершателе
    int i = mAutoComplete->currentRow();
    if( e->key() == Qt::Key_Up ) i--;
    else i++;
    if( i < 0 ) i = mAutoComplete->count() - 1;
    if( i >= mAutoComplete->count() ) i = 0;
    mAutoComplete->setCurrentRow( i );
    }

  //Клавиша забой
  else if( e->key() == Qt::Key_Backspace ) {
    QPlainTextEdit::keyPressEvent(e);
    //Если активно автозавершение, то перенакопить
    // if( !mAutoComplete->isHidden() )
    //refillAutoComplete();
    //emit contensChanged();
    }

  else {
    //Проверить ввод цифро-буквенного значения
    if( !e->text().isEmpty() ) {
      auto symbol = e->text().at(0);
      if( symbol.isLetterOrNumber() || symbol == QChar('_') || symbol == QChar('<') ) {
        //Ввести знак
        QPlainTextEdit::keyPressEvent(e);
        refillAutoComplete();
        return;
        }
      }
    mAutoComplete->hide();
    if( e->key() == Qt::Key_Enter ) {
      //после стандартной обработки нажатия Enter сделать автоотступ
      returnPressed = true;
      }
    QPlainTextEdit::keyPressEvent(e);
    }

  if( returnPressed ) {
    if( mAutoCompleteParenthesis ) {
      if( !completeParenthesis() )
        autoIndent();
      }
    else autoIndent();
    mLastPressedReturn = true;
    }
  else {
    mLastPressedReturn = false;
    }
  }




void TextEditor::keyReleaseEvent(QKeyEvent *e)
  {
  if( mControlPress && (e->modifiers() & Qt::ControlModifier) == 0 ) {
    mControlPress = false;
    mLink.clear();
    unsetCursor();
    //setCursor( Qt::IBeamCursor );
    emit setLink( mLink );
    emit rehighlightBlock( mLinkBlock );
    }
  QPlainTextEdit::keyReleaseEvent( e );
  if( !e->isAutoRepeat() && !e->text().isEmpty() ) {
    //if( !mAutoComplete->isHidden() )
    //   refillAutoComplete();
    emit contensChanged();
    }
  }





void TextEditor::mouseMoveEvent(QMouseEvent *ev)
  {
  //Выполнять всякую херь только если есть фоновый компилятор
  if( Highlighter::mCompiler ) {
    QTextCursor c = cursorForPosition( ev->pos() );
    QString word = getWordCursorOver( c );
    if( mControlPress ) {
      //if( !word.isEmpty() && Highlighter::mSymSources.contains(word) )
      if( word != mLink ) {
        mLink = word;
        emit setLink( mLink );
        emit rehighlightBlock( mLinkBlock );
        mLinkBlock = c.block();
        emit rehighlightBlock( mLinkBlock );
        if( mLink.isEmpty() ) unsetCursor();
        else setCursor( Qt::ArrowCursor );
        }
      }

    if( word != mOverWord ) {
      mOverWord = word;

      if( !mOverWord.isEmpty() ) {
        QString help; //Строка с помощью

        //Получить значение
        if( svMirrorManager && svMirrorManager->mirror()->addressOfName(mOverWord) ) {
          //Это идентификатор значение которого известно
          help = QString("[%1:%2] ").arg(svMirrorManager->mirror()->addressOfName(mOverWord)).arg(svMirrorManager->mirror()->memoryGetByName(mOverWord) );
          }

        //Установить текст помощи, соответствующий тексту ссылки
        SvCompiler6::SvVariablePtr vp = Highlighter::mCompiler->mVarGlobal.getVariable(mOverWord);
        if( vp && !vp->mRemark.isEmpty() ) help.append( vp->mRemark );
        else if( Highlighter::mCompiler->mMacroTable.contains(mOverWord) ) {
          //Вместо помощи показываем содержимое макро
          help = Highlighter::mCompiler->mMacroTable.value(mOverWord)->mExpander;
          }
        else if( Highlighter::mCompiler->mFunGlobal.isPresent(mOverWord) ) {
          //Show function signature
          SvCompiler6::SvFunctionPtr fp = Highlighter::mCompiler->mFunGlobal.getFunction(mOverWord);
          help = fp->mType->mSignature + QChar('\n') + fp->mRemark;
          }

        if( !help.isEmpty() ) {
          mHelpPopUp->setPlainText( help );
          QPoint p = cursorRect( c ).topLeft();
          p.ry() -= mHelpPopUp->height();
          mHelpPopUp->move( p );
          mHelpPopUp->show();
          }
        else mHelpPopUp->hide();
        }
      else mHelpPopUp->hide();
      }
    }

  QPlainTextEdit::mouseMoveEvent( ev );
  }






void TextEditor::mousePressEvent(QMouseEvent *e)
  {
  //Если автозавершатель не скрыть - скрыть его
  if( !mAutoComplete->isHidden() )
    mAutoComplete->hide();
  if( mControlPress && !mLink.isEmpty() ) {
    mControlPress = false;
    toggleF2();
    }
  else
    //Обработать нажатие мыши по умолчанию
    QPlainTextEdit::mousePressEvent(e);
  }






void TextEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));

  }






void
TextEditor::setDebugLine(int line) {
  QTextBlock block;
  if ( mDebugLine > 0 && mDebugLine < document()->blockCount() ) {
    block = document()->findBlockByLineNumber( mDebugLine - 1);
    if ( block.isValid() )
      block.setUserState( block.userState() | 2 );
    emit rehighlightBlock(block);
    }
  //Установить строчку с паузой
  mDebugLine = line;
  //Показать строчку с паузой
  if ( mDebugLine >= 0 && mDebugLine < document()->blockCount() ) {
    block = document()->findBlockByLineNumber( mDebugLine - 1);
    if ( block.isValid() )
      block.setUserState( block.userState() & ~2 );
    emit rehighlightBlock(block);
    }
  locateLine(line);
  }





void
TextEditor::onCursorPositionChanged() {
  mBreakLine = textCursor().blockNumber() + 1;
  highlightCurrentBlock();
  highlightCurrentLine();
  updateHighlights();
  }





void
TextEditor::locateLine(int line)
  {
  if ( line > 0 ) line--;
  else return;

  //Текущее положение курсора
  int curLine = textCursor().blockNumber();

  //Относительное смещение курсора
  curLine -= line;
  while ( curLine > 0 ) {
    moveCursor( QTextCursor::PreviousBlock );
    curLine--;
    }
  while ( curLine < 0 ) {
    moveCursor( QTextCursor::NextBlock );
    curLine++;
    }

  //Сделать курсор видимым
  ensureCursorVisible();

  //Обеспечить фокус
  setFocus();
  }





QColor cHighlightedColor = QColor(Qt::green).lighter(160);
QColor cSelectedHighlightedColor = QColor(Qt::yellow).darker(110);
//подсветка результатов поиска
void TextEditor::highlightSearchResults(const QList<TextSearchResults> & searchResults, int currentIndex)
  {
  QList<QPair<double, double>> highlightPositions;


  QTextCursor c = textCursor();
  c.movePosition(QTextCursor::Start);
  //получить высоту строки в пикселях
  auto lineHeight = cursorRect(c).height();
  //высота документа в пикселях
  auto documentHeight = qMax(document()->blockCount()*lineHeight, viewport()->height());
  //количество строк
  double contentLines = document()->blockCount();
  //высота строки относительно высоты всего документа
  double lineRelativeHeight = lineHeight/(double)documentHeight;

  mSearchHighlight.clear();
  int i =0;
  foreach (auto searchResult, searchResults) {
    QTextEdit::ExtraSelection selection;

    c = textCursor();
    c.setPosition(searchResult.mStartIndex);
    c.setPosition(searchResult.mEndIndex, QTextCursor::KeepAnchor);
    auto pos = c.blockNumber()/(contentLines + 1);

    highlightPositions.append(QPair<double,double>(pos, lineRelativeHeight));
    if (i == currentIndex)
      selection.format.setBackground(cSelectedHighlightedColor);
    else
      selection.format.setBackground(cHighlightedColor);
    selection.cursor = c;
    mSearchHighlight.append(selection);
    ++i;
    }
  //setTextCursor(prev);
  updateHighlights();
  mScrollBarMarker->setHighlights(highlightPositions, currentIndex);
  }





//замена текста с индекса startIndex по endIndex
void TextEditor::replace(int startIndex, int endIndex, const QString &text)
  {
  auto currentCursor = textCursor();
  currentCursor.beginEditBlock();
  currentCursor.setPosition(startIndex);
  currentCursor.setPosition(endIndex, QTextCursor::KeepAnchor);
  currentCursor.removeSelectedText();
  currentCursor.insertText(text);
  currentCursor.endEditBlock();
  }




//замена текста по результатам поиска
void TextEditor::replaceAll(const QList<TextSearchResults> &searchResults, const QString & text)
  {
  auto editorText = toPlainText();
  auto sortedSearchResults = searchResults;
  //отсортировать в обратном порядке
  std::sort(sortedSearchResults.begin(), sortedSearchResults.end(), [](const TextSearchResults & c1, const TextSearchResults & c2){
    return c1.mStartIndex > c2.mStartIndex;
    });
  //заменить текст с конца, так что индексы не собьются как если бы заменяли с начала
  foreach( auto res, sortedSearchResults){
    editorText.replace(res.mStartIndex, res.mEndIndex - res.mStartIndex, text);
    }

  //заменить содержимое документа
  auto currentCursor = textCursor();
  auto oldPosition = currentCursor.position();
  currentCursor.setPosition(0);
  currentCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  currentCursor.beginEditBlock();
  currentCursor.removeSelectedText();
  currentCursor.insertText(editorText);
  currentCursor.endEditBlock();
  //вернуть курсор на место
  currentCursor.setPosition(oldPosition);
  setTextCursor(currentCursor);
  }




//автоотступ строк, содержащихся между позициями по символам startIndex и endIndex
void TextEditor::autoIndent(int startIndex, int endIndex, bool registerUndo)
  {
  //перестроить граф блоков
  updateBlockGraph();

  //определить строки, которые попадают между символами
  auto cursor = textCursor();
  cursor.setPosition(startIndex);
  auto blockNumStart = cursor.blockNumber();
  cursor.setPosition(endIndex);
  auto blockNumEnd = cursor.blockNumber();
  auto currentCursor = textCursor();
  if (registerUndo)
    currentCursor.beginEditBlock();
  //сделать автоотступ
  for (auto i=blockNumStart; i<=blockNumEnd; ++i){
    autoIndentLine(i, false);
    }
  if (registerUndo)
    currentCursor.endEditBlock();
  }




bool TextEditor:: isCommented(int index)const{
  auto cursort = textCursor();
  cursort.setPosition(index);
  return cursort.charFormat().foreground().color() == Qt::darkGreen;
  }




const QStringList cBlockOperators({"if", "else", "while"});
//получить оператор начала блока если такой встретился начиная с позиции j  в тексте text
QString foundBlockOperator(const QString & text, int j){
  bool found = false;
  foreach (auto blockOperator, cBlockOperators) {

    found = true;
    for (auto i=0; i<blockOperator.length(); ++i){
      if (i + j >= text.length()){
        found = false;
        break;
        }
      if (blockOperator[i] != text[j + i]){
        found = false;
        break;
        }
      }
    if (found){
      //спереди и сзади не должно быть текста или цифр
      if (j > 0)
        found &= !(text[j-1].isLetterOrNumber() || text[j-1] == '#');
      if (j + blockOperator.length() < text.length())
        found &= !text[j+blockOperator.length()].isLetterOrNumber();
      if (found)
        return blockOperator;
      }
    }
  return "";
  }





//обновить граф локальных блоков
void TextEditor::updateBlockGraph(){
  //удалить старые
  if (mRootBlock != nullptr)
    mRootBlock->deleteLater();
  mAllBlocks.clear();

  //создать корневой блок - весь документ
  mRootBlock = new BlockNode(nullptr);
  mRootBlock->mLevel = 0;
  mRootBlock->mStartLineNumber = 0;
  mRootBlock->mStartIndex = 0;
  mRootBlock->mSingle = false;
  mRootBlock->mEndIndex = toPlainText().length()-1;
  mRootBlock->mEndLineNumber = document()->lineCount();

  auto doc = document();
  auto numLines = doc->lineCount();
  BlockNode* currentBlock = mRootBlock;
  mAllBlocks.append(mRootBlock);

  //текст закомментирован
  bool blockCommented = false;
  //пройтись по строкам
  for(auto i=0; i<numLines; ++i){
    //получить очередную строку
    auto block =  doc->findBlockByLineNumber(i);
    auto blockText = block.text();
    auto blockPos = block.position();
    auto blockLength = blockText.length();
    //пройтись по строке
    for(auto j=0; j<blockLength; ++j){
      auto c = blockText[j];
      //проверка на комментарии
      if (j < blockLength - 1){
        //строка закоментирована - можно идти дальше
        if (c =='/' && blockText[j+1] == '/')
          break;
        //встретили блочный комментарий
        if (c == '/' && blockText[j+1] == '*'){
          j++;
          blockCommented = true;
          continue;
          }
        //блочный комментарий закончен
        if (blockCommented && (blockText[j] == '*' && blockText[j+1] == '/')){
          j++;
          blockCommented = false;
          continue;
          }
        }
      //если символ внутри блочного комментария - пропустить его
      if (blockCommented)
        continue;

      //проверить оператор начала блока (if, while, else)
      auto foundOperator = foundBlockOperator(blockText, j);
      if (foundOperator.length() > 0){
        //создать соответствующий блок, одиночный
        auto newBlock = new BlockNode(currentBlock);
        newBlock->mLevel = currentBlock->mLevel + 1;
        newBlock->mStartLineNumber = i;
        newBlock->mStartIndex = blockPos + j;
        newBlock->mSingle = true;
        //задать текущий блок
        currentBlock = newBlock;
        mAllBlocks.append(newBlock);
        //пропустить лишние символы оператора
        j += foundOperator.length() - 1;
        if (j >= blockText.length())
          continue;
        }
      //если текущий блок одиночный и встретили ; - закончить одиночный блок
      if (currentBlock->mSingle){
        if (c == ';' ){
          //для вложенный одиночных блоков , например
          //if (a==0)
          //     if(b==0)
          //         c = 0;
          //подниматься вверх, пока не найден неодиночный блок
          while (currentBlock->mSingle){

            currentBlock->mEndLineNumber = i;
            if (currentBlock->mEndLineNumber == currentBlock->mStartLineNumber){
              currentBlock->mLevel = currentBlock->mLevel - 1;
              }
            currentBlock->mEndIndex = blockPos + j;
            currentBlock = qobject_cast<BlockNode*>(currentBlock->parent());
            }
          }
        }
      //встретили открывающую скобку
      if ( c == '{' ){
        //если текущий блок считался одиночным - перестать считать его таким, обрабатывать как обычный
        if (currentBlock->mSingle){
          currentBlock->mSingle = false;
          currentBlock->mStartLineNumber = i;
          currentBlock->mStartIndex = blockPos + j;
          }
        else{
          //создать новый блок
          auto newBlock = new BlockNode(currentBlock);
          newBlock->mLevel = currentBlock->mLevel + 1;
          newBlock->mSingle = false;
          newBlock->mStartLineNumber = i;
          newBlock->mStartIndex = blockPos + j;
          currentBlock = newBlock;
          mAllBlocks.append(newBlock);
          }
        }
      //найдена закрывающая скобка
      if ( c == '}'){
        //сейчас уровень документа - значит ошибка в фигурных скобках, прервать процесс
        if (currentBlock == mRootBlock)
          return;
        else{
          //закончить блок
          currentBlock->mEndLineNumber = i;
          if (currentBlock->mEndLineNumber == currentBlock->mStartLineNumber)
            currentBlock->mEndIndex = blockPos + j;
          currentBlock = qobject_cast<BlockNode*>(currentBlock->parent());
          //обработать вложенные единичные блоки
          //случай if(a==0)
          //           if(b==0)
          //              while(c==0)
          //              {
          //                   ...
          //              }
          while (currentBlock->mSingle){
            currentBlock->mEndLineNumber = i;
            currentBlock->mEndIndex = blockPos + j;
            currentBlock = qobject_cast<BlockNode*>(currentBlock->parent());
            }
          }
        }
      }
    }
  }




//автоотступ строки
void TextEditor::autoIndentLine(int lineNum, bool registerUndo)
  {
  BlockNode* containerBlock = getBlockForLine(lineNum);
  indentLine(lineNum, containerBlock->mLevel*mAutoIndentSpacesCount, registerUndo);
  }




//отступ строки на заданное количество пробелов
void TextEditor::indentLine(int lineNum, int spacesCount, bool registerUndo)
  {
  auto block =  document()->findBlockByLineNumber(lineNum);
  auto blockText = block.text();
  auto leadingSpace = getLeadingSpaceCount(blockText);
  auto position = block.position();
  auto currentCursor = textCursor();
  if (registerUndo)
    currentCursor.beginEditBlock();
  //убрать существующие пробелы
  currentCursor.setPosition(position);
  currentCursor.setPosition(position + leadingSpace, QTextCursor::KeepAnchor);
  currentCursor.removeSelectedText();
  //добавить необходимое количество пробелов
  auto leadingSpaces = QString("");
  for (auto i=0; i<spacesCount; ++i)
    leadingSpaces += " ";
  currentCursor.insertText(leadingSpaces);
  if (registerUndo)
    currentCursor.endEditBlock();
  }



QString getRemark( const QStringList line ) {
  //Просканировать строку и обнаружить комментарии
  int i;
  for( i = 0; i < line.count(); i++ )
    if( line.at(i).startsWith(QString("//")) ) {
      //Комментарии обнаружены, накопить
      QString str;
      while( i < line.count() ) {
        str.append( line.at(i) );
        str.append( QString(" ") );
        i++;
        }
      return str;
      }
  return QString();
  }



//Сгенерировать Sv QML код на основе выделения
void TextEditor::editSvQmlGenerate()
  {
  QStringList result;
  //Получить выделенный текст
  QString src = textCursor().selectedText();
  if( !src.isEmpty() ) {
    //Разобрать его по строкам
    QStringList list = src.split( QChar(QChar::ParagraphSeparator), QString::SkipEmptyParts );
    for( QString str : list ) {
      //Парсим каждую строчку
      QStringList line = str.split( QChar(' '), QString::SkipEmptyParts );
      if( line.count() && line.at(0) != QString("int") && line.at(0).at(0).isLetter() ) {
        //Идентификатор прямо с первой позиции
        QString ident = line.at(0);
        if( ident.endsWith(QChar(',')) || ident.endsWith(QChar(';')) )
          ident.chop(1);
        QString s("SvQmlValue { id: %1; mirror: vpu; name: \"%1\" } ");
        result.append( s.arg(ident) + getRemark(line) );
        }
      else if( line.count() > 1 && line.at(0) == QString("int") && line.at(1).at(0).isLetter() ) {
        //Идентификатор со второй позиции
        QString ident = line.at(1);
        if( ident.endsWith(QChar(',')) || ident.endsWith(QChar(';')) )
          ident.chop(1);
        QString s("SvQmlValue { id: %1; mirror: vpu; name: \"%1\" } ");
        result.append( s.arg(ident) + getRemark(line) );
        }
      else if( line.count() > 2 && line.at(0) == QString("#define") ) {
        //Это define
        QString s("SvQmlDefine { id: df%1; config: cfg; name: \"%2\" }");
        QString ident;
        QString src(line.at(1));
        bool lover = false;
        for( int i = 0; i < src.length(); i++ )
          if( src.at(i).isLetter() ) {
            if( lover ) ident.append( src.at(i).toLower() );
            else        ident.append( src.at(i).toUpper() );
            lover = true;
            }
          else if( src.at(i) == QChar('_') ) lover = false;
          else if( src.at(i).isDigit() ) ident.append( src.at(i) );
        result.append( s.arg(ident).arg(src) + getRemark(line) );
        }
      }
    //Разместить преобразованный текст в буфере обмена
    QClipboard *clip = QGuiApplication::clipboard();
    clip->setText( result.join(QChar('\r')) );
    }
  }


