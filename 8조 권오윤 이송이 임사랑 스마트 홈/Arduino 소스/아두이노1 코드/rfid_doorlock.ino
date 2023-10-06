/*
  blue test: 
  http://www.kccistc.net/
  작성일 : 2022.12.19
  작성자 : IoT 임베디드 KSH
*/
#include <SoftwareSerial.h>

#define DEBUG
#include <SPI.h>
#include <MFRC522.h>
#include <MsTimer2.h>
#define SS_PIN 10
#define RST_PIN 9
#define LED_PIN 4
MFRC522 rfid(SS_PIN, RST_PIN);  //rfid 객체 생성

#include <Servo.h>  // 서보 라이브러리를 지정
#define servoPin 8  // 서보 모터핀을 지정
Servo servo;        // 서보 라이브러리 변수를 선언
int pos = 0;        // 현재 각도를 저장할 변수를 지정한다
int RLED = 3;       // 빨간색 LED단자를 아두이노 7번과 연결
int GLED = 2;       // 초록색 LED단자를 아두이노 6번과 연결
bool tagFlag = false;
bool updatTimeFlag = false;
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
} DATETIME;
DATETIME dateTime = {0, 0, 0, 12, 0, 0};

#define ARR_CNT 5
#define CMD_SIZE 60

char sendBuf[CMD_SIZE];
char recvId[10] = "KSH_SQL";  // SQL 저장 클라이이언트 ID

bool timerIsrFlag = false;
unsigned int secCount;
unsigned int myservoTime;

SoftwareSerial BTSerial(6, 7); // RX ==>BT:TXD, TX ==> BT:RXD

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("setup() start!");
  
#endif
  BTSerial.begin(9600); // set the data rate for the SoftwareSerial port
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);      // 블루투스 모듈과 시리얼 통신 속도 설정
  pinMode(RLED, OUTPUT);   // RED LED를 출력으로 지정
  pinMode(GLED, OUTPUT);   // GREEN LED를 출력으로 지정
  servo.attach(servoPin);  // 서보모터 핀을 설정한다
  servo.write(0);          // 서보모터 0도 위치로 초기화

  SPI.begin();      // SPI 통신 시작
  rfid.PCD_Init();  // rfid(MFRC522) 초기화
  Serial.println("Approximate your card to the reader...");
  Serial.println();

}

void loop()
{
  
  if (BTSerial.available())
  {
    bluetoothEvent();  
  }

  // 새카드 접촉이 있을 때만 다음 단계로 넘어감
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  // 카드 읽힘이 제대로 되면 다음으로 넘어감
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }
  // UID 값을 16진 값으로 읽고 저장한 후 시리얼 모니터로 표시함
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
    content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(rfid.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (tagFlag == false && content.substring(1) == "E1 E3 2D 0D")  // 엑세스 승인 하고자하는 UID 기록
  {//
    // 인증이 되면 Green LED와 함께 서보모터를 작동시킨다.
    Serial.println("Authorized access");
    Serial.println();
    digitalWrite(GLED, HIGH);
    servo.write(120);  // 서보모터의 각도를 변경한다
    if (servo.attached() && (myservoTime + 2 == secCount))
    {
      servo.detach();
    }
    tagFlag = true;                // 안열림
    sprintf(sendBuf, "[LSR_RAS]TAG@ON\n");
             //열림
    BTSerial.write(sendBuf);
    delay(1500);
                           // 서보 모터의 각도가 변하는 것을 기다려 준다.
    servo.write(0);                    // 시간지연 후 문을 닫는다
    digitalWrite(GLED, LOW);           // 시간지연 후 LED 끈다
  } 
  else if( tagFlag == true) {//                             // 서보모터는 작동 없이 Red LED만 켜고 끈다
    Serial.println(" Access denied");  // 그외 UID는 승인 거부 표시
    digitalWrite(RLED, HIGH);
    tagFlag = false;
    sprintf(sendBuf, "[LSR_RAS]TAG@OFF\n");
    BTSerial.write(sendBuf);  // 값을 블루투스로 전송
    delay(1500);
    digitalWrite(RLED, LOW);
    
    
  }

}
void bluetoothEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);

#ifdef DEBUG
  Serial.print("Recv : ");
  Serial.println(recvBuf);
#endif

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  //recvBuf : [XXX_BTM]LED@ON
  //pArray[0] = "XXX_LIN"   : 송신자 ID
  //pArray[1] = "LED"
  //pArray[2] = "ON"
  //pArray[3] = 0x0

  if (!strcmp(pArray[1], "LED")) {
    if (!strcmp(pArray[2], "ON")) {
      digitalWrite(LED_PIN, HIGH);
    }
    else if (!strcmp(pArray[2], "OFF")) {
      digitalWrite(LED_PIN, LOW);
    }
    sprintf(sendBuf, "[%s]%s@%s\n", pArray[0], pArray[1], pArray[2]);
  }
#ifdef DEBUG
  Serial.print("Send : ");
  Serial.print(sendBuf);
#endif
  BTSerial.write(sendBuf);
}
void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
  clock_calc(&dateTime);
}
void clock_calc(DATETIME *dateTime)
{
  int ret = 0;
  dateTime->sec++;          // increment second

  if(dateTime->sec >= 60)                              // if second = 60, second = 0
  { 
      dateTime->sec = 0;
      dateTime->min++; 
             
      if(dateTime->min >= 60)                          // if minute = 60, minute = 0
      { 
          dateTime->min = 0;
          dateTime->hour++;                               // increment hour
          if(dateTime->hour == 24) 
          {
            dateTime->hour = 0;
            updatTimeFlag = true;
          }
       }
    }
}