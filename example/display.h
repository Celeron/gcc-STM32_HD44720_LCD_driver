#ifndef DISPLAY_H
#define DISPLAY_H


//-------------------------------------
// Служебные методы
//-------------------------------------

// Инициализация модуля (функция должна быть вызвана единожды, при старте программы)
void Display_Init(void);


// DEBUG: Тестирование LCD-дисплея (блокирующая глюкалка)
void Display_TestLCD(void);


// Запретить всем Потокам использование разделяемого ресурса (Дисплей)
//  Примечание: используется для системных целей, для синхронизации потоков...
osStatus Display_Lock(void);
osStatus Display_UnLock(void);




//-------------------------------------
// Универсальные Экраны
//-------------------------------------


// Экран "Сообщение" (универсальный)
typedef struct {
  const char *Line1;
  const char *Line2;
  const char *Line3;
  const char *Line4;
  uint8_t     AlignCentered;
} Display_Message_Parameters;

void Display_Message(Display_Message_Parameters params);

#define Display_Show_Message(...)  Display_Message((Display_Message_Parameters) {.Line1 = NULL, .Line2 = NULL, .Line3 = NULL, .Line4 = NULL, .AlignCentered = 0, __VA_ARGS__})



// Экран "полоса прогресса" 
typedef struct {
  const char *Title;
  uint8_t     Progress; 
  uint8_t     ClearScreen;
} Display_ProgressBar_Parameters;

void Display_ProgressBar(Display_ProgressBar_Parameters params);

#define Display_Show_ProgressBar(...)  Display_ProgressBar((Display_ProgressBar_Parameters) {.Title = NULL, .Progress = 100, .ClearScreen = 1, __VA_ARGS__})



// Экран "Ввод числа"
typedef struct {
  const char *Title;
  const char *Bottom;
  uint8_t     ClearScreen;
  int32_t     Min;
  int32_t     Max;
  int32_t     Value;
} Display_InputNumber_Parameters;

void Display_InputNumber(Display_InputNumber_Parameters params);

#define Display_Show_InputNumber(...)  Display_InputNumber((Display_InputNumber_Parameters) {.Title = NULL, .Bottom = NULL, .ClearScreen = 1, .Min = 1000, .Max = 9999, .Value = 9999, __VA_ARGS__})



// Экран "Нижнее Меню"
typedef struct {
  const char *ItemL;
  const char *ItemC;
  const char *ItemR;
} Display_BottomMenu_Parameters;

void Display_BottomMenu(Display_BottomMenu_Parameters params);

#define Display_Show_BottomMenu(...)  Display_BottomMenu((Display_BottomMenu_Parameters) {.ItemL = NULL, .ItemC = NULL, .ItemR = NULL, __VA_ARGS__})




#endif  // DISPLAY_H
