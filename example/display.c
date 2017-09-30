/******************************************************************************
 * ������:      display.c
 * �����:       Celeron (c) 2017
 * ����������:  ������ ������ ������ ��������� ��� ��������� ����������� �� �������. 
 *              ������������� ������������� ��������� ��� ����������� LCD-������: ������������ ��������� ��������, ����������� ������ �� ������ � ����������� �� �������� ������� ������ � �.�.
 *              ������������ �������� ������ (Display_Run) [����] �� ��������, ��������� �������������� � �������� �� ���� ������� ������� �� �����.
 ******************************************************************************/


#include "stm32f1xx_hal.h"          // ���������� HAL API
#include "cmsis_os.h"               // ���������� RTOS API

#include "hd44780.h"                // ���������� LCD
#include "hd44780_stm32f1xx_hal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "display.h"                // ��������� ��������� ������� � �����




//============================================================================
// ����������� � ������������� ������� (���������� �����)
//============================================================================


//���������-���������� ���������� ������ ����������
HD44780 lcd;
HD44780_STM32F10x_GPIO_Driver lcd_pindriver;



//-------------------------------------
//�������� ��� ������������ �����������
void assert_failed(uint8_t* file, uint32_t line);   // Note: function defined in "main.c"...
void hd44780_assert_failure_handler(const char *filename, unsigned long line)
{
#ifdef  USE_FULL_ASSERT
  assert_failed((uint8_t*) filename, line);
#endif
}



//-------------------------------------
//�������� ��� ������������ �����������
#include "delays.h"   // Original library taken from Easyrider83 (c) http://kazus.ru/forums/showpost.php?p=524970&postcount=14
void hd44780_delay_microseconds(uint16_t us)
{
  delay_us((uint32_t) us);
  
//  // ������ (������� ����� �����������), �� ������������� ��������...
//  // ��������: �����������! ����������!
//  uint32_t ms = us/1000;
//  if (ms==0) 
//      ms = 1;
//  osDelay(ms);
}



//-------------------------------------
// ������������� ���������� ����� (LCD-�������� � ����������� LCD-������)
void Display_InitLCD(void)
{
  /* ���������� ������� */
  const HD44780_STM32F10x_Pinout lcd_pinout =
  {
    {
      /* RS        */  { GPIOD, GPIO_PIN_11 },
      /* ENABLE    */  { GPIOD, GPIO_PIN_7 },
      /* RW        */  { GPIOD, GPIO_PIN_5 },
      /* Backlight */  { GPIOE, GPIO_PIN_2 },    // ������� ���������: { NULL, 0 },
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

  /* ����������� �������: ��������� ��������� �������� (�����������),
     ��������� ���� ���������� � ���������� ������ GPIO (������������). */
  lcd_pindriver.interface = HD44780_STM32F10X_PINDRIVER_INTERFACE;
  //lcd_pindriver.interface.configure = NULL;   // ����� ���������� �����: ���� ����� �������� ���� ������� ����������� GPIO ��� ������� (����� �� �����), �� �������� ����� =NULL (���������� ���� ���)... 
                                                // ��������: �� ������������! ������! ���� ��������� ����� NULL, �� ���������� ������� ��������� � ������ �� �������� (��������, ���������� ���������� ��� ����� ������?)
  lcd_pindriver.pinout = lcd_pinout;
  lcd_pindriver.assert_failure_handler = hd44780_assert_failure_handler;

  /* �, �������, ������ ������������ �������: ��������� ��� �������,
     ������� ��������, ���������� ������ ������� (������������) � �����.
     �� ������ ������ �������� ��� ����� - ������������ ��� ���
     ����� RW ������� (� ��������� ������ ��� ����� ������� � GND),
     � �� �� ��� ���������� ����������. */
  const HD44780_Config lcd_config =
  {
    (HD44780_GPIO_Interface*)&lcd_pindriver,
    hd44780_delay_microseconds,
    hd44780_assert_failure_handler,
    HD44780_OPT_USE_RW
  };

  /* ��, � ������ �� ����������: ����� ������������ �� GPIO,
     �������������� �������: 16x2, 8-������ ���������, ������� 5x8 �����. */
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  //hd44780_init(&lcd, HD44780_MODE_8BIT, &lcd_config, 16, 2, HD44780_CHARSIZE_5x8);  // ������� ���: Winstar WH1602L-YGH-CT (Cutter) http://www.kosmodrom.com.ua/el.php?name=WH1602L-YGH-CT
  hd44780_init(&lcd, HD44780_MODE_8BIT, &lcd_config, 20, 4, HD44780_CHARSIZE_5x8);    // ������� ���: Winstar WH2004L-TFH-CT (Merger) http://www.kosmodrom.com.ua/el.php?name=WH2004L-TFH-CT
  // ��� �������: 4-������ ���������...
  //hd44780_init(&lcd, HD44780_MODE_4BIT, &lcd_config, 16, 2, HD44780_CHARSIZE_5x8);
  
  
  // �������, ��������� ���� ������� � ������ �����������:
  //  ������� hd44780_create_char() ������ ������ � �������� ASCII-����� �� 0 �� 7, ��������� ������� �������� ������� � ���� ������� �� 8 ������, ������ �� ������� �������� ������ �� 5 �����.
  //  ����������: ������ � ����� #0 ���������� ����� ������ � �� � �������������� �� �����! ����� �������, ����������� ������� �������������� �� ����, ������� � #1...
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
  hd44780_create_char(&lcd, 1, charmap1);  // �������������� ������ � ����� #1 (������� ������ ���������)

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
  hd44780_create_char(&lcd, 2, charmap2);  // �������������� ������ � ����� #2 (������������ ������)

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
  hd44780_create_char(&lcd, 3, charmap3);  // �������������� ������ � ����� #3 (������ ������� >)

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
  hd44780_create_char(&lcd, 4, charmap4);  // �������������� ������ � ����� #4 (������ ������� <)
}




//============================================================================
// ���������� ������� � ������� (API ������)
//============================================================================

// ����������� multithreading-safe ������� � �������
// ������ ���������� ����������� ������ API-�������: ���� �� ���������� ���� - ������ ������ ������� �������������  (�������� ������������� ������ �������, ��� ��������� � ������������ �������)
osMutexId DisplayMutex;



//-------------------------------------
// �� ���� API-������� ������� ��������� �������, ��������������� � ����������� ������������� ����� �� ������� �� ������ ������� 
// (�����, ��� �������� ��������� ��������, ��������� ����)...

#define GET_DISPLAY_MUTEX()                           \
  if(Display_Lock() != osOK) return;  /* ���� �� �������� �������, �� ������ �� ������... */

#define RELEASE_DISPLAY_MUTEX()                       \
  Display_UnLock();



//-------------------------------------
// ��������� ����������� ������ (�������)
osStatus Display_Lock(void)
{
  osStatus status = osMutexWait(DisplayMutex, 50);  // GET_SOUND_MUTEX
  assert_param(status == osOK);
  return status;
}



//-------------------------------------
// ���������� ����������� ������ (�������)
osStatus Display_UnLock(void)
{
  osStatus status = osMutexRelease(DisplayMutex);   // RELEASE_SOUND_MUTEX
  assert_param(status == osOK);
  return status;
}




//============================================================================
// ������������� ������ (��������� ������)
//============================================================================


/*
// ����� (���������������) ���� ��������� ������� Display_Run, ��������� �������������� � �������� �� ���� ������� ������� �� �����! ��������...
//-------------------------------------
// ����������� �������
osThreadId Display_Run_ThreadId;

//-------------------------------------
// ������� ���� ��������� ������� ������ �� �������
// (����������: ��������� ��������, ���� ��������� � �������� ��� ���������� ����������, �������������� ����������� �� �������...)
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
// ������������� ������
void Display_Init(void)
{
  // ������������� ���������� ����� (LCD-�������� � ����������� LCD-������)
  Display_InitLCD();

  // �������� ���������
  hd44780_backlight_on(&lcd);
  
  // ������ ���������� ����������� ������ API-�������
  DisplayMutex = osMutexCreate(NULL);
  assert_param(DisplayMutex != NULL);
  
//  // ������ ������� �������
//  osThreadDef(Display_Run, Display_Run, osPriorityNormal, 0, 128);
//  Display_Run_ThreadId = osThreadCreate(osThread(Display_Run), NULL); // ������� ���� ��������� ������� ������ �� �������
//  assert_param(Display_Run_ThreadId != NULL);  
}




//============================================================================
// ����������� ������
//============================================================================


// �����: �� ���� Display_EncodeString ����� ��������� ������ �� ������ �� ������ � ��� (���������� ������), �� � ����� �������� �� FLASH (������������ ������)!
// ������� ������ �� ����� ���� ���������������� inplace - ���������� ��������� ��������� ����� (���������� ���� ������) � ���������� ���...

// ����� ��� ��������� ������ - �������� ����������, � ���������� �������� ������������. 
//    ������ ������: ����� �������������� ����� �� ����� ��������� 40 �������� (����� ������ �������) + 1 ������ '\0'.
//    ����������: � ��, ��� ���������� ���������� ������, static ����� �� ����������� ������� private, ���������� �� ������ �����... (c) http://digitalchip.ru/osobennosti-ispolzovaniya-extern-i-static-v-c-c
static char EncodedString[HD44780_COLUMNS_AMOUNT_MAX + 1];


// ��������: ��� ������ ��������� ������ ������ Display_EncodeString, ���������� ��������� ������ EncodedString ����� ����������������! 
// �������, ������� ����������� ������ �����: ��������������� ������ - � ��� �� �������� �� �����... ��������������� - �����... 
// ��������: hd44780_write_string(&lcd, Display_EncodeString(s1));


// ������: ������������� ������ � ������ ����� ������ ����� ESC-������������������: \��� (������������ ��� �������, ��� �����)... ��������: char *s1 = "������ ������ 0xED = \351";
// �� ����: ���� ����� ������ �������������� ������� ����� ������ Display_EncodeString(s1), �� ��� ������������� ������� � ��� �����������! 
// (�����: �� ��������� ������� ������������������� � �������� ������! � ������ �����, �������� ������� ������������� � ������ Display_EncodeString.)



//-------------------------------------
// ����� Display_EncodeString ������������ ��������� ������ � ���������, �������������� LCD-������������ HD44780 (��� ������ ������� ��������)
//  ����������: ��������� �� ����� � ��������������� ������� (null-terminated).
char *Display_EncodeString(char *s)
{
  char *output = EncodedString;         // �������������
  
  do                                    // ����: ���� �� �������� ����� ������...
  {
    switch (*s)                         // ����������: *s �������� �������� ������ �� ������� ������ (�������� ������������� ���������), �.�. ��� �������.
    {
      // ����������:
      //  ��������� �������� ��� Winstar WH1602L-YGH-CT (Cutter) http://www.kosmodrom.com.ua/el.php?name=WH1602L-YGH-CT
      //  ��������� �������� ��� Winstar WH2004L-TFH-CT (Merger) http://www.kosmodrom.com.ua/el.php?name=WH2004L-TFH-CT
      //  ������ "������� �������� ��� ����������� HD44780 (�����������������)" ��. �� http://cxem.net/mc/book52.php ��� � datasheet...
      case '�':  *output = 97;   break; 
      case '�':  *output = 178;  break;
      case '�':  *output = 179;  break;
      case '�':  *output = 180;  break;
      case '�':  *output = 227;  break;
      case '�':  *output = 101;  break;
      case '�':  *output = 181;  break;
      case '�':  *output = 182;  break;
      case '�':  *output = 183;  break;
      case '�':  *output = 184;  break;
      case '�':  *output = 185;  break;
      case '�':  *output = 186;  break;
      case '�':  *output = 187;  break;
      case '�':  *output = 188;  break;
      case '�':  *output = 189;  break;
      case '�':  *output = 111;  break;
      case '�':  *output = 190;  break;
      case '�':  *output = 112;  break;
      case '�':  *output = 99;   break;
      case '�':  *output = 191;  break;
      case '�':  *output = 121;  break;
      case '�':  *output = 228;  break;
      case '�':  *output = 120;  break;
      case '�':  *output = 229;  break;
      case '�':  *output = 192;  break;
      case '�':  *output = 193;  break;
      case '�':  *output = 230;  break;
      case '�':  *output = 194;  break;
      case '�':  *output = 195;  break;
      case '�':  *output = 196;  break;
      case '�':  *output = 197;  break;
      case '�':  *output = 198;  break;
      case '�':  *output = 199;  break;
      case '�':  *output = 65;   break;
      case '�':  *output = 160;  break;
      case '�':  *output = 66;   break;
      case '�':  *output = 161;  break;
      case '�':  *output = 224;  break;
      case '�':  *output = 69;   break;
      case '�':  *output = 162;  break;
      case '�':  *output = 163;  break;
      case '�':  *output = 164;  break;
      case '�':  *output = 165;  break;
      case '�':  *output = 166;  break;
      case '�':  *output = 75;   break;
      case '�':  *output = 167;  break;
      case '�':  *output = 77;   break;
      case '�':  *output = 72;   break;
      case '�':  *output = 79;   break;
      case '�':  *output = 168;  break;
      case '�':  *output = 80;   break;
      case '�':  *output = 67;   break;
      case '�':  *output = 84;   break;
      case '�':  *output = 169;  break;
      case '�':  *output = 170;  break;
      case '�':  *output = 88;   break;
      case '�':  *output = 225;  break;
      case '�':  *output = 171;  break;
      case '�':  *output = 172;  break;
      case '�':  *output = 226;  break;
      case '�':  *output = 173;  break;
      case '�':  *output = 174;  break;
      case '�':  *output = 98;   break;
      case '�':  *output = 175;  break;
      case '�':  *output = 176;  break;
      case '�':  *output = 177;  break;
      case '~':  *output = 233;  break; // (������ "������")
      //case 13:                        // ��������� �������� �������� ������ �� ������������
      //case 10:
      default:   *output = *s;   break; // ��� ��������� ������� - �������� � �������� ������ ��� ���������
    }
    s++;                                // ������� � ���������� �������
    output++;
  } while( *(s-1)                       // ����: ���� ��� [�����������, ������ ��� �������������] ������� �� ����� 0...
         && (output - EncodedString) < HD44780_COLUMNS_AMOUNT_MAX );  // ��� ���� �� ������������ �����

  EncodedString[HD44780_COLUMNS_AMOUNT_MAX] = 0;                      // �� ������ ������������ ������: ������ ����������� ��� "null-terminated"...
  
  return EncodedString;                 // ���������� ������ �� ������ ������ (��������: ����� ������ ���������� ��������� ��������� output)
}



//-------------------------------------
// ������� itoa ����������� ����� ����� value (� ������� ��������� � ���������� base) � ������ result.
//  ����������: ������� itoa � ������ ��������������� ������������� ���������� ������������ ����� ���������������� ��. �� ������������� �� ��������������� �������������, ��������� ��� ������� �� ���������� �� � ����� ��������� ����� ��; ��� �� �����, �������� ����������� ������������ �� �� ���� ������������� ��������� <stdlib.h>, ������ �� ������ � ������� ����, ��� ��� ��� ������ ������ �� ������ � ����������� ������������ ������� atoi.
//  ������������: ��� �������������� ����� � ������ � ���������� 8 (������������), 10 (����������) ��� 16 (����������������� ������� ���������) �������������, ����������� �� ����������, �������� ������������� ����������� ������������ ������� sprintf.
/**
 * C++ version 0.4 char* style "itoa":
 * Written by Luk�s Chmela
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
// ����� ����������� ������ ��� ������ �� �������
//  �����: �������� buf (��������) ������ ��������� �� ���������� ����� � ������ ������������ �������, ������ (������ ������� + 1 ������ ��� "null-terminated"), �.�. (lcd.columns_amount + 1).
char *Display_FormatString(char *buf, const char *s, uint8_t align)
{
  // ������, �������� ������ ���������
  memset(buf, ' ', lcd.columns_amount);
  buf[lcd.columns_amount] = 0;                      // ����������: ��� ����� ������ ������ - ����� ������ null-terminated.

  // ���������� ���������� �������� ������
  size_t len = strlen(s);
  if(len > lcd.columns_amount)
    // ������, ��� �������� �� ������� ������� �� �����������, ����������
    len = lcd.columns_amount;
  
  // ������ �������, � ������� ����������� ������
  uint8_t pos = 0;
  if(align)
    pos = (lcd.columns_amount - len)>>1;
  
  // �����, ������, ������������ ��������� �������
  memcpy( buf+pos, s, len);
  
  // ���������� ��������� �� buf, ��� ��������� ������������ ������ ����� ��� "������" � "��������"...
  return buf;
}




//============================================================================
// ������������� ������
//============================================================================


//-------------------------------------
//������� ���� ������ ������� (�������� ���������)
void Display_CleanRow(uint8_t row)
{
  hd44780_move_cursor(&lcd, 0, row);
  for(uint8_t i=0; i < lcd.columns_amount; i++)
    hd44780_write_char(&lcd, ' ');
  
  //hd44780_move_cursor(&lcd, 0, row);  //���������, ���������� ����������� (�� ����� ���������� ������ � ������ ������)
}



//-------------------------------------
// DEBUG: ������������ LCD-������� (����������� ��������)
// ����������: � ������ ������ ������� "����������������", �� ������ ������ ����������� ����������� ��������...
// ����������: ���������� ������ ������� ���������� � �� thread-safe!
void Display_TestLCD(void)
{
  //char Msg0[]={67,97,188,111,191,101,99,191,184,112,111,179,97,189,184,101,0};  // ����� �������� "����������������", � �������� ��������� LCD-����������� HD44780
  //char Msg1[]="0123456789ABCDEFGHIJ";

  hd44780_clear(&lcd);
  hd44780_move_cursor(&lcd, 0, lcd.lines_amount - 1);     // ��������� ������� �������� �����������: (column=0..19, row=0..3)
  hd44780_write_string(&lcd, Display_EncodeString("�������� �������"));   // ��� �������: "�������� �������"

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
// ����� "���������" (�������������)
//  ��������� �� ������� ������ �������: 1-2-4 ������, �� 16-20-24 �������.
//  ������ ������ ������������� ��������������� ��������: s1, s2, s3, s4.
//  ���� ����� �������� sx == NULL, �� ������ ������ �� ������� �� ��������� (���� �� ���������, � ������ ������������)!
//  ���� ���������� ������� null-terminated ������, �� ��� ������������� �� ��� ������ ������ ������� � ���������.
//  ���� �������� AlignCentered == 1, �� ������ ������������ �� �����������. �����, ���� AlignCentered == 0 (�� ���������), �� ������ LeftAligned.
void Display_Message(Display_Message_Parameters params)
{
  const char *lines[] = {params.Line1, params.Line2, params.Line3, params.Line4};
  uint8_t     align   = params.AlignCentered;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //��������� ����� ������� ������, �������������� ������������ HD44780 - ����� 40 ��������.

  GET_DISPLAY_MUTEX();
  
  // ���� �� �������
  for(uint8_t y=0; y < lcd.lines_amount; y++)
  {
    // ����������: ������� ������ �������������� ������ ���� ��������� �� ������ ���������
    const char *s = lines[y];
    if(s)
    {
      // ����� ����������� ������ ��� ������ �� �������
      Display_FormatString(buf, s, align);

      // ��������� ������� �� �������
      hd44780_move_cursor(&lcd, 0, y);
      hd44780_write_string(&lcd, Display_EncodeString(buf));     
    }
  }
  
  RELEASE_DISPLAY_MUTEX();
}



//-------------------------------------
// ����� "������ ���������" 
//  � ������ ������ ������� ����������/��������� "������ ���������", ����������� �� Progress ��������� = [0..100]
//  ����� ����, � ������� ������ ���������� ����� ������������� ��������� Title ��� ������������...
void Display_ProgressBar(Display_ProgressBar_Parameters params)
{
  const char *Title       = params.Title;
  uint8_t     progress    = params.Progress;
  uint8_t     ClearScreen = params.ClearScreen;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //��������� ����� ������� ������, �������������� ������������ HD44780 - ����� 40 ��������.
  char        s[5];
  size_t      count;
  int8_t      row         = lcd.lines_amount/2;           //����� ������ � �������. (�������->������: 1->0, 2->1, 3->1, 4->2)

  GET_DISPLAY_MUTEX();

  //���������
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
  
  //������
  if(progress>100) progress=100;
  count = progress*lcd.columns_amount/100;
  memset(buf,       '\1', count);                         //����������: �����, ������ � ����� #1 �������� ��������� charmap-�� (������� ������ ���������, ��������� �� ����� ������������� �������).
  memset(buf+count, ' ',  lcd.columns_amount-count);
  buf[lcd.columns_amount] = 0;

  //�����
  itoa(progress, s, 10);
  count = strlen(s);
  assert_param(count<=3);
  s[count++]='%';                                         //���������� ������ ��������, ����� �����...
  memcpy(buf+lcd.columns_amount/2-count/2, s, count);

  //������ ���� ������ ������������ � ������ (��� ���������� ��������) - ������ ������� �� �� �������
  hd44780_move_cursor(&lcd, 0, row);
  hd44780_write_string(&lcd, buf);

  //�������� ��������� ������ ������� (���� �� ������� ������ 2 �����)
  if(ClearScreen)
    for(uint8_t i=0; i<lcd.lines_amount; i++)
      if(i < row-1 || i > row)
        Display_CleanRow(i);

  RELEASE_DISPLAY_MUTEX();
}



//-------------------------------------
// ����� "���� �����"
void Display_InputNumber(Display_InputNumber_Parameters params)
{
  const char *Title       = params.Title;
  const char *Bottom      = params.Bottom;
  uint8_t     ClearScreen = params.ClearScreen;
  int32_t     Min         = params.Min;
  int32_t     Max         = params.Max;
  int32_t     Value       = params.Value;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //��������� ����� ������� ������, �������������� ������������ HD44780 - ����� 40 ��������.
  char        s[17];
  
  const char *Subtitle    = "���.  ����  ����";           //������ "������������" (��� "��������� ��-���������").
  int8_t      row         = lcd.lines_amount/2;           //����� ������ � ������. (�������->������: 1->0, 2->1, 3->1, 4->2)
  
  //���� ��� ������ �������� ������ ���� �����, � ����� ��������� - �� ������� ��� ������ ������������... (���������)
  if(row <= 1 && Title)
    Subtitle = Title;
  
  GET_DISPLAY_MUTEX();
  
  //���������
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
  
  //������������
  if(row-1 >= 0)
  {
    Display_FormatString(buf, Subtitle, 1);
    hd44780_move_cursor(&lcd, 0, row-1);
    hd44780_write_string(&lcd, Display_EncodeString(buf));
  }
    
  //�����
  sprintf(s, "%-4d< %4d <%4d", Min, Value, Max);
  Display_FormatString(buf, s, 1);
  hd44780_move_cursor(&lcd, 0, row);
  hd44780_write_string(&lcd, buf);

  //������
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
  
  //�������� ��������� ������ ������� (�� ��� ����������� ������, ���� �� ������� ������ 4 �����)
  if(ClearScreen)
    for(uint8_t i=0; i<lcd.lines_amount; i++)
      if(i < row-2 || i > row+1)
        Display_CleanRow(i);

  RELEASE_DISPLAY_MUTEX();
}



//-------------------------------------
// ����� "������ ����"
//  ����������� ������ ����, ����� ������, ������ ������� (��������� ������������, ������ � ��� �����������).
//  ������ ������������ �� ��������� ����������: ItemL (������������� � ������ ����) + ItemC (������������) + ItemR (������������� � ������� ����)
//  ���� �����-�� �� ���������� == NULL, �� ��������������� ����� ������������... ������������� ������������: 
//      ItemC=NULL, ��� ���� ��������� ������; 
//      ��� ItemL=ItemC=NULL, ��� ������ �������� ������ (���� ItemR="�����->"); 
//      ��� ItemC=ItemR=NULL, ��� ������ �������� ������ (���� ItemL="<-�����").
void Display_BottomMenu(Display_BottomMenu_Parameters params)
{
  const char *ItemL = params.ItemL;
  const char *ItemC = params.ItemC;
  const char *ItemR = params.ItemR;
  char        buf[HD44780_COLUMNS_AMOUNT_MAX + 1];        //��������� ����� ������� ������, �������������� ������������ HD44780 - ����� 40 ��������.
  size_t      count;
  uint8_t     left  = 0;
  uint8_t     right = lcd.columns_amount;

  GET_DISPLAY_MUTEX();

  //������, �������� ������ ���������
  memset(buf, ' ', lcd.columns_amount);
  buf[lcd.columns_amount] = 0;                            // ����������: ��� ����� ������ ������ - ����� ������ null-terminated.

  //�����
  if(ItemL)
  {
    count = strlen(ItemL);
    memcpy(buf, ItemL, count);
    left += count;
  }
    
  //������
  if(ItemR)
  {
    count = strlen(ItemR);
    memcpy(buf+right-count, ItemR, count);
    right -= count;
  }

  //�������
  if(ItemC)
  {
    count = strlen(ItemC);
    memcpy(buf+left + (right-left - count)/2, ItemC, count);
  }

  //������ ���� ������ ������������ � ������ (��� ���������� ��������) - ������ ������� �� �� �������
  hd44780_move_cursor(&lcd, 0, lcd.lines_amount-1);
  hd44780_write_string(&lcd, Display_EncodeString(buf));

  RELEASE_DISPLAY_MUTEX();
}



