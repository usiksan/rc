﻿/*
  Проект     "Скриптовый язык SaliScript: упрощенный c++"
  Подпроект  "Host-система"
  Автор
    Alexander Sibilev
  Интернет
    www.rc.saliLab.ru - домашний сайт проекта
    www.saliLab.ru
    www.saliLab.com

  Описание
    Отображение локальной исполняющей системы. Исполняющая система передается в качестве
    параметра
*/


#ifndef SVMIRRORLOCAL_H
#define SVMIRRORLOCAL_H

#include "SvMirror.h"
#include "SvVMachineLocal.h"

class SvMirrorLocal : public SvMirror
  {
    Q_OBJECT

    SvVMachineLocal *mController;  //! Local controller
    int              mDivider;     //! Divider for change event thinning
    QVector<int>     mVpuDebugRun; //! Previous states of debug flag every task
    QString          mControllerType; //! Тип контроллера, возвращаемый функцией controllerType
  public:
    SvMirrorLocal(SvVMachineLocal *controller, const QString &ctrType = QString{} );
    virtual ~SvMirrorLocal() override;

    //!
    //! \brief controllerType Return current linked controller type name string
    //! \return Current linked controller type name string. If no controller linked
    //!         then empty string returned
    //!
    virtual QString     controllerType() const override;

    //!
    //! \brief programmName Return programm name loaded into controller
    //! \return Programm name loaded into controller
    //!
    virtual QString     programmName() const override;

    //!
    //! \brief vpuVector Return all vpu current status
    //! \return All vpu current status
    //!
    virtual SvVpuVector vpuVector() const override;

    //!
    //! \brief addressOfName Return address of symbol or zero if name not defined
    //! \param name          Name which address need to find
    //! \return              Address of symbol name
    //!
    virtual int         addressOfName( const QString name ) const override;

    //!
    //! \brief memoryGet Return value of memory cell with index
    //! \param index     Cell index which value will be retrived
    //! \return          Value of cell
    //!
    virtual int         memoryGet( int index ) const override;

    //!
    //! \brief memoryGlobalCount Return ram cell memory count for global variables
    //! \return Ram cell memory count for global variables
    //!
    virtual qint32      memoryGlobalCount() const override;

  public slots:


    //!
    //! \brief setProgrammFlashRun Flash programm to controller and run it or paused
    //! \param prog                Programm which flashed to controller
    //! \param runOrPause          If true then programm automaticly started after flash, else - it paused
    //! \param flash               If true then programm flashed into external controller, else - do nothing
    //!
    virtual void setProgrammFlashRun( SvProgrammPtr prog, bool runOrPause, bool flash ) override;


    //!
    //! \brief memorySet Set memory cell new value
    //! \param index     Memory cell index
    //! \param value     Memory cell value
    //!
    virtual void memorySet( int index, int value ) override;

    //!
    //! \brief debug     Execute one debug command
    //! \param taskId    Task index for which debug command
    //! \param debugCmd  Debug command code
    //! \param start     Start address (used some debug commands)
    //! \param stop      Stop address (used some debug commands)
    //!
    virtual void debug( int taskId, int debugCmd, int start, int stop ) override;

    //!
    //! \brief processing Perform periodic mirror handle
    //! \param tickOffset Time in ms between previous calling this function and this one
    //!
    virtual void processing( int tickOffset ) override;

  };

#endif // SVMIRRORLOCAL_H
