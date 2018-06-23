﻿/*
Проект "Компилятор"
Содержание
  Реализация класса TCompiler часть 1 (макропроцессор)
  Конструктор
    TCompiler();
  Функции:
            void     Error( int errId, CPChar err, CPChar param1, CPChar param2, CPChar param3 );
            void     ErrorKillLine( int errId, CPChar err, CPChar param1, CPChar param2, CPChar param3 );
            void     ErrorEndSt( int errId, CPChar err, CPChar param1, CPChar param2, CPChar param3 );
*/
#include "SrCompiler.h"

using namespace SrCompiler6;

/*
Функция "Выдача ошибок"
*/
void
SrCompiler::Error( const QString &err ) {
  //Вывести сообщение о имени файла и номере ошибки
  ErrorInLine( err, mToken.mFileId, mToken.mLine );
  }


//Зафиксировать ошибку в заданном файле и строке
void SrCompiler::ErrorInLine(const QString &err, int fileId, int lineId)
  {
  SrError error;
  error.mError = err;
  error.mFile  = mFileTable.value(fileId);
  error.mLine  = lineId;
  mErrorList.append( error );
  }




void SrCompiler::errorInLine(const QString &err, const SrMark &mark)
  {
  ErrorInLine( err, mark.mFile, mark.mLine );
  }






/*
Функция "Ошибка и подавить до конца строки"
Описание
  Выдаем ошибку, подавляем до конца строки
*/
void
SrCompiler::ErrorKillLine( const QString &err ) {
  //Выдать ошибку
  Error( err );
  //Подавить до конца строки
  mPtr = mLine.size();
  //Прочитать дальше
  Blank();
  }






/*
Функция "Ошибка и подавить до конца оператора"
Описание
  Выдаем ошибку, подавляем до конца оператора
*/
void
SrCompiler::ErrorEndSt( const QString &err ) {
  //Выдать ошибку
  Error( err );
  //Подавить до конца оператора
  while( !mEof ) {
    if( !IsEoln() ) {
      if( mLine.at(mPtr++) == QChar(';') ) {
        NextToken();
        return;
        }
      }
    else Blank();
    }
  }


/*
Функция "Выдать список ошибок в виде сплошного текста, разделенного на строки"
Описание
  Для каждой ошибки формируем строку вида "описание ошибки" в файле ff в строке nn
  */
QString
SrCompiler::errorList() {
  QString dest;

  foreach( const SvError &err, mErrorList ) {
    //Вывести сообщение о имени файла и номере ошибки
    dest.append( err.mError ).append( QObject::tr("\nIn file '%1' line %2:\n").arg(err.mFile).arg(err.mLine) );
    }

  return dest;
  }




void
SrCompiler::ErrNeedLValue() {
  Error( QObject::tr("Error. Expression must be LValue.") );
  }









void
SrCompiler::ErrIllegalOperation() {
  Error( QObject::tr("Error. Invalid operation with that types.") );
  }




bool SrCompiler::needToken(int token, const QString need)
  {
  if( mToken == token ) {
    NextToken();
    return true;
    }
  ErrorEndSt( QObject::tr( "Need %1").arg(need) );
  NextToken();
  return false;
  }

