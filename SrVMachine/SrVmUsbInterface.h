/*
              VPU microcontroller system
                    VMS system
  Author
    Alexander Sibilev
  Internet
     www.saliLab.ru
     www.saliLab.com
 
  Description
    Интерфейс с программируемым контроллером.
  
    Обмен с контроллером строится по принципу запрос-ответ. Ведущим выступает
    персональный компьютер (планшет). Ведущий отсылает запросы, на которые
    контроллер отправляет ответы.
*/
#ifndef SV_VPU_USB_INTERFACE_H
#define SV_VPU_USB_INTERFACE_H

//В качестве запроса посылается код (один байт), который определяет
//тип запроса, размер данных передаваемых в запросе и тип и размер данных
//в ответе.

//Справочные
#define SVU_CMD_VERSION      1 //Получить версию ПО
                               //данные - нет
                               //ответ [0-3]   номер версии
                               //      [4-7]   размер памяти программ
                               //      [8-11]  размер памяти данных
                               //      [12]    тип контроллера (процессор и количество слотов), из этой информации определяются возможности контроллера
                               //      [13]    максимальное количество VPU
                               //      [14]    фактическое количество активных vpu
                               //      [15]   
                               //      [16-23] сигнатура установленной программы
                               //      [24-27] размер прошитой программы
                               //      [28-31] количество глобальных переменных
                               //      [31]    SVU_ANS_OK

#define SVU_CMD_LED          3 //Переключить светодиод
                               //данные [0] - код состояния светодиода (0 - выключить, 1 - включить)
                               //ответ SVU_ANS_OK

#define SVU_CMD_STATE        4 //Получить состояние контроллера
                               //данные - нет
                               //ответ [0] - количество активных vpu,
                               //      [1] - SVU_ANS_OK или SVU_ANS_BUSY




//Работа с памятью программ
#define SVU_CMD_FLASH_STATE 10 //Получить состояние операций памяти (операция выполняется - память занята, память свободна)
                               //Данных нет
                               //ответ SVU_ANS_OK или SVU_ANS_BUSY

#define SVU_CMD_FLASH_ERASE 11 //Стереть память программ
                               //Данных нет
                               //ответ SVU_ANS_OK или SVU_ANS_BUSY

#define SVU_CMD_FLASH_READ  12 //Прочитать блок данных из Flash
                               //данные [0-3]   (4 байта) количество читаемых данных
                               //       [4-7]   (4 байта) адрес начала чтения относительно начала программ
                               //ответ блок данных памяти программ

#define SVU_CMD_FLASH_WRITE 13 //Записать блок данных во Flash
                               //данные [0-3]   (4 байта) адрес блока (относительно начала программы)
                               //       [4-59]  (56 байт) блок данных для записи
                               //ответ SVU_ANS_OK или SVU_ANS_BUSY

#define SVU_CMD_FLASH_HEAD  14 //Записать заголовочную информацию во Flash
                               //данные [0-7]   (8 байт)  сигнатура программы (имя)
                               //       [8-11]  (4 байта) адрес начала таблицы инициализации
                               //       [12-15] (4 байта) адрес точки входа для автоматически стартуемой программы
                               //       [16-19] (4 байта) размер стека для начального исполнения
                               //       [20-23] (4 байта) размер программы
                               //       [24-27] (4 байта) количество глобальных переменных
                               //       [28-31] (4 байта) резерв
                               //ответ SVU_ANS_OK или SVU_ANS_BUSY

#define SVU_HEAD_SIZE       32 //Размер заголовка программы





//Работа с памятью данных
#define SVU_CMD_VARS_READ   20 //Прочитать значения переменных
                               //данные [0-3]   (4 байта) количество читаемых ячеек
                               //       [4-7]   (4 байта) индекс начальной ячейки для чтения
                               //ответ блок данных с переменными

#define SVU_CMD_VARS_WRITE  21 //Записать значения в переменные
                               //данные [0] - кол-во пар адрес-значение (максимум 10)
                               //       [1-2] адрес
                               //       [3-6] значение
                               //       ...
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY




#define SVU_CMD_VPU_STATE   30 //Количество и состояние VPU
                               //данные - нет
                               //ответ [0] - количество VPU
                               //      [1-62] - состояния VPU (0-halt, 1-running, 2-paused, 3-vpu not used)
                               //      [63] - SVU_ANS_OK
//Состояние VPU
#define VPU_STATE_HALT       0 //VPU остановлено
#define VPU_STATE_RUNNING    1 //VPU работает
#define VPU_STATE_PAUSED     2 //VPU приостановлено по отладке
#define VPU_STATE_EMPTY      3 //VPU не используется



#define SVU_CMD_VPU_READ    31 //Текущее значение IP заданных VPU
                               //данные [0] номер начального VPU
                               //ответ  [1] количество VPU
                               //       блоки по 64 байта для каждого VPU с состоянием

#define SVU_CMD_VPU_RESET   32 //Все очистить и убрать все виртуальные процессоры

#define SVU_CMD_VPU_RESTART 33 //Перезапустить текущую программу с заданными флагами отладки
                               //данные [0] запускать без отладки (1-без отладки, 0-с отладкой)
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY

#define SVU_CMD_DEBUG_RUN   34 //Запустить на исполнение заданный VPU
                               //данные [0] номер VPU
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY

#define SVU_CMD_DEBUG_PAUSE 35 //Остановить заданный VPU
                               //данные [0] номер VPU
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY

#define SVU_CMD_DEBUG_TRACE 36 //Трассировка (шаг со входом в пп) для заданного VPU
                               //данные [0] номер VPU
                               //       [1-4] нижняя граница
                               //       [5-8] верхняя граница
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY
                               
#define SVU_CMD_DEBUG_STEP  37 //Шаг для заданного VPU
                               //данные [0] номер VPU
                               //       [1-4] нижняя граница
                               //       [5-8] верхняя граница
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY
                               
#define SVU_CMD_DEBUG_UNTIL 38 //Выполнять до условия для заданного VPU
                               //данные [0] номер VPU
                               //       [1-4] нижняя граница
                               //       [5-8] верхняя граница
                               //ответ PLC_ANS_OK или PLC_ANS_BUSY
                               

//Ответ-подтверждение
#define SVU_ANS_OK        0x7e
//Ответ-ошибка операции
#define SVU_ANS_ERROR     0x11
//Ответ-устройство занято, последняя операция должна быть повторена
#define SVU_ANS_BUSY      0xc2


#endif