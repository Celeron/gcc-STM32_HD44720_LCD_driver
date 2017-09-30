/******************************************************************************
 * Модуль:      display.c
 * Автор:       Celeron (c) 2017
 * Назначение:  Данный модуль служит адаптером для генерации изображения на Дисплей. 
 *              Реализовывает специфическое поведение для конкретного LCD-модуля: подстраивает кодировку символов, располагает строки на экране в зависимости от размеров области вывода и т.п.
 *              Собственного рабочего потока (Display_Run) [пока] не содержит, поскольку Автообновление и Анимация на этом простом Дисплее не нужны.
 ******************************************************************************/


#include "stm32f1xx_hal.h"          // подключаем HAL API
#include "cmsis_os.h"               // подключаем RTOS API

#include "hd44780.h"                // подключаем LCD
#include "hd44780_stm32f1xx_hal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "display.h"                // прототипы локальных функций и типов




//============================================================================
// Подключение и Инициализация Дисплея (аппаратная часть)
//============================================================================


//Синглтоны-переменные содержащие данные интерфейса
HD44780 lcd;
HD44780_STM32F10x_GPIO_Driver lcd_pindriver;



//-------------------------------------
//заглушка для согласования интерфейсов
void assert_failed(uint8_t* file, uint32_t line);   // Note: function defined in "main.c"...
void hd44780_assert_failure_handler(const char *filename, unsigned long line)
{
#ifdef  USE_FULL_ASSERT
  assert_failed((uint8_t*) filename, line);
#endif
}



//-------------------------------------
//заглушка для согласования интерфейсов
#include "delays.h"   // Original library taken from Easyrider83 (c) http://kazus.ru/forums/showpost.php?p=524970&postcount=14
void hd44780_delay_microseconds(uint16_t us)
{
  delay_us((uint32_t) us);
  
//  // Грубая (гораздо более тормознутая), но неблокирующая выдержка...
//  // Внимание: эксперимент! неотлажена!
//  uint32_t ms = us/1000;
//  if (ms==0) 
//      ms = 1;
//  osDelay(ms);
}



//-------------------------------------
// Инициализация аппаратной части (LCD-драйвера и контроллера LCD-модуля)
void Display_InitLCD(void)
{
  /* Распиновка дисплея */
  const HD44780_STM32F10x_Pinout lcd_pinout =
  {
    {
      /* RS        */  { GPIOD, GPIO_PIN_11 },
      /* ENABLE    */  { GPIOD, GPIO_PIN_7 },
      /* RW        */  { GPIOD, GPIO_PIN_5 },
      /* Backlight */  { GPIOE, GPIO_PIN_2 },    // Вариант отключено: { NULL, 0 },
      /* DP0       */  { GPIOD, GPIO_PIN_14 },
      /* DP1       */  { GPIOD, GPIO_PIN_15 },
      /* DP2       */  { GPIOD, GPIO_PIN_0 },
      /* DP3       */  { GPIOD, GPIO_PIN_1 },
      /* DP4       */  { GPIOE, GPIO_PIN_7 },
      /* DP5       */  { GPIOE, GPIO_PIN_8 },
      /* DP6       */  { GPIOE, GPIO_PIN_9 },
      /* DP7       */  { GPIOE, GPIO_PIN_10 },
    }
  };

  /* Настраиваем драйвер: указываем интерфейс драйвера (стандартный),
     указанную выше распиновку и обработчик ошибок GPIO (необязателен). */
  lcd_pindriver.interface = HD44780_STM32F10X_PINDRIVER_INTERFACE;
  //lcd_pindriver.interface.configure = NULL;   // Автор библиотеки пишет: Если вдруг захотите сами вручную настраивать GPIO для дисплея (зачем бы вдруг), то напишите здесь =NULL (библиотека учтёт это)... 
                                                // Внимание: НЕ использовать! Глючит! Если присвоить здесь NULL, то начинается большой глюкодром и ничего не работает (возможно, библиотека неотлажена для этого режима?)
  lcd_pindriver.pinout = lcd_pinout;
  lcd_pindriver.assert_failure_handler = hd44780_assert_failure_handler;

  /* И, наконец, создаём конфигурацию дисплея: указываем наш драйвер,
     функцию задержки, обработчик ошибок дисплея (необязателен) и опции.
     На данный момент доступны две опции - использовать или нет
     вывод RW дисплея (в последнем случае его нужно прижать к GND),
     и то же для управления подсветкой. */
  const HD44780_Config lcd_config =
  {
    (HD44780_GPIO_Interface*)&lcd_pindriver,
    hd44780_delay_microseconds,
    hd44780_assert_failure_handler,
    HD44780_OPT_USE_RW
  };

  /* Ну, а теперь всё стандартно: подаём тактирование на GPIO,
     инициализируем дисплей: 16x2, 8-битный интерфейс, символы 5x8 точек. */
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  //hd44780_init(&lcd, HD44780_MODE_8BIT, &lcd_config, 16, 2, HD44780_CHARSIZE_5x8);  // Вариант для: Winstar WH1602L-YGH-CT (Cutter) http://www.kosmodrom.com.ua/el.php?name=WH1602L-YGH-CT
  hd44780_init(&lcd, HD44780_MODE_8BIT, &lcd_config, 20, 4, HD44780_CHARSIZE_5x8);    // Вариант для: Winstar WH2004L-TFH-CT (Merger) http://www.kosmodrom.com.ua/el.php?name=WH2004L-TFH-CT
  // Как вариант: 4-битный интерфейс...
  //hd44780_init(&lcd, HD44780_MODE_4BIT, &lcd_config, 16, 2, HD44780_CHARSIZE_5x8);
  
  
  // Наконец, определим свои символы в памяти контроллера:
  //  Функция hd44780_create_char() создаёт символ с заданным ASCII-кодом от 0 до 7, используя битовое описание символа в виде массива из 8 байтов, каждый из которых кодирует строку из 5 точек.
  //  Примечание: символ с кодом #0 обозначает конец строки в Си и использоваться не будет! Таким образом, спецсимволы следует регистрировать на коды, начиная с #1...
  static const uint8_t charmap1[8] =
  {
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(1,1,1,1,1),
    HD44780_MAKE_5BITS(0,0,0,0,0),
  };
  hd44780_create_char(&lcd, 1, charmap1);  // зарегистрируем символ с кодом #1 (элемент полосы прогресса)

  static const uint8_t charmap2[8] =
  {
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,1,0,0),
    HD44780_MAKE_5BITS(0,0,0,0,0),
  };
  hd44780_create_char(&lcd, 2, charmap2);  // зарегистрируем символ с кодом #2 (вертикальная полоса)

  static const uint8_t charmap3[8] =
  {
    HD44780_MAKE_5BITS(1,0,0,0,0),
    HD44780_MAKE_5BITS(1,1,0,0,0),
    HD44780_MAKE_5BITS(1,1,1,0,0),
    HD44780_MAKE_5BITS(1,1,1,1,0),
    HD44780_MAKE_5BITS(1,1,1,0,0),
    HD44780_MAKE_5BITS(1,1,0,0,0),
    HD44780_MAKE_5BITS(1,0,0,0,0),
    HD44780_MAKE_5BITS(0,0,0,0,0),
  };
  hd44780_create_char(&lcd, 3, charmap3);  // зарегистрируем символ с кодом #3 (жирная стрелка >)

  static const uint8_t charmap4[8] =
  {
    HD44780_MAKE_5BITS(0,0,0,0,1),
    HD44780_MAKE_5BITS(0,0,0,1,1),
    HD44780_MAKE_5BITS(0,0,1,1,1),
    HD44780_MAKE_5BITS(0,1,1,1,1),
    HD44780_MAKE_5BITS(0,0,1,1,1),
    HD44780_MAKE_5BITS(0,0,0,1,1),
    HD44780_MAKE_5BITS(0,0,0,0,1),
    HD44780_MAKE_5BITS(0,0,0,0,0),
  };
  hd44780_create_char(&lcd, 4, charmap4);  // зарегистрируем символ с кодом #4 (жирная стрелка <)
}




//============================================================================
// Разделение доступа к Дисплею (API модуля)
//============================================================================

// Обеспечение multithreading-safe доступа к Дисплею
// Мютекс защищающий атомарность вызова API-функций: пока не отработает одна - запуск другой функции задерживается  (механизм синхронизации разных потоков, при обращении к разделяемому ресурсу)
osMutexId DisplayMutex;



//-------------------------------------
// Ко всем API-методам следует приделать Мьютекс, предупреждающий и разделяющий одновременный вывод на Дисплей из разных Потоков 
// (иначе, при смешении выводимых сигналов, случается глюк)...

#define GET_DISPLAY_MUTEX()                           \
  if(Display_Lock() != osOK) return;  /* Если не дождался доступа, то ничего не делаем... */

#define RELEASE_DISPLAY_MUTEX()                       \
  Display_UnLock();



//-------------------------------------
// Захватить разделяемый ресурс (Дисплей)
osStatus Display_Lock(void)
{
  osStatus status = osMutexWait(DisplayMutex, 50);  // GET_SOUND_MUTEX
  assert_param(status == osOK);
  return status;
}



//-------------------------------------
// Освободить разделяемый ресурс (Дисплей)
osStatus Display_UnLock(void)
{
  osStatus status = osMutexRelease(DisplayMutex);   // RELEASE_SOUND_MUTEX
  assert_param(status == osOK);
  return status;
}




//============================================================================
// Инициализация модуля (служебные методы)
//============================================================================


/*
// Убрал (закомментировал) цикл обработки событий Display_Run, поскольку Автообновление и Анимация на этом простом Дисплее не нужны! Упрощаем...
//-------------------------------------
// Дескрипторы Потоков
osThreadId Display_Run_ThreadId;

//-------------------------------------
// Главный цикл обработки событий вывода на Дисплей
// (Назначение: реализует анимацию, съем показаний с датчиков или внутренних переменных, автообновление индикаторов на Дисплее...)
void Display_Run(void const * argument)
{
  // Infinite loop
  for(;;)
  {
    osDelay(1);
  }
}
*/


//-------------------------------------
// Инициализация модуля
void Display_Init(void)
{
  // Инициализация аппаратной части (LCD-драйвера и контроллера LCD-модуля)
  Display_InitLCD();

  // Включить подсветку
  hd44780_backlight_on(&lcd);
  
  // Мютекс защищающий атомарность вызова API-функций
  DisplayMutex = osMutexCreate(NULL);
  assert_param(DisplayMutex != NULL);
  
//  // Запуск Рабочих Потоков
//  osThreadDef(Display_Run, Display_Run, osPriorityNormal, 0, 128);
//  Display_Run_ThreadId = osThreadCreate(osThread(Display_Run), NULL); // Главный цикл обработки событий вывода на Дисплей
//  assert_param(Display_Run_ThreadId != NULL);  
}




//============================================================================
// Конвертация данных
//============================================================================


// Важно: на вход Display_EncodeString может поступать строка не только из буфера в ОЗУ (изменяемые данные), но и адрес литерала во FLASH (неизменяемые данные)!
// Поэтому данные не могут быть отконвертированы inplace - необходимо заполнять отдельный буфер (копировать туда данные) и возвращать его...

// Буфер для исходящих данных - выделяем статически, в глобальном адресном пространстве. 
//    Размер буфера: длина конвертируемых строк не будет превышать 40 символов (длину строки дисплея) + 1 символ '\0'.
//    Примечание: в Си, для глобальных переменных модуля, static похож на модификатор доступа private, работающий на уровне файла... (c) http://digitalchip.ru/osobennosti-ispolzovaniya-extern-i-static-v-c-c
static char EncodedString[HD44780_COLUMNS_AMOUNT_MAX + 1];


// Внимание: при каждом следующем вызове метода Display_EncodeString, содержимое выходного буфера EncodedString будет перезаписываться! 
// Поэтому, порядок пользования Метода таков: отконвертировал строку - и тут же отправил на вывод... отконвертировал - вывел... 
// Например: hd44780_write_string(&lcd, Display_EncodeString(s1));


// Рецепт: нестандартный символ в строке можно задать через ESC-последовательность: \КОД (восьмеричный код символа, три цифры)... Например: char *s1 = "Символ тильда 0xED = \351";
// НО УЧТИ: если далее строка форматирования пройдет через фильтр Display_EncodeString(s1), то все нестандартные символы в ней перехерятся! 
// (Вывод: не используй кодовых последовательностей в исходной строке! А вместо этого, расширяй таблицу перекодировок в методе Display_EncodeString.)



//-------------------------------------
// Метод Display_EncodeString конвертирует текстовую строку в кодировку, поддерживаемую LCD-контроллером HD44780 (для вывода русских символов)
//  Возвращает: указатель на буфер с преобразованной строкой (null-terminated).
char *Display_EncodeString(char *s)
{
  char *output = EncodedString;         // Инициализация
  
  do                                    // Цикл: пока не достигли конца строки...
  {
    switch (*s)                         // Примечание: *s означает значение ячейки по данному адресу (операция разыменования указателя), т.е. код символа.
    {
      // Примечания:
      //  Кодировка подходит для Winstar WH1602L-YGH-CT (Cutter) http://www.kosmodrom.com.ua/el.php?name=WH1602L-YGH-CT
      //  Кодировка подходит для Winstar WH2004L-TFH-CT (Merger) http://www.kosmodrom.com.ua/el.php?name=WH2004L-TFH-CT
      //  Полная "Таблица Символов для контроллера HD44780 (руссифицированный)" см. на http://cxem.net/mc/book52.php или в datasheet...
      case 'а':  *output = 97;   break; 
      case 'б':  *output = 178;  break;
      case 'в':  *output = 179;  break;
      case 'г':  *output = 180;  break;
      case 'д':  *output = 227;  break;
      case 'е':  *output = 101;  break;
      case 'ё':  *output = 181;  break;
      case 'ж':  *output = 182;  break;
      case 'з':  *output = 183;  break;
      case 'и':  *output = 184;  break;
      case 'й':  *output = 185;  break;
      case 'к':  *output = 186;  break;
      case 'л':  *output = 187;  break;
      case 'м':  *output = 188;  break;
      case 'н':  *output = 189;  break;
      case 'о':  *output = 111;  break;
      case 'п':  *output = 190;  break;
      case 'р':  *output = 112;  break;
      case 'с':  *output = 99;   break;
      case 'т':  *output = 191;  break;
      case 'у':  *output = 121;  break;
      case 'ф':  *output = 228;  break;
      case 'х':  *output = 120;  break;
      case 'ц':  *output = 229;  break;
      case 'ч':  *output = 192;  break;
      case 'ш':  *output = 193;  break;
      case 'щ':  *output = 230;  break;
      case 'ъ':  *output = 194;  break;
      case 'ы':  *output = 195;  break;
      case 'ь':  *output = 196;  break;
      case 'э':  *output = 197;  break;
      case 'ю':  *output = 198;  break;
      case 'я':  *output = 199;  break;
      case 'А':  *output = 65;   break;
      case 'Б':  *output = 160;  break;
      case 'В':  *output = 66;   break;
      case 'Г':  *output = 161;  break;
      case 'Д':  *output = 224;  break;
      case 'Е':  *output = 69;   break;
      case 'Ё':  *output = 162;  break;
      case 'Ж':  *output = 163;  break;
      case 'З':  *output = 164;  break;
      case 'И':  *output = 165;  break;
      case 'Й':  *output = 166;  break;
      case 'К':  *output = 75;   break;
      case 'Л':  *output = 167;  break;
      case 'М':  *output = 77;   break;
      case 'Н':  *output = 72;   break;
      case 'О':  *output = 79;   break;
      case 'П':  *output = 168;  break;
      case 'Р':  *output = 80;   break;
      case 'С':  *output = 67;   break;
      case 'Т':  *output = 84;   break;
      case 'У':  *output = 169;  break;
      case 'Ф':  *output = 170;  break;
      case 'Х':  *output = 88;   break;
      case 'Ц':  *output = 225;  break;
      case 'Ч':  *output = 171;  break;
      case 'Ш':  *output = 172;  break;
      case 'Щ':  *output = 226;  break;
      case 'Ъ':  *output = 173;  break;
      case 'Ы':  *output = 174;  break;
      case 'Ь':  *output = 98;   break;
      case 'Э':  *output = 175;  break;
      case 'Ю':  *output = 176;  break;
      case 'Я':  *output = 177;  break;
      case '~':  *output = 233;  break; // (символ "тильда")
      //case 13:                        // обработка символов переноса строки не производится
      //case 10:
      default:   *output = *s;   break; // все остальные символы - копируем в выходную строку без изменений
    }
    s++;                                // переход к следующему символу
    output++;
  } while( *(s-1)                       // Цикл: пока код [предыдущего, только что обработанного] символа не равен 0...
         && (output - EncodedString) < HD44780_COLUMNS_AMOUNT_MAX );  // или пока не переполнится буфер

  EncodedString[HD44780_COLUMNS_AMOUNT_MAX] = 0;                      // На случай переполнения буфера: вконце прописываем код "null-terminated"...
  
  return EncodedString;                 // Возвращаем ссылку на начало буфера (внимание: здесь нельзя возвращать временный указатель output)
}



//-------------------------------------
// Функция itoa преобразует целое число value (в системе счисления с основанием base) в строку result.
//  Примечание: Функция itoa — широко распространённое нестандартное расширение стандартного языка программирования Си. Ее использование не предусматривает переносимости, поскольку эта функция не определена ни в одном стандарте языка Си; тем не менее, зачастую компиляторы поддерживают ее за счет использования заголовка <stdlib.h>, причем не совсем в удобном виде, так как она весьма близка по смыслу к стандартной библиотечной функции atoi.
//  Альтернативы: Для преобразования числа в строку с основанием 8 (восьмеричная), 10 (десятичная) или 16 (шестнадцатеричная система счисления) альтернативой, совместимой со стандартом, является использование стандартной библиотечной функции sprintf.
/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukбs Chmela
 * Released under GPLv3.
 * http://www.strudel.org.uk/itoa/#newest
 */

char* itoa(int value, char* result, int base) {
  // check that the base if valid
  if (base < 2 || base > 36) { *result = '\0'; return result; }

  char* ptr = result, *ptr1 = result, tmp_char;
  int tmp_value;

  do {
    tmp_value = value;
    value /= base;
    *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
  } while ( value );

  // Apply negative sign
  if (tmp_value < 0) *ptr++ = '-';
  *ptr-- = '\0';
  while(ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr--= *ptr1;
    *ptr1++ = tmp_char;
  }
  return result;
}



//-------------------------------------
// Метод форматирует строку для вывода на дисплей
//  Важно: параметр buf (приемник) должен ссылаться на выделенный буфер в памяти достаточного размера, равный (ширина дисплея + 1 символ для "null-terminated"), т.е. (lcd.columns_amount + 1).
char *Display_FormatString(char *buf, const char *s, uint8_t align)
{
  // сперва, забиваем шаблон пробелами
  memset(buf, ' ', lcd.columns_amount);
  buf[lcd.columns_amount] = 0;                      // Примечание: это очень важный момент - конец буфера null-terminated.

  // определить количество исходных данных
  size_t len = strlen(s);
  if(len > lcd.columns_amount)
    // данные, что вылезают за пределы дисплея по горизонтали, игнорируем
    len = lcd.columns_amount;
  
  // решить позицию, с которой размещается строка
  uint8_t pos = 0;
  if(align)
    pos = (lcd.columns_amount - len)>>1;
  
  // затем, поверх, переписываем исходными данными
  memcpy( buf+pos, s, len);
  
  // Возвращает указатель на buf, что позволяет использовать данный метод как "фильтр" в "конвеере"...
  return buf;
}




//============================================================================
// Универсальные Экраны
//============================================================================


//-------------------------------------
//Очищает одну строку дисплея (забивает пробелами)
void Display_CleanRow(uint8_t row)
{
  hd44780_move_cursor(&lcd, 0, row);
  for(uint8_t i=0; i < lcd.columns_amount; i++)
    hd44780_write_char(&lcd, ' ');
  
  //hd44780_move_cursor(&lcd, 0, row);  //отключено, вследствие оптимизации (не нужно переводить курсор в начало строки)
}



//-------------------------------------
// DEBUG: Тестирование LCD-дисплея (блокирующая глюкалка)
// Отображает: в первой строке надпись "Самотестирование", во второй строке посекундное наращивание счетчика...
// Примечание: реализация данной функции простейшая и не thread-safe!
void Display_TestLCD(void)
{
  //char Msg0[]={67,97,188,111,191,101,99,191,184,112,111,179,97,189,184,101,0};  // здесь написано "Самотестирование", в нативной кодировке LCD-контроллера HD44780
  //char Msg1[]="0123456789ABCDEFGHIJ";

  hd44780_clear(&lcd);
  hd44780_move_cursor(&lcd, 0, lcd.lines_amount - 1);     // положение курсора задается параметрами: (column=0..19, row=0..3)
  hd44780_write_string(&lcd, Display_EncodeString("АВТОТЕСТ ДИСПЛЕЯ"));   // как вариант: "АвтоТест ДиСпЛеЯ"

  /* Infinite loop */
  for(;;)
  {
    for(int y=0; y < lcd.lines_amount; y++)
    {
      Display_CleanRow(y);
      for(int x=0; x < lcd.columns_amount; x++)
      {
        hd44780_move_cursor(&lcd, x, y);
        hd44780_write_char(&lcd, ' ' + y*lcd.columns_amount + x);
        osDelay(100);
      }
    }
  }
}



//-------------------------------------
// Экран "Сообщение" (универсальный)
//  Рассчитан на Дисплей любого размера: 1-2-4 строки, по 16-20-24 символа.
//  Каждой строке соответствует соответствующий параметр: s1, s2, s3, s4.
//  Если некий параметр sx == NULL, то данная строка не дисплее не трогается (даже не очищается, а просто игнорируется)!
//  Если параметром указана null-terminated строка, то она форматируется на всю ширину строки дисплея и выводится.
//  Если параметр AlignCentered == 1, то строки центрируются по горизонтали. Иначе, если AlignCentered == 0 (по умолчанию), то строки LeftAligned.
void Display_Message(Display_Message_Parameters params)
{
  const char *lines[] = {params.Line1, params.Line2, params.Line3, params.Line4};
  uint8_t     align   = params.AlignCentered;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //Поскольку самая длинная строка, поддерживаемая контроллером HD44780 - равна 40 символам.

  GET_DISPLAY_MUTEX();
  
  // Цикл по строкам
  for(uint8_t y=0; y < lcd.lines_amount; y++)
  {
    // Примечание: текущая строка обрабатывается только если указатель на данные ненулевой
    const char *s = lines[y];
    if(s)
    {
      // Метод форматирует строку для вывода на дисплей
      Display_FormatString(buf, s, align);

      // результат вывести на дисплей
      hd44780_move_cursor(&lcd, 0, y);
      hd44780_write_string(&lcd, Display_EncodeString(buf));     
    }
  }
  
  RELEASE_DISPLAY_MUTEX();
}



//-------------------------------------
// Экран "полоса прогресса" 
//  В нижней строке дисплея отображает/обновляет "полосу прогресса", заполненную на Progress процентов = [0..100]
//  Кроме того, в верхней строке отображает некое пояснительное сообщение Title для пользователя...
void Display_ProgressBar(Display_ProgressBar_Parameters params)
{
  const char *Title       = params.Title;
  uint8_t     progress    = params.Progress;
  uint8_t     ClearScreen = params.ClearScreen;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //Поскольку самая длинная строка, поддерживаемая контроллером HD44780 - равна 40 символам.
  char        s[5];
  size_t      count;
  int8_t      row         = lcd.lines_amount/2;           //Номер строки с Полосой. (дисплей->строка: 1->0, 2->1, 3->1, 4->2)

  GET_DISPLAY_MUTEX();

  //Заголовок
  if(row-1 >= 0)
  {
    if(Title)
    {
      Display_FormatString(buf, Title, 1);
      hd44780_move_cursor(&lcd, 0, row-1);
      hd44780_write_string(&lcd, Display_EncodeString(buf));
    }
    else if(ClearScreen)
      Display_CleanRow(row-1);   
  }
  
  //Полоса
  if(progress>100) progress=100;
  count = progress*lcd.columns_amount/100;
  memset(buf,       '\1', count);                         //Примечание: здесь, символ с кодом #1 является кастомным charmap-ом (элемент полосы прогресса, созданный на этапе инициализации дисплея).
  memset(buf+count, ' ',  lcd.columns_amount-count);
  buf[lcd.columns_amount] = 0;

  //Число
  itoa(progress, s, 10);
  count = strlen(s);
  assert_param(count<=3);
  s[count++]='%';                                         //Дописываем символ процента, после числа...
  memcpy(buf+lcd.columns_amount/2-count/2, s, count);

  //Строка была сперва сформирована в памяти (для исключения мерцания) - теперь выводим ее на Дисплей
  hd44780_move_cursor(&lcd, 0, row);
  hd44780_write_string(&lcd, buf);

  //Очистить остальные строки дисплея (если на дисплее больше 2 строк)
  if(ClearScreen)
    for(uint8_t i=0; i<lcd.lines_amount; i++)
      if(i < row-1 || i > row)
        Display_CleanRow(i);

  RELEASE_DISPLAY_MUTEX();
}



//-------------------------------------
// Экран "Ввод числа"
void Display_InputNumber(Display_InputNumber_Parameters params)
{
  const char *Title       = params.Title;
  const char *Bottom      = params.Bottom;
  uint8_t     ClearScreen = params.ClearScreen;
  int32_t     Min         = params.Min;
  int32_t     Max         = params.Max;
  int32_t     Value       = params.Value;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //Поскольку самая длинная строка, поддерживаемая контроллером HD44780 - равна 40 символам.
  char        s[17];
  
  const char *Subtitle    = "Мин.  ВВОД  Макс";           //Шаблон "Подзаголовка" (или "заголовка по-умолчанию").
  int8_t      row         = lcd.lines_amount/2;           //Номер строки с Числом. (дисплей->строка: 1->0, 2->1, 3->1, 4->2)
  
  //Если над Числом осталось меньше двух строк, и задан Заголовок - то выводим его вместо Подзаголовка... (замещение)
  if(row <= 1 && Title)
    Subtitle = Title;
  
  GET_DISPLAY_MUTEX();
  
  //Заголовок
  if(row-2 >= 0)
  {
    if(Title)
    {
      Display_FormatString(buf, Title, 1);
      hd44780_move_cursor(&lcd, 0, row-2);
      hd44780_write_string(&lcd, Display_EncodeString(buf));
    }
    else if(ClearScreen)
      Display_CleanRow(row-2);
  }
  
  //Подзаголовок
  if(row-1 >= 0)
  {
    Display_FormatString(buf, Subtitle, 1);
    hd44780_move_cursor(&lcd, 0, row-1);
    hd44780_write_string(&lcd, Display_EncodeString(buf));
  }
    
  //Число
  sprintf(s, "%-4d< %4d <%4d", Min, Value, Max);
  Display_FormatString(buf, s, 1);
  hd44780_move_cursor(&lcd, 0, row);
  hd44780_write_string(&lcd, buf);

  //Подвал
  if(row+1 <= lcd.lines_amount-1)
  {
    if(Bottom)
    {
      Display_FormatString(buf, Bottom, 1);
      hd44780_move_cursor(&lcd, 0, row+1);
      hd44780_write_string(&lcd, Display_EncodeString(buf));
    }
    else if(ClearScreen)
      Display_CleanRow(row+1);
  }
  
  //Очистить остальные строки дисплея (на тот невероятный случай, если на дисплее больше 4 строк)
  if(ClearScreen)
    for(uint8_t i=0; i<lcd.lines_amount; i++)
      if(i < row-2 || i > row+1)
        Display_CleanRow(i);

  RELEASE_DISPLAY_MUTEX();
}



//-------------------------------------
// Экран "Нижнее Меню"
//  Заполняется только одна, самая нижняя, строка дисплея (остальные игнорируются, данные в них сохраняются).
//  Строка составляется из текстовых фрагментов: ItemL (выравнивается к левому краю) + ItemC (центрируется) + ItemR (выравнивается к правому краю)
//  Если какой-то из параметров == NULL, то соответствующий пункт пропускается... Рекомендуется использовать: 
//      ItemC=NULL, для двух вариантов ответа; 
//      или ItemL=ItemC=NULL, для одного варианта ответа (типа ItemR="Далее->"); 
//      или ItemC=ItemR=NULL, для одного варианта ответа (типа ItemL="<-Назад").
void Display_BottomMenu(Display_BottomMenu_Parameters params)
{
  const char *ItemL = params.ItemL;
  const char *ItemC = params.ItemC;
  const char *ItemR = params.ItemR;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //Поскольку самая длинная строка, поддерживаемая контроллером HD44780 - равна 40 символам.
  size_t      count;
  uint8_t     left  = 0;
  uint8_t     right = lcd.columns_amount;

  GET_DISPLAY_MUTEX();

  //Сперва, забиваем шаблон пробелами
  memset(buf, ' ', lcd.columns_amount);
  buf[lcd.columns_amount] = 0;                            // Примечание: это очень важный момент - конец буфера null-terminated.

  //Левый
  if(ItemL)
  {
    count = strlen(ItemL);
    memcpy(buf, ItemL, count);
    left += count;
  }
    
  //Правый
  if(ItemR)
  {
    count = strlen(ItemR);
    memcpy(buf+right-count, ItemR, count);
    right -= count;
  }

  //Средний
  if(ItemC)
  {
    count = strlen(ItemC);
    memcpy(buf+left + (right-left - count)/2, ItemC, count);
  }

  //Строка была сперва сформирована в памяти (для исключения мерцания) - теперь выводим ее на Дисплей
  hd44780_move_cursor(&lcd, 0, lcd.lines_amount-1);
  hd44780_write_string(&lcd, Display_EncodeString(buf));

  RELEASE_DISPLAY_MUTEX();
}



