LCD driver port for STM32F10x microcontrollers
==============================================


**driver\hd44780.c** 

Мой порт драйвера для символьного LCD-дисплея (на широкоизвестном контроллере HD44780 или аналогах) под микроконтроллеры STM32.
Автор драйвера: Artem Borisovskiy (bytefu@gmail.com) [2012]
Я внёс незначительные исправления, и написал HAL-адаптер для связи с STM32F10x.

Особенности драйвера: поддерживаются любые размеры символьных LCD (до 4 строк, и до 24 символов).
Интерфейс связи с LCD-модулем: параллельная шина 4/8-пин. 
Реализация передачи данных: программная, с блокирующей задержкой! Таким образом, загрузка данных довольно тормознутая (до 20мс на один Refresh дисплея, по 4-битной шине...)

::

        //printf("Display_Show_ControlQuantity END: saveEepromTimeout = %d, PreviousWakeTime = %d, SysTick = %d\n", saveEepromTimeout, PreviousWakeTime, HAL_GetTick());   //PROFILER: тестируем тайминги
        //  Результат профилирования: 
        //    при 4-битной параллельной шине данных - вывод на дисплей, отработка Display_Show_ControlQuantity, длится 18мс! 
        //    при 8-битной параллельной шине данных - вывод на дисплей, отработка Display_Show_ControlQuantity, длится  9мс! 
        //    Довольно долго - это потому, что внутри LCD-драйвера работа с параллельной шиной осуществляется ВРУЧНУЮ ("ногодрыгом") и тайминги сигналов отмеряются БЛОКИРУЮЩЕ.
        //  Вывод: 
        //    при 4-битной шине данных, нельзя ставить "точную задержку" (osDelayUntil), в циклах с выводом на Дисплей, меньше 25мс. 
        //    при 8-битной шине данных, нельзя ставить "точную задержку" (osDelayUntil), в циклах с выводом на Дисплей, меньше 15мс. 
        //    Иначе, система станет подтормаживать: низкоприоритетные Задачу станут блокироваться!
        //  TODO: возможно, следует перевести связь с LCD (работу с параллельной шиной) на аппаратный модуль FSMC? 
        //    Хотя, это сложное и долгое решение, требующее серьезно перерабатывать код драйвера LCD (нерентабельно). 
        //    Да, и результат получится неуниверсальным (привязанным к аппаратной шине FSMC, и к специфической адресации - что не гарантирует гладкой переносимости даже между микроконтроллерами STM32, со встроенным модулем FSMC, соседних семейств)...
        //    Кроме того, у меня есть сомнения, что это ускорит сам обмен с модулем LCD: ядро микроконтроллера-то может и разгрузим, вот только воспользоваться этим преимуществом вряд ли получится... По шине данных биты все равно пройдут с задержкой, заданной спецификацией модуля. А в пакете - много байт... Вероятно, и выйдем на те же тайминги!
        //  Решение: распаял еще четыре провода шины данных, и подключил LCD-модуль по 8-битной шине данных - связь стала в 2 раза быстрее (замерено эмпирически)!




**example\display.c**

Пример использования драйвера LCD: 

Модуль "display.c" реализует функции рендеры "прикладных Экранов", отобрающиеся на Дисплее. Функция принимает набор прикладных данных, которые конвертирует в форматированные строки и выводит на дисплей. Принцип каждой функции - атомарность: одна отработка - один полный Refresh дисплея...

API модуля, для каждой "экранной функции" предоставляет Макрос-обёртку, реализующий "необязательные параметры" (которые отсутствуют в нативном Си синтаксисе, но хитрым приёмом возможны для использования). Данный принцип построения API функция - гибкий и наращиваемый. [Идея взята отсюда...] (<http://we.easyelectronics.ru/Soft/inicializaciya-periferii-s-pomoschyu-imenovannyh-argumentov.html>)

Требования: Модуль "display.c" требует наличие (предлагает использовать) FreeRTOS (или другую RTOS). Без RTOS приличную прошивку не написать - всё быстро превратиться в говнокод. 
Но в принципе, этот пример легко адаптировать и без использования RTOS: там только Мьютексы закомментировать в секции "Разделение доступа к Дисплею (API модуля)"...




**delays\delays.c**

Библиотека "delays.c" включёна по зависимостям... Используется для реализации синхронных (блокирующих) задержек порядка нс, в протоколе обмена с LCD-дисплеем.
Original library taken from Easyrider83 (c) http://kazus.ru/forums/showpost.php?p=524970&postcount=14
Библиотека была мною слегка исправлена и дополнена... А также, протестирована (нормально работает, для своей ниши).

Замечу: эту Библиотеку необходимо калибровать (подстраивать значение константы "K_Const" в "delays.h") под конкретный Микроконтроллер (набор инструкций), компилятор (и конкретные установки оптимизации), и тактовую частоту MPU! 
Используйте метод delay_test() и аппаратный осциллограф/логический анализатор для калибровки...

