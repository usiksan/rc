/*
  Проект     "Скриптовый язык reduced c++ (rc++) v6"
  Подпроект  "Виртуальная машина"
  Автор
    Alexander Sibilev
  Интернет
    www.rc.saliLab.ru - домашний сайт проекта
    www.saliLab.ru
    www.saliLab.com

  Описание
    Виртуальная машина для исполнения байт-кода.
    Виртуальная машина содержит в себе память программ, память данных
    и набор виртуальных процессоров.
*/

#ifndef SVCONTROLLER_H
#define SVCONTROLLER_H

#include "SvVMachine/SvVmTypes.h"
#include "SvVMachine/SvVmCodeHeader.h"
#include "SvVMachine/SvVmVpu.h"
#include "SvVMachine/SvVmUtils.h"

#define SV_VMACHINE_VERSION "v6.01"

/*!
   \brief The SvVMachine class - виртуальная машина для исполнения байт-кода

    Виртуальная машина содержит в себе память программ, память данных
    и набор виртуальных процессоров. Класс SvVMachine не может использоваться напрямую.
    Вместо этого должен быть создан производный класс в котором будут реализованы
    функции реакции на нарушение доступа к памяти, а также функция executeMethod,
    в которой определяются внешние, по отношению к скрипту, функции.

    Также важно помнить, что конструктор SvVMachine по умолчанию не выделяет никакой памяти.
    Поэтому количество памяти программ, данных и количество виртуальных процессоров - все это
    задается в производном классе.

 */
class SvVMachine
  {
  protected:
    int            *mMemory;          //! Указатель на память данных
    int             mMemorySize;      //! Размер памяти данных
    int             mStack;           //! Положение стека для операций выделения стека для процессов
    SvVmCode       *mProg;            //! Указатель на программу (память программ)
    int             mProgSize;        //! Размер памяти программ
    SvVmVpu        *mVpuList;         //! Список виртуальных процессоров
    int             mVpuCount;        //! Количество активных виртуальных процессоров
    int             mVpuMax;          //! Максимальное количество виртуальных процессоров
  public:

    /*!
       \brief SvVMachine Конструктор виртуальной машины.
     */
    SvVMachine();
    virtual ~SvVMachine();



    //===========================
    // Информационные
    /*!
       \brief getMemorySize Возвращает размер памяти данных
       \return Размер памяти данных
     */
    int getMemorySize() const { return mMemorySize; }

    const int *getMemory() const { return mMemory; }




    /*!
       \brief getVpuMax Возвращает максимальное количество виртуальных процессоров
       \return Максимальное количество виртуальных процессоров
     */
    int getVpuMax() const { return mVpuMax; }




    /*!
       \brief processing Выполняет один шаг обработки всех активных виртуальных процессоров
       \param timeOffset - интервал времени, который нужно прибавить к счетчику системного времени (в мс)

       Эта функция выполняет один цикл обработки всех активных виртуальных процессоров.

       Перед началом этого цикла к встроенной переменной, которая отвечает за отсчет временных
       интервалов vpuTickCount добавляет смещение timeOffset. Время, записанное в этой переменной
       показывает количество милисекунд с момента старта виртуальной машины. При длительной работе
       виртуальной машины существует проблема заворачивания значения времени. Эта проблема легко
       решается путем правильного вычисления временных интервалов (timeOut - vpuTickCount) > 0.
       Приведенным образом можно отслеживать интервалы продолжительностью до 23 дней, что на практике
       более чем достаточно. Для работы со временем также можно использовать календарь.

       Активным считается виртуальный процессор, который не находится в состоянии останова.

       Обработка одного активного виртуального процессора включает в себя последовательное исполнение
       команд начиная с текущей до момента вызова функции переключения задач. Для исключения
       зацикливания при отладке скрипта установлено ограничение на количество команд в последовательности,
       после которого все-равно произойдет переключение задачи.
     */
    virtual void processing( int timeOffset );




    /*!
       \brief memFail Вызывается из виртуальной машины при ошибке доступа к памяти
         данных. В производных классах должна быть реализована соответствующая реакция
         Такая ошибка возникает при попытке обратиться к несуществующей памяти.
       \param vpu Виртуальный процессор, вызвавший нарушение
       \param index Индекс ячейки памяти, который вызвал нарушение
     */
    virtual void memFail( SvVmVpu *vpu, int index ) const = 0;





    /*!
       \brief progFail Вызывается из виртуальной машины при ошибке доступа к памяти
         программ. В производных класса должна быть реализована соответствующая реакция.
         Данная ошибка возникает при попытке обратиться по несуществующему адресу
         памяти программ.
       \param vpu Виртуальный процессор, вызвавший нарешние
       \param address Начальный адрес, по которому производилось считывание
       \param len Длина данных, подлежащих считыванию
     */
    virtual void progFail( SvVmVpu *vpu, int address, int len ) = 0;





    /*!
       \brief executeMethod Выполняет внешнюю, по отношению к скрипту, функцию
       \param vpu Виртуальный процессор, вызвавший внешнюю функцию
       \param methodId Числовой идентификатор внешней функции
       \return Возвращает истину, если нужно ждать (переключиться на другую задачу).
         Если вернуть ложь, то выполнение программы будет продолжено.

       Данная функция реализует механизм вызова из скрипта функций обработки - драйверов.
       Такие функции могут выполнить обращение к оборудованию и/или
       выполнить сложные ресурсоемкие вычисления.

       Для определения внешний функций в скрипте используется следующий механизм:
       \code
       file.h
       #define MY_FUNC_ID 1 //Числовой идентифкатор внешней функции
       ...
       #ifdef VPU_COMPILER
       //Описание прототипа внешней функции
       import MY_FUNC_ID void myFunc( int a, int b );
       #endif
       \endcode

       Файл file.h может включаться в программу скрипта, чтобы правильно вызывать внешние функции.
       Также он может включаться в файл конкретной реализации виртуальной машины, где в
       переопределении executeMethod будет реализована эта функция.
     */
    virtual bool executeMethod( SvVmVpu *vpu, int methodId );





    //===========================
    //Раздел исполнения прерываний
    /*!
       \brief executeHandler0 Исполнение прерываний без параметра
       \param addr Адрес функции прерывания

       Функции прерываний позволяют организовать в скрипте обработку внешних событий.
       Примером таких событий может служить запрос по шине CAN, UART, SPI и т.д.
       Т.е. прерывание актуально в том месте, где контроллер с исполняемым скриптом
       является ведомым по отношению к другим устройствам и должен оперативно отреагировать.

       Важно! Быстродействие виртуальной машины ограничено, поэтому, если событие требует
       быстрой реакции, то для таких случаев прерывание в скриптах не подходит.
       Обычная задержка по реакции составляет единицы мс.

       Механизм регистрации прерываний, а также их вызова должен быть реализован
       в производном классе от SvVMachine.

       Прерывание исполняется в контексте виртуального процессора 0, т.е. стартовой задачи.
       Для исполнения прерывания стартовая задача должна обладать стеком соответствующего
       размера.

       Функция прерывания - это обычная функция. Из нее могут вызываться другие функции.
       Переключения задач в функции прерывания и в вызываемых функциях не допускаются.
       Использование исключений также не допускается.
     */
    void executeHandler0( int addr );
    
    /*!
       \brief executeHandler1 Исполнение прерывания с одним параметром
       \param addr Адрес функции прерывания
       \param value0 Параметр для прерывания
     */
    void executeHandler1( int addr, int value0 );
    
    /*!
       \brief executeHandler2 Исполнение прерывания с двумя параметрами
       \param addr Адрес функции прерывания
       \param value0 Параметр 0 для прерывания
       \param value1 Параметр 1 для прерывания
     */
    void executeHandler2( int addr, int value0, int value1 );






    //===========================
    //Раздел управления

    /*!
       \brief setProgramm Установить новую программу
       \param code Адрес начала заголовка программы

       При установке новой программы исполнение предыдущей программы прекращается
       Все задачи останавливаются. Обновляется указатель на память программ
       и запоминается размер памяти программ.
     */
    void            setProgramm( const SvVmCode *code);


    /*!
       \brief reset Все очистить и убрать все виртуальные процессоры
     */
    void            reset();




    /*!
       \brief restart Сначала сброс, затем создание корневого виртуального процессора
                       и пуск с начального адреса
       \param run задает режим работы. Если ложно, то программа будет стоять на паузе
         и ожидать шаг. Если истина, то программа будет выполняться обычным образом
         со стартовой функции.
     */
    void            restart( bool run );




    /*!
       \brief taskCount Возвращает количество активных задач
       \return Количество активных задач
     */
    int             taskCount() const { return mVpuCount; }




    /*!
       \brief vpu Выдает виртуальный процессор по его индексу
       \param id Индекс виртуального процессора
       \return Виртуальный процессор с индексом id

       Важно! Контроль границ индекса не производится
     */
    const SvVmVpu  *vpu( int id ) const { return mVpuList + id; }


    /*!
       \brief vpuId Выдает индекс виртуального процессора
       \param vpu Виртуальный процессор
       \return Индекс виртуального процессора
     */
    int             vpuId( SvVmVpu *vpu ) { return vpu - mVpuList; }


    /*!
       \brief code Возвращает указатель на память программ
       \return указатель на память программ
     */
    const SvVmCode *code() const { return mProg; }





    //===========================
    //Информация из программы
    /*!
       \brief hashProg Возвращает хэш значение текущей программы
       \return Хэш значение текущей программы
     */
    int  hashProg()    const { return svIRead32( mProg + SVVMH_HASH ); }


    /*!
       \brief signature0 Возвращает сигнатуру программы как целое
       \return 4 байта сигнатуры программы

       Группа функций возвращает сигнатуру программы как набор целых.
       Все вместе они образуют текстовую строку, представляющую собой сигнатуру
       программы - текстовое имя.
     */
    int  signature0()  const { return svIRead32( mProg + SVVMH_SIGNATURE ); }
    int  signature1()  const { return svIRead32( mProg + SVVMH_SIGNATURE + 4 ); }
    int  signature2()  const { return svIRead32( mProg + SVVMH_SIGNATURE + 8 ); }
    int  signature3()  const { return svIRead32( mProg + SVVMH_SIGNATURE + 12 ); }
    int  signature4()  const { return svIRead32( mProg + SVVMH_SIGNATURE + 16 ); }



    /*!
       \brief initTable Возвращает начало таблицы инициализации
       \return Начало таблицы инициализации

       Таблица инициализации состоит из записей размером 7 байт.
       Каждая запись таблицы адрес 3 байта : значение 4 байта
       Последняя запись - нуль
     */
    int  initTable()   const { return svIRead32( mProg + SVVMH_INIT_TABLE ); }




    /*!
       \brief enterPoint Адрес точки входа в программу. Это адрес функции main
       \return Адрес функции main
     */
    int  enterPoint()  const { return svIRead32( mProg + SVVMH_ENTER_POINT ); }




    /*!
       \brief stackSize0 Возвращает размер стека для стартовой задачи
       \return Размер стека для начальной задачи
     */
    int  stackSize0()  const { return svIRead32( mProg + SVVMH_STACK_SIZE0 ); }



    /*!
       \brief vpuProgSize Возвращает размер чистого кода программы без учета последующих
         таблиц
       \return Размер чистого кода
     */
    int  vpuProgSize() const { return svIRead32( mProg + SVVMH_VM_PROG_SIZE ); }



    /*!
       \brief globalCount Возвращает объем памяти, занятый глобальными переменными
       \return Объем памяти, занятый глобальными переменными
     */
    int  globalCount() const { return svIRead32( mProg + SVVMH_GLOBAL_COUNT ); }




    /*!
       \brief constTable Возвращает начало таблицы константных блоков переменной длины.
       \return Начало таблицы константных блоков

       Каждая запись таблицы состоит из адреса начала блока. Длина записи 4 байта.
       Размер блока определяется в самом блоке. Константными блоками представлены
       изображения, звуки, текстовые строки и т.п.
     */
    int  constTable() const { return svIRead32( mProg + SVVMH_CONST_TABLE ); }



    /*!
       \brief constBlock Возвращает адрес константного блока по его индексу
       \param index Индекс константного блока
       \return Адрес начала константного блока
     */
    int  constBlock( int index ) { return svIRead24( mProg + constTable() + index * 4 ); }



    /*!
       \brief progSize Возвращает размер памяти программ включая все дополнительные таблицы
       \return Размер всей программы
     */
    int  progSize()    const { return svIRead32( mProg + SVVMH_PROG_SIZE ); }






    //===========================
    //Раздел управления отладкой


    /*!
       \brief debugRun Отладка - пуск
       \param vpu Виртуальный процессор, для которого предназначена команда

       Выставляет флаг, что программа в состоянии работы. Поэтому в следующем цикле обработки
       программа данного процессора начнет выполняться
     */
    void debugRun( int vpu );



    /*!
       \brief debugPause Отладка - стоп (пауза)
       \param vpu Виртуальный процессор, для которого предназначена команда

       Выставляет флаг, что программа в состоянии паузы. В последующих циклах обработки
       программа данного процессора исполняться не будет до тех пор, пока
       не будет выставлен флаг пуск.
     */
    void debugPause( int vpu );




    /*!
       \brief debugRunTrace Выполнить один "шаг" программы с заходом в функции
       \param vpu Виртуальный процессор, для которого предназначена команда
       \param start Начальный адрес
       \param stop Конечный адрес

       Виртуальный процессор будет находиться в состоянии исполнения, пока
       его указатель программ будет в заданном диапазоне адресов. Диапазоном
       адресов отмечается величина шага, который необходимо выполнить. Эта
       величина берется из набора команд, представляющих одну строку
       исходного кода.

       Как только указатель программ выйдет из заданного диапазона, то программа
       переходит в состояние паузы.
     */
    void debugRunTrace( int vpu, int start, int stop );



    /*!
       \brief debugRunStep Выполнить один "шаг" программы без захода в функции
       \param vpu Виртуальный процессор, для которого предназначена команда
       \param start Начальный адрес
       \param stop Конечный адрес

       Виртуальный процессор будет находиться в состоянии исполнения, пока
       его указатель программ будет в заданном диапазоне адресов ИЛИ указатель bp
       будет меньше, чем был на момент вызова данной функции. Диапазоном
       адресов отмечается величина шага, который необходимо выполнить. Эта
       величина берется из набора команд, представляющих одну строку
       исходного кода.

       Условие с указателем bp обеспечивает выполнение всех вложенных функций,
       вне зависимости от диапазона адресов.

       Как только управление программы выйдет из всех вложенных функций и
       указатель программ выйдет из заданного диапазона, то программа
       переходит в состояние паузы.

     */
    void debugRunStep( int vpu, int start, int stop );



    //Исполнять, пока находится вне диапазона
    /*!
       \brief debugRunUntil Задать диапазон останова (break point)
       \param vpu Виртуальный процессор, для которого предназначена команда
       \param start Начальный адрес
       \param stop Конечный адрес

       Виртуальный процессор будет находиться в состоянии исполнения, пока
       его указатель программ не окажется внутри заданного диапазона адресов.
       При этом процессор перейдет в состоянии паузы.
     */
    void debugRunUntil( int vpu, int start, int stop );





    /*!
       \brief memSet Установить значение в память данных
       \param vpu Виртуальный процессор, затребовавший запись
       \param index Номер ячейки памяти, в которую выполняется запись
       \param value Записываемое значение
     */
    virtual void memSet( SvVmVpu *vpu, int index, int value ) {
      if( index > 0 && index < mMemorySize )
        mMemory[index] = value;
      else memFail( vpu, index );
      }





    /*!
       \brief memGet Получить значение из памяти данных
       \param vpu Виртуальный процессор, затребовавший данные
       \param index Номер ячейки памяти
       \return Значение из требуемой ячейки или ноль при нарушении границ памяти
     */
    virtual int memGet( SvVmVpu *vpu, int index ) const {
      if( index > 0 && index < mMemorySize )
        return mMemory[index];
      memFail( vpu, index );
      return 0;
      }


  protected:
            /*!
               \brief executeCore Вычислительное ядро. Выполняет блок команд для
                  одного вируального процессора до момента переключения задач
               \param vpu Виртуальный процессор, для которого нужно выполнить блок команд
             */
            void executeCore( SvVmVpu *vpu );




            /*!
               \brief callInternal Выполняет вызов функции скрипта
               \param vpu Виртуальный процессор, с помощью которого нужно выполнить вызов
               \param addr Адрес функции
               \param retAddr Адрес возврата
             */
            void callInternal( SvVmVpu *vpu, int addr, int retAddr );



            /*!
               \brief call Универсальный вызов подпрограммы. Это может быть вызов
                  обычной функции из скрипта или вызов внешней функции,
                  обработка которой осуществляется с помощью executeMethod
               \param vpu Виртуальный процессор, осуществляющий вызов
               \param addrOffset по смещению addrOffset относительно sp содержится адрес вызова
             */
            void call( SvVmVpu *vpu, int addrOffset );




            /*!
               \brief doThrow Обработка исключения
               \param vpu Виртуальный процессор, вызвавший исключение
               \param mask Возникшее исключение

               Обработка исключения заключается в последовательном возврате из
               вложенных функций до тех пор пока возникшее исключение объединенное по И
               с маской обрабатываемых исключений не даст ненулевой результат или
               пока не будет достигнута функция задачи. Для стартовой задачи - это
               функция main
             */
            void doThrow( SvVmVpu *vpu, int mask );




            /*!
               \brief value1 Возвращает однобайтовый параметр по заданному адресу
               \param vpu Виртуальный процессор, которому нужен этот параметр
               \param index Адрес параметра в памяти программ
               \return Параметр, расширенный до целого
             */
            int value1( SvVmVpu *vpu, int index ) {
              //Проверка границ доступа к памяти, вызов обработчика ошибок при нарушении
              if( index < 0 || index >= mProgSize ) {
                progFail( vpu, index, 1 );
                return 0;
                }
              return (signed char)(mProg[index]);
              }



            /*!
               \brief value2 Возвращает двухбайтовый параметр по заданному адресу
               \param vpu Виртуальный процессор, которому нужен этот параметр
               \param index Адрес параметра в памяти программ
               \return Параметр, расширенный до целого
             */
            int value2( SvVmVpu *vpu, int index ) {
              //Проверка границ доступа к памяти, вызов обработчика ошибок при нарушении
              if( index < 0 || index + 1 >= mProgSize ) {
                progFail( vpu, index, 2 );
                return 0;
                }
              return svIRead16( mProg + index );
              }




            /*!
               \brief value3 Возвращает трехбайтовый параметр по заданному адресу
               \param vpu Виртуальный процессор, которому нужен этот параметр
               \param index Адрес параметра в памяти программ
               \return Параметр, расширенный до целого
             */
            int value3( SvVmVpu *vpu, int index ) {
              //Проверка границ доступа к памяти, вызов обработчика ошибок при нарушении
              if( index < 0 || index + 2 >= mProgSize ) {
                progFail( vpu, index, 3 );
                return 0;
                }
              return svIRead24( mProg + index );
              }




            /*!
               \brief value4 Возвращает четырехбайтовый параметр по заданному адресу
               \param vpu Виртуальный процессор, которому нужен этот параметр
               \param index Адрес параметра в памяти программ
               \return Параметр, расширенный до целого
             */
            int value4( SvVmVpu *vpu, int index ) {
              //Проверка границ доступа к памяти, вызов обработчика ошибок при нарушении
              if( index < 0 || index + 3 >= mProgSize ) {
                progFail( vpu, index, 4 );
                return 0;
                }
              return svIRead32( mProg + index );
              }
  };

//Макросы для упрощенной записи доступа к параметрам внешних методов и их результатам
#define VPU_GET_FUN_PARAM( idx ) memGet( vpu, vpu->mSp + (idx) )
#define VPU_SET_FUN_RESULT( idx, val ) memSet( vpu, vpu->mSp + (idx), val )
//Пример реализации
//1             0
//int  Abs( int val );
//VPU_SET_FUN_RESULT( 1, abs(VPU_GET_FUN_PARAM(0)) );


#endif // SVCONTROLLER_H
