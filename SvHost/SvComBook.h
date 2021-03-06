/*
  Проект     "Скриптовый язык SaliScript: упрощенный c++"
  Подпроект  "Виртуальная машина на микроконтроллерах"

  Автор
    Сибилев А.С.
  Описание
    Здесь описан протокол взаимодействия виртуальной машины встроенной
    в микроконтроллер с внешними программами.

    База взаимодействия - COM порт, в том числе его реализация через USB.
    Данный подход избавляет от необходимости установки драйвера, что упрощает
    эксплуатацию.

    Для обеспечения взаимодействия со строками все данные перекодируются в текст.

*/
#ifndef SVCOMBOOK_H
#define SVCOMBOOK_H

#define SV_CB_INFO_GET   'I' //Запросить информацию о контроллере. Выдается информация о контроллере
#define SV_CB_INFO       'i' //Информация о контроллере
                             // i8[20] - тип контроллера
                             // i8     - количество доступных процессоров
                             // i32[3] - серийный номер процессора
                             // i32    - hash-значение загруженной программы
                             // i8[20] - наименование загруженной программы



#define SV_CB_RESET      'R' //Сброс контроллера. Выдается состояние извещения
                             // i8 - 1 - запустить, 0 - пауза



#define SV_CB_VPU_GET    'V' //Запросить состояние виртуального процессора. Выдается состояние виртуального процессора
                             // i8 - номер виртуального процессора, состояние которого передается
#define SV_CB_VPU        'v' //Состояние виртуального процессора
                             // i8 - номер виртуального процессора, состояние которого передается
                             // i32 - ip указатель инструкций
                             // i32 - sp указатель стека
                             // i32 - bp указатель базы в стеке локальных переменных для текущей функции (указывает на фрейм возврата из функции)
                             // i32 - tm маска обрабатываемых исключений
                             // i32 - baseSp Начало стека для данного процессора
                             // i32 - throw  Текущее значение исключения
                             // i32 - debugRun if eq 0 then in debug state else - in run state



#define SV_CB_DEBUG      'D' //Команда отладчика. Без подтверждения
                             // i8  - номер виртуального процессора
                             // i8  - команда
                             // i32 - параметр 1
                             // i32 - параметр 2




#define SV_CB_MEMORY_GET 'M' //Запросить блок памяти. Выдается блок памяти
                             // i16     - начальный адрес блока памяти
#define SV_CB_MEMORY     'm' //Блок памяти
                             // i16     - начальный адрес блока памяти
                             // i32[12] - блок памяти из 12 ячеек



#define SV_CB_MEMORY_SET 'W' //Записать ячейки памяти. Без подтверждения
                             // i8 - количество пар значений
                             // пары[1-8] i16:i32 - адрес ячейки:значение



#define SV_CB_LOG_GET    'L' //Запросить состояние лога. Выдается состояние лога
#define SV_CB_LOG        'l' //Состояние лога
                             // строка с текстом



#define SV_CB_TICKET_GET 'T' //Запросить состояние извещения. Выдается состояние извещения
#define SV_CB_TICKET     't' //Состояние извещения
                             // i16    - 0-извещения нету, иначе номер извещения
                             // i32[8] - параметры извещения, если извещения нету, то параметры не передаются


#define SV_CB_ERASE      'E' //Стереть память. Выдается состояние занятости



#define SV_CB_FLASH      'F' //Прошить блок памяти. Выдается состояние занятости
                             // i32 - адрес начала блока
                             // i8  - количество байт блока
                             // i8[1-48] - блок данных



#define SV_CB_BUSY_GET   'B' //Получить состояние занятости. Выдается состояние занятости
#define SV_CB_BUSY       'b' //Состояние занятости
                             // i8 - 0 - свободен, иначе - занят




#endif // SVCOMBOOK_H
