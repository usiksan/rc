﻿/*
Проект "Компилятор"
Содержание
  Реализация класса TCompiler часть 2 (входной поток низкого уровня)
  Функции:
            void     DoFile( CPChar fname, bool current );
            TPSource CreateSource( CPChar fname, bool current );
            void     InLine();

*/
#include "SrCompiler.h"

using namespace SrCompiler6;

//============================================================================
// TSource
SrSource::~SrSource() {
  if( mInputStream ) delete mInputStream;
  }

QString
SrSource::ReadLine() {
  if( mInputStream ) {
    mLineCount++;
    return mInputStream->readLine();
    }
  return QString();
  }

bool
SrSource::Eof() {
  if( mInputStream )
    return mInputStream->atEnd();
  return true;
  }



//============================================================================
// TSource
SrFileSource::SrFileSource(QFile *file, int fileId) :
  SrSource( fileId ),
  mFile(file)
  {
  mInputStream = new QTextStream(mFile);
  }

SrFileSource::~SrFileSource() {
  if( mFile ) delete mFile;
  }




//============================================================================
// TTextSource
SrStringSource::SrStringSource(const QString &src, int fileId) :
  SrSource(fileId),
  mSource(src)
  {
  mInputStream = new QTextStream( &mSource, QIODevice::ReadOnly );
  }





/*
Функция "Начать файл"
Описание
  Пытаемся открыть файл, в случае неуспеха выдаем ошибку
*/
void
SrCompiler::DoFile( const QString &fname, bool current ) {
  try {
    SrSourcePtr source = CreateSource( fname, current );
    if( source ) {
      //Проверить рекурсию
      bool recurse = false;
      foreach( SrSourcePtr ptr, mSourceStack )
        if( ptr->FileId() == source->FileId() ) {
          //Обнаружена рекурсия
          recurse = true;
          break;
          }

      if( recurse )
        ErrorKillLine( QObject::tr("Error. Recursion on include file \"%1\".").arg(fname) );
      else {
        //Инициировать чтение из нового источника
        mSourceStack.push( source ); //Поместить источник в стек
        mPtr = mLine.size();         //Инициировать чтение строки из файла
        mEof = false;                //Файл открыт, все нормально
        }
      }
    else ErrorKillLine( QObject::tr("Error. Can not open file \"%1\".").arg(fname) );
    }
  catch(...) {
    ErrorKillLine( QObject::tr("Error. Can not open file \"%1\".").arg(fname) );
    }
  }




/*
Функция "Создание источника исходного кода"
Параметры
  fname   - имя файла для источника
  current - истина, когда файл берется из текущего пути, в противном случае из системного
Описание
  Если файл нужен из текущего места, то просто его открываем,
  если из системного, то предварительно добавляем путь к системным файлам
*/
SrSource*
SrCompiler::CreateSource( const QString &fname, bool current ) {
  QString file;
  if( current )
    file = mProjectPath + fname;
  else {
    //Вначале искать в пути проекта
    if( QFile::exists( mProjectPath + fname ) )
      file = mProjectPath + fname;
    else {
      //Искать в списке системных путей
      foreach( QString s, mSystemPathList )
        if( QFile::exists( s + fname ) ) {
          file = s + fname;
          break;
          }
      }
    }
  //Если файла нету, то выдать ошибку
  if( !QFile::exists(file) ) {
    SrError err;
    err.mError = QObject::tr("Can't find file ").append( fname );
    err.mFile  = file;
    err.mLine  = 0;
    mErrorList.append( err );
    return 0;
    }
  //Грузить
  QFile *is = new QFile( file );
  if( is->open( QIODevice::ReadOnly ) ) {
    //Файл открылся, создать читатель файла
    return new SrFileSource( is, AddFile(file) );
    }
  return 0;
  }



int SrCompiler::AddFile(const QString &fname)
  {
  //Разместить файл в общей таблице файлов
  int id = mFileTable.key( fname );
  if( id == 0 ) {
    id = ++mFileIdCount;
    mFileTable.insert( id, fname );
    }
  return id;
  }


/*
Функция "Получить очередную строку из файла"
Описание
  Если файл пуст, закрываем, извлекаем из стека и переходим к предыдущему
  Если стек пуст, то устанавливаем конец источника и выход
*/
void
SrCompiler::InLine() {
  while( !mEof ) {
    if( mSourceStack.isEmpty() ) {
      //Если стек источников пуст, то установить конец файла и возврат
      mEof = true;
      return;
      }

    if( mSourceStack.top()->Eof() ) {
      //Если конец источника, то удалить источник
      delete mSourceStack.pop();
      }
    else {
      //Прочитать очередную строку
      mInLine = mSourceStack.top()->ReadLine();
      //Если строка не пустая, то инициализация и возврат
      if( !mInLine.isEmpty() ) {
        mLine = mInLine;
        mPtr = 0;
        //if( mPtr[strlen(mPtr)-1] == '\r' ) mPtr[strlen(mPtr)-1] = ' ';
        return;
        }
      }
    }
  }




/*
Функция "Получить место текущего токена во исходном коде"
Описание
  Возвращаем текущее место в исходном коде
*/
SrMark SrCompiler::mark() const
  {
  return SrMark( mFileId, mLineIndex, 0 /* mLineInt[mPtr] */ );
  }

