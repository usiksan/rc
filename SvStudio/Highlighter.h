﻿#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include "SvCompiler/SvCompiler.h"


class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

    QString mLink; //Идентификатор для перехода по линку
  public:
    static SvCompiler6::SvCompiler   *mCompiler;       //Компилятор

//    static QHash<QString,QString>     mMacroTable;     //Таблица макроимен
//    static QHash<QString,TFileLine>   mSymSources;     //Места определения переменных

    Highlighter(QTextDocument *parent = nullptr);


  protected:
    void highlightBlock(const QString &text);

  private:

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat mKeywordFormat;           //!< Ключевые слова
    QTextCharFormat mSingleLineCommentFormat; //!< Однострочные коментарии
    QTextCharFormat mMultiLineCommentFormat;  //!< Многострочный коментарий
    QTextCharFormat mQuotationFormat;         //!< Строки и символы
    QTextCharFormat mIdentFormat;             //!< Для идентификаторов глобальных
    QTextCharFormat mFuncFormat;              //!< Для функций глобальных
    QTextCharFormat mMacroFormat;             //!< Макрос
    QTextCharFormat mPreprocessFormat;        //!< Препроцессор
    QTextCharFormat mDecimalFormat;           //!< Десятичные числа
    QTextCharFormat mOctalFormat;             //!< Восьмиричные числа
    QTextCharFormat mHexFormat;               //!< Шестнадцатиричные числа
    QTextCharFormat mDebugFormat;             //!< Текущая отлаживаемая строка
    QTextCharFormat mLinkFormat;              //!< Ссылка на определение переменной

    int SkipQuotation( const QString &text, int index, int count );
    int MultiLineComment( const QString &text, int index, int count );
  signals:

  public slots:
    void setLink( const QString link );

  };

#endif // HIGHLIGHTER_H
