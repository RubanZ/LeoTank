/*-----------------------------------------------------------/
 LeoTank v1.3.9 от 26.02.2015
 Автор: Александр Рубан
 Список изменений:
 v0.1- Тестирование двигателей с Arduino NANO
 v0.2- Случайный выбор действий
 v0.3- Добавление кода для УЗ-датчика
 v0.3.1- Arduino Leonardo
 v0.4- Удаление кода для задних сенсоров
 v0.4.1- Компоновка кода и связывание с Front
 v0.4.2- Выбор действий при неиспользовании УЗ-датчика
 v0.4.3- Изменен алгоритм измерения растояния УЗ-датчика до 150см не врет
 v0.5- Рефакторинг
 v0.6- Все переписано заново
 v0.7- Не прерывное измерение датчика УЗ(снижение точности замера примерно на 20%)
 v0.8- Объезд припятствий работает превосходно
 v0.9- + Bluetooth управление по нажатию кнопки
 v0.9.1- Настроено Bluetooth управление(я так и не понял почему с телефона и компьютера передовались не те значения которые отправлял)
 v0.9.2- Исправлен обход припятствия.
 v1.0- Последние изменения и свет 14.04.2014
 v1.1- Новый алгоритм 06.07.2014
 v1.2- Повышение точности замера и временное отключение Bluetooth управления
 v1.2.1- Bluetooth управление
 v1.3 - Юху
 v1.4 - ИК датчик поверхности
 /-----------------------------------------------------------*/
#define REMOTEXY_MODE__SOFTWARESERIAL
#include <SoftwareSerial.h>
#include <Ultrasonic.h>
#include <RemoteXY.h>
/* настройки соединения */
#define REMOTEXY_SERIAL_RX 0
#define REMOTEXY_SERIAL_TX 1
#define REMOTEXY_SERIAL_SPEED 9600
/* конфигурация интерфейса  */
unsigned char RemoteXY_CONF[] =
{
  3, 1, 30, 0, 2, 8, 5, 15, 55, 14
  , 40, 40, 1, 2, 0, 5, 5, 22, 7, 4
  , 79, 78, 0, 79, 70, 70, 0, 66, 0, 5
  , 40, 7, 20, 1
};

/* структура определяет все переменные вашего интерфейса управления */
struct {

  /* input variable */
  signed char joystick_1_x; /* =-100..100 координата x положения джойстика */
  signed char joystick_1_y; /* =-100..100 координата y положения джойстика */
  unsigned char switch_1; /* =1 если переключатель включен и =0 если отключен */

  /* output variable */
  unsigned char level_1; /* =0..100 положение уровня */

  /* other variable */
  unsigned char connect_flag;  /* =1 if wire connected, else =0 */

}
RemoteXY;
//motorПравый//
#define dir1PinA   A0 //Серый
#define dir2PinA   A1 //Белый
#define speedA     9
//motorЛевый//
#define dir1PinB   A2 //Чёрный
#define dir2PinB   A3 //Коричневый
#define speedB     10
//Массивы//
unsigned char RightMotor[3] = {
  dir1PinA, dir2PinA, speedA
};
unsigned char LeftMotor[3] = {
  dir1PinB, dir2PinB, speedB
};
//Sensors//
Ultrasonic ultrasonic(6, 11);
//Random//
int napravlenie = 0 ;
//Кнопка//
#define buttonPin  4
boolean logicBlack = 0;
int buttonState = 1;
int val;
int val1 = 1;
//Свет//
#define PIN_SWITCH_1 8
#define LightA     7
#define ikPin      2

void Wheel (unsigned char * motor, int v)
{
  if (v > 100) v = 100;
  if (v < -100) v = -100;
  if (v > 0) {
    digitalWrite(motor[0], HIGH);
    digitalWrite(motor[1], LOW);
    analogWrite(motor[2], v * 2.55);
  }
  else if (v < 0) {
    digitalWrite(motor[0], LOW);
    digitalWrite(motor[1], HIGH);
    analogWrite(motor[2], (-v) * 2.55);
  }
  else {
    digitalWrite(motor[0], LOW);
    digitalWrite(motor[1], LOW);
    analogWrite(motor[2], 0);
  }
}

void setup()
{
  pinMode(buttonPin, INPUT);
  pinMode(dir1PinA, OUTPUT);
  pinMode(dir2PinA, OUTPUT);
  pinMode(dir1PinB, OUTPUT);
  pinMode(dir2PinB, OUTPUT);
  pinMode(speedA, OUTPUT);
  pinMode(speedB, OUTPUT);
  pinMode(LightA, OUTPUT);
  pinMode(ikPin, INPUT);
  pinMode (PIN_SWITCH_1, OUTPUT);
  RemoteXY_Init ();
  Serial.begin(9600);
}

void go_forward() {
  digitalWrite(dir1PinA, LOW);//едем вперед
  digitalWrite(dir2PinA, HIGH);
  digitalWrite(dir1PinB, LOW);
  digitalWrite(dir2PinB, HIGH);
  analogWrite(speedA, 150);
  analogWrite(speedB, 150);
}

void go_back() { //ну по названию понятно куда едем
  digitalWrite(dir1PinA, HIGH);
  digitalWrite(dir2PinA, LOW);
  digitalWrite(dir1PinB, HIGH);
  digitalWrite(dir2PinB, LOW);
  analogWrite(speedA, 150);
  analogWrite(speedB, 150);
  delay(1500);
}

void go_right() { //вправо
  digitalWrite(dir1PinA, HIGH);
  digitalWrite(dir2PinA, LOW);
  digitalWrite(dir1PinB, LOW);
  digitalWrite(dir2PinB, HIGH);
  analogWrite(speedA, 150);
  analogWrite(speedB, 150);
  delay(500);
}

void go_left() { //влево
  digitalWrite(dir1PinA, LOW);
  digitalWrite(dir2PinA, HIGH);
  digitalWrite(dir1PinB, HIGH);
  digitalWrite(dir2PinB, LOW);
  analogWrite(speedA, 150);
  analogWrite(speedB, 150);
  delay(500);
}

void stop_robot() { //стоп
  digitalWrite(dir1PinA, LOW);
  digitalWrite(dir2PinA, LOW);
  digitalWrite(dir1PinB, LOW);
  digitalWrite(dir2PinB, LOW);
  analogWrite(speedA, 0);
  analogWrite(speedB, 0);
  delay(1500);
}

void napravlenie1() {
  float dist_cm = ultrasonic.Ranging(CM);
  delay(100);
  napravlenie = random(1, 5);
      if (napravlenie == 1 && logicBlack == 0) {
        if (dist_cm < 25) {
          go_back();
        }
        logicBlack = 1;
      }else if (napravlenie == 2) {
        if (dist_cm < 25) {
          go_right();
        }
        logicBlack = 0;
      }else if (napravlenie == 3) {
        if (dist_cm < 25) {
          go_left();
        }
        logicBlack = 0;
      }else if (napravlenie == 4) {
        stop_robot();
        logicBlack = 0;
      }
}

void real() {
  digitalWrite(LightA, HIGH);
  digitalWrite(PIN_SWITCH_1, LOW);
  float dist_cm = ultrasonic.Ranging(CM);
  delay(100);
  if (dist_cm > 25) {
    val1 = digitalRead(ikPin);
    if (val1 == 1) {
      go_forward();
    } else {
      napravlenie1();
    }
  }
  else if (dist_cm < 25) {
    napravlenie1();
  }
}

void loop() {
  // считываем значения с входа кнопки
  buttonState = digitalRead(buttonPin);

  // проверяем нажата ли кнопка
  // если нажата, то buttonState будет HIGH:
  if (buttonState == HIGH) {
    // выключаем управление
    real();
  }
  else {

    // включаем управление
    /* обработчик событий модуля RemoteXY */
    RemoteXY_Handler ();

    /* управляем пином светодиода */
    digitalWrite (PIN_SWITCH_1, (RemoteXY.switch_1 == 0) ? LOW : HIGH);

    /* управляем правым мотором */
    Wheel (RightMotor, RemoteXY.joystick_1_y - RemoteXY.joystick_1_x);
    /* управляем левым мотором */
    Wheel (LeftMotor, RemoteXY.joystick_1_y + RemoteXY.joystick_1_x);
  }
}
