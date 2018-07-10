﻿/*
  Проект "SaliDs"
    Визуальное программирование микроконтроллеров
  Автор
    Сибилев А.С.
  Описание
*/
#include "SvConfig.h"
#include "SvProject.h"
#include "SvUtils.h"
#include "WCommand.h"
#include "Host/SvMirror.h"
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QDebug>


SvProject* svProject;

SvProject::SvProject() :
  QObject()
  {
  //Установить поля проекта по-умолчанию
  setDefaults();
  }

SvProject::~SvProject()
  {

  }


void SvProject::openProject(const QString fname)
  {
  qDebug() << "openProject" << fname;
  addRecentProject( fname );

  QFile file(fname);
  if( file.open(QIODevice::ReadOnly) ) {
    //Прочитать проект
    mProjectSrc = QJsonDocument::fromJson( file.readAll() ).object();
    if( mProjectSrc.isEmpty() ) {
      //Проект не загрузился, сбросить информацию
      mProjectName.clear();
      mProjectPath.clear();
      mMainScript.clear();
      }
    else {
      //Сформировать информацию о проекте
      QFileInfo info(fname);
      mProjectName = info.completeBaseName();
      mProjectPath = info.absolutePath();
      if( !mProjectPath.endsWith( QChar('/') ) )
        mProjectPath.append(QChar('/'));

      mDebugType    = mProjectSrc.value( WPK_DEBUG_TYPE ).toInt();    //Тип отладчика
      mRemoteIp     = mProjectSrc.value( WPK_REMOTE_IP ).toString();    //IP адрес удаленной машины
      mRemotePort   = mProjectSrc.value( WPK_REMOTE_PORT ).toInt();         //Порт удаленной машины
      mGlobalId     = mProjectSrc.value( WPK_GLOBAL_ID ).toString();     //Глобальный идентификатор машины в интернете
      mCompilerType = mProjectSrc.value( WPK_COMPILER_TYPE ).toString(); //Тип компилятора
      mCtrVersion   = mProjectSrc.value( WPK_CTR_VERSION ).toInt();      //Исполнение контроллера
      mMainScript   = mProjectSrc.value( WPK_MAIN_SCRIPT ).toString();   //Основной скрипт узла
      mVid          = mProjectSrc.value( WPK_VID ).toInt();
      mPid          = mProjectSrc.value( WPK_PID ).toInt();
      }
    }
  }





void SvProject::saveProject()
  {
  //Записать текущие данные
  mProjectSrc.insert( WPK_DEBUG_TYPE,     mDebugType );
  mProjectSrc.insert( WPK_REMOTE_IP,      mRemoteIp );
  mProjectSrc.insert( WPK_REMOTE_PORT,    mRemotePort );
  mProjectSrc.insert( WPK_GLOBAL_ID,      mGlobalId );
  mProjectSrc.insert( WPK_COMPILER_TYPE,  mCompilerType );
  mProjectSrc.insert( WPK_CTR_VERSION,    mCtrVersion );
  mProjectSrc.insert( WPK_MAIN_SCRIPT,    mMainScript );
  mProjectSrc.insert( WPK_VID,            mVid );
  mProjectSrc.insert( WPK_PID,            mPid );


  QFile file(mProjectPath + mProjectName + EXTENSION_PROJECT);
  if( file.open(QIODevice::WriteOnly) ) {
    //Записать
    file.write( QJsonDocument(mProjectSrc).toJson() );
    }
  }





void SvProject::addRecentProject(const QString fname)
  {
  QSettings s;
  QStringList list;
  if( s.contains(CFG_PROJECTS) ) {
    //Получить список
    list = s.value(CFG_PROJECTS).toStringList();
    //Проверить, есть ли в списке нужный проект
    int i = findInList( fname, list );
    if( i >= 0 ) {
      //Вынести наверх
      list.removeAt( i );
      }
    else {
      //Удалить последний
      while( list.count() >= MAX_RECENT_PROJECT )
        list.removeLast();
      }
    list.insert( 0, fname );
    }
  else list.append( fname );

  s.setValue( CFG_PROJECTS, list );

  updateRecentProjects();
  }





void SvProject::updateRecentProjects()
  {
  QSettings s;
  QStringList list;
  if( s.contains(CFG_PROJECTS) )
    //Получить список
    list = s.value(CFG_PROJECTS).toStringList();


  //Изменить меню прежних проектов
  for( int i = 0; i < MAX_RECENT_PROJECT; ++i )
    if( i < list.count() ) {
      QFileInfo info( list.at(i) );
      maRecentProject[i]->setText( info.completeBaseName() );
      maRecentProject[i]->setData( list.at(i) );
      maRecentProject[i]->setToolTip( list.at(i) );
      maRecentProject[i]->setVisible(true);
      }
    else maRecentProject[i]->setVisible(false);

  maFileRecentProject->setEnabled( list.count() != 0 );

  emit recentProjectsList( list );
  }





void SvProject::clearRecentProjects()
  {
  QStringList list;
  QSettings s;
  s.setValue( CFG_PROJECTS, list );
  updateRecentProjects();
  }





void SvProject::createNew(const QString fname)
  {
  addRecentProject( fname );

  QFile file(fname);
  if( file.open(QIODevice::WriteOnly) ) {
    //Сформировать пустой проект
    mProjectSrc = QJsonObject();
    mProjectSrc.insert( QString("A contens"), QString("SaliLAB SV " SV_VERSION) );
    file.write( QJsonDocument(mProjectSrc).toJson() );
    file.close();
    //Сформировать информацию о проекте
    QFileInfo info(fname);
    mProjectName = info.completeBaseName();
    mProjectPath = info.absolutePath();
    if( !mProjectPath.endsWith( QChar('/') ) )
      mProjectPath.append(QChar('/'));

    setDefaults();
    }
  }

//Заполнить поля проекта по-умолчанию
void SvProject::setDefaults()
  {
  mDebugType    = SMT_LOCAL;    //Тип отладчика
  mRemoteIp     = QString("127.0.0.1");     //IP адрес удаленной машины
  mRemotePort   = DEFAULT_REMOTE_PORT;   //Порт удаленной машины
  mGlobalId     = QString();     //Глобальный идентификатор машины в интернете
  mPassw        = QString();
  mVid          = 0x2743;
  mPid          = 0x1971;
  mCompilerType = QString(DEFAULT_COMPILER); //Тип компилятора
  mCtrVersion   = 0;   //Исполнение контроллера
  mMainScript   = QString("main.c");   //Основной скрипт узла
  }

