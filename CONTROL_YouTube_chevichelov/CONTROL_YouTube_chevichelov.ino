#include <avr/io.h>
#include <util/delay.h>


#define PIN_A2   A2                                                                             //Назначаем пин A2
#define PIN_P3   PB3                                                                            //Назначаем пин P3

float R1_R3                           =  109.97;                                                //Сопротивление резистора R1 + R3 в кОм 100.00 + 9.97 = 109.97
float R2                              =  10.14;                                                 //Сопротивление резистора R2 в кОм 10,14

#define OLED_ADDR       0x78                                                                    //Адрес дисплея
#define OLED_CMD_MODE   0x00                  
#define OLED_DAT_MODE   0x40                  
#define DISPLAY_INIT_LEN   15  
 
#define I2C_SDA         PB2                   
#define I2C_SCL         PB1                  
 
#define I2C_SDA_HIGH()  DDRB &= ~(1<<I2C_SDA) 
#define I2C_SDA_LOW()   DDRB |=  (1<<I2C_SDA) 
#define I2C_SCL_HIGH()  DDRB &= ~(1<<I2C_SCL) 
#define I2C_SCL_LOW()   DDRB |=  (1<<I2C_SCL) 
 
uint16_t VOLT       = 0;
uint8_t DATA[5]      = {};

float DEFAULT_VOLT                    = 0;                                                    //Напряжение по умолчанию;
float  VOLT_DISPLAY                   = 0;                                                    //Объявляем переменную для хранения значения напряжения
unsigned long TIME                    = 0;                                                    //Объявляем переменную таймера задержки измерений

#include "DISPLAY.h"
 
void setup() 
{
    ADMUX |= (1 << REFS2) | (1 << REFS1) | (1 << REFS0);                                        //Устанавливаем биты REFS2-0 = 111, после чего опорное напряжение становится 2.56 вольта. Это напряжение будет выведено на PB0 (его нельзя никак трогать ни в программе, ни в схеме) по мимо этой строчки требуется на пин PB0 припаять конденсатор 100нФ.
    DISPLAY_INIT();                                                                             //Инициализируем дисплей
    pinMode(PIN_P3, OUTPUT);                                                                    //Пин установлен на выход
    pinMode(PIN_A2, INPUT);                                                                     //Пин установлен на вход
    DEFAULT_VOLT                        = GET_DEFAULT_VOLT();                                   //Измеряем опорное напряжение ATtiny85
}

void loop()
{
  if (millis() - TIME <= 200)                                                                   //Добавляем задержку в 200 миллисекунд
    return;
  TIME = millis(); 


                                                                                                
  VOLT_DISPLAY = analogRead(PIN_A2) * DEFAULT_VOLT / 1024 * (( R1_R3 + R2 ) / R2);              //Рассчитываем значение напряжения

  
  if (VOLT_DISPLAY >= 0.5)                                                                      //Если напряжение больше 0.5 вольт, значит плюс
  {
    digitalWrite(PIN_P3, LOW);                                                                  //Отключаем напряжение на пине P3
  } 
  else                                                                                          //Иначе минус
  {
    digitalWrite(PIN_P3, HIGH);                                                                 //Подаём напряжение на пин P3
  }
  if (VOLT_DISPLAY >= 10)                                                                       //Напряжение больше 10 вольт
  {
    VOLT  = VOLT_DISPLAY * 100;                                                                 //Сдвигаем значение напряжение на 2 бита влево
  }
  else                                                                                          //Напряжение меньше 10 вольт
  {                                                                                  
    VOLT  = VOLT_DISPLAY * 1000;                                                                //Сдвигаем значение напряжение на 3 бита влево
  }
  
  DISPLAY_CLEAR();
  
  if (VOLT_DISPLAY >= 0.5 || VOLT_DISPLAY == 0)                                                //Если напряжение больше 0.5 вольт или равно нулю выводим данные на дисплей
  {
    DATA[0]       = VOLT / 1000;   
    DATA[1]       = VOLT_DISPLAY >= 10 ? (VOLT % 1000) / 100 : 13;
    DATA[2]       = VOLT_DISPLAY >= 10 ? 13 : (VOLT % 1000) / 100;
    DATA[3]       = (VOLT % 100) / 10;
    DATA[4]       = VOLT % 10;                                           
    DISPLAY_PRINT(DATA); 
  }
  else                                                                                          //Иначе отображаем на дисплее слово MASS
  {
    DATA[0]       = 11;   
    DATA[1]       = 10;
    DATA[2]       = 12;
    DATA[3]       = 12;
    DATA[4]       = 10;
    DISPLAY_PRINT(DATA);                                                                       
  }   
}


float GET_DEFAULT_VOLT() {                                                                      //Функция измеряет внутреннее напряжение Arduino
  long RESULT         = 0;                                                                      //Определяем переменную для получения результата.
  byte  COUNT_RESULT  = 100;                                                                    //Определяем сколько значений АЦП требуется получить для усреднения результата.
                                                                                                //Для Arduino Mega, Leonardo и Micro, сбрасываем бит «MUX5» регистра «ADCSRB», так как «MUX[5-0]» должно быть равно 011110 (см. регистр «ADMUX»).
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //Устанавливаем биты регистра «ADMUX»: «REFS»=01 (ИОН=VCC), «ADLAR»=0 (выравнивание результата по правому краю), «MUX[4-0]»=11110 или «MUX[3-0]»=1110 (источником сигнала для АЦП является напряжение ИОН на 1,1 В).   
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  for(byte i=0; i<COUNT_RESULT; i++)                                                            //Получаем несколько значений АЦП
  {
    ADCSRA |= _BV(ADSC);                                                                        //Запускаем преобразования АЦП:Устанавливаем биты регистра «ADCSRA»: «ADEN»=1  (вкл АЦП), «ADSC» =1 (запускаем новое преобразование). 
    while (bit_is_set(ADCSRA, ADSC));                                                           //Получаем данные АЦП:
    uint8_t _LOW  = ADCL;
    uint8_t _HIGH = ADCH;
  
    RESULT += (_HIGH << 8) | _LOW;                                                              //Суммируем результат
  }
  RESULT /= COUNT_RESULT;                                                                       //Делим результат «RESULT» на «COUNT_RESULT», так как мы получили его «COUNT_RESULT» раз.    
  return (1.1f/RESULT) * 1024;                                                                  //Рассчитываем напряжение питания:  //  АЦП = (Uвх/Vcc)*1023. Напряжение Uвх мы брали с внутреннего ИОН на 1.1 В, значение которого возвращает функция analogSave_1V1(0).
}
