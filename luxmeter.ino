#include<LiquidCrystal_I2C.h>
#include<Wire.h>
#include<BluetoothSerial.h>

BluetoothSerial SerialBT;
LiquidCrystal_I2C lcd(0x27,16,2);

const int ldrpin=34;
const float resistors=3520;// i used 16 220 ohm resistors since i dont have 10k resistor
const float scaling_factor=350000000.0;
const float response_curve=1.4;
const int blue=13;
const int red=12;
const int buzzer=2;
const int redchannel=0;
const int bluechannel=1;
const int freq=5000;
const int res=8;
bool toggle=true;
bool lastToggle=true;
int samples=100;
unsigned long gap;
unsigned long start;
unsigned long prev=0;
unsigned long now;
bool ledstate=LOW;
bool sensorOn=true;
char message[20];
char lastMessage[20];

void blinkLED(float lux=-1.0){
if(lux==-1.0){
  now=millis();
  if(now-prev>=200){
    prev=now;
    ledstate=!ledstate;
    digitalWrite(red,ledstate);
    digitalWrite(blue,!ledstate);
  }
}
else{
  if(lux<250){
    digitalWrite(blue,LOW);
    digitalWrite(red,HIGH);
  }
  else{
    digitalWrite(red,LOW);
    digitalWrite(blue,HIGH);
  }
 }
}
void status(int value){
  lcd.setCursor(0,1);
   if(value<35){
    strncpy(message,"VERY LOW LIGHT  ",sizeof(message));
  }
  else if(value<80){
    strncpy(message,"LOW LIGHT       ",sizeof(message));
  }
  else if(value<250){
    strncpy(message,"AMBIENT LIGHT   ",sizeof(message));
  }
  else if(value<600){
    strncpy(message,"TASK LIGHT      ",sizeof(message));
  }
  else if(value<2500){
    strncpy(message,"HIGH TASK LIGHT ",sizeof(message));
  }
  else if(value<10000){
    strncpy(message,"BRIGHT LIGHT    ",sizeof(message));
  } 
  else{
    strncpy(message,"DAY LIGHT       ",sizeof(message));
  }
  message[sizeof(message)-1]='\0';
  if(strcmp(message,lastMessage)!=0){
    SerialBT.print("LAST:");
    SerialBT.println(lastMessage);
    strcpy(lastMessage,message);
    digitalWrite(buzzer,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
  }
  lcd.print(message);
  SerialBT.print("STATUS:");
  SerialBT.println(message);
}
float filterReading(){
  long total=0;
  for(int i=0;i<samples;i++){
    total+=analogRead(ldrpin);
    delay(10);
  }
  int average=total/samples;
  float voltage=average*3.3/4095.0;
  if(voltage<0.1)voltage=0.1;
  float ldr_resistance=resistors*(3.3/voltage-1);
  return scaling_factor/pow(ldr_resistance,(double)response_curve);
}
void setup() {
  // put your setup code here, to run once:
  SerialBT.begin("LUX METER");
  Serial.begin(115200);
  Wire.begin(21,22);
  lcd.init();
  lcd.backlight();
  strcpy(lastMessage," ");
  pinMode(red,OUTPUT);
  pinMode(blue,OUTPUT);
  pinMode(buzzer,OUTPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(ldrpin,ADC_11db);
  lcd.clear();
  while(true){
    if(SerialBT.hasClient()){
      start=millis();
      lcd.setCursor(0,0);
      lcd.print("CONNECTED    ");
      lcd.setCursor(0,1);
      lcd.print("SENSOR ON  ");
      digitalWrite(red,HIGH);
      digitalWrite(blue,HIGH);
      digitalWrite(buzzer,HIGH);
      if(start-gap>=1000){
        lcd.clear();
        digitalWrite(buzzer,LOW);
        break;
      }
    }
    else{
      gap=millis();
      blinkLED();
      lcd.setCursor(0,0);
      lcd.print("NOT CONNECTED");
      lcd.setCursor(0,1);
      lcd.print("SENSOR OFF");
    }
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  if(toggle!=lastToggle&&sensorOn){
    if(SerialBT.hasClient()){
      lcd.setCursor(0,0);
      lcd.print("CONNECTED    ");
      lcd.setCursor(0,1);
      lcd.print("SENSOR ON    ");
    }
    sensorOn=false;
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
    lastToggle=true;
  }
  if(SerialBT.hasClient()){
   sensorOn=true; 
   if(lastToggle){
   float lux=filterReading();
   blinkLED(lux);
   Serial.println(lux,2);
   lcd.setCursor(0,0);
   lcd.print("Lux:");
   lcd.print(lux,2);
   lcd.print("      ");
   SerialBT.print("Lux:");
   SerialBT.println(lux);
   status(lux);
   }
  }
  else{
    blinkLED();
    lastToggle=false;
    lcd.setCursor(0,0);
    lcd.print("DISCONNECTED    ");
    lcd.setCursor(0,1);
    lcd.print("SENSOR OFF      ");
  }
}
