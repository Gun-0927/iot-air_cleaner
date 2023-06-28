#include <SoftwareSerial.h>
SoftwareSerial BTserial(7,6);
#include <LiquidCrystal_I2C.h>
#include<Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2);

#include <DHT.h>
#define  DHTPIN 9
DHT dht11(DHTPIN,DHT11);


#define GET_DUST_VOLTAGE_INTV 200   
#define dust_array 15
 
#define DUST_CLEAN_VOLTAGE  0.8 
 
int dustVoutPin=A0;
int dustLED=2;
 
int dustList[dust_array];
int darray = 0;
float dustDensity = 0;
float humi = 0;
float temp = 0;
 
unsigned long sumVo;
unsigned long avgVo;
 
unsigned long prevMillisOfVoutReadValue = 0;
unsigned long prevMillisOfDust = 0;
unsigned long prevMillisofdht11 = 0;
int redPin = 3;
int greenPin = 4;
int bluePin = 5;
char message;
char message_state;
int INA= 10 ;
int INB =11;
#define fan2 12
 byte smile_Left[8]={B11111,B10000,B10010,B10000,B10100,B10011,B10000,B11111};
 byte smile_Right[8]={ B11111,B00001,B01001,B00001,B00101,B11001,B00001,B11111};
 byte normal_Left[8]={B11111,B10000,B10010,B10000,B10000,B10111,B10000,B11111};
 byte normal_Right[8]={B11111,B00001,B01001, B00001,B00001, B11101, B00001, B11111};
 byte sad_Left[8]={ B11111,B10000,B11110,B10100,B10100,B10000,B10000,B11111};
 byte sad_Right[8] = {B11111,B00001,B01111,B00101,B00101,B00001,B00001,B11111 };
 byte temp_icon[8]={B00100,B01010,B01010, B01110,B01110,B11111,B11111,B01110};
byte water[8] = {B00100,B01010,B01010,B10001,B11111,B11111,B11111,B01110};

 
void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.begin(16,2);
  lcd.createChar(0, smile_Left);
  lcd.createChar(1, smile_Right);
  lcd.createChar(2, normal_Left);
  lcd.createChar(3, normal_Right);
  lcd.createChar(4, sad_Left);
  lcd.createChar(5, sad_Right);
  lcd.createChar(6, temp_icon);
  lcd.createChar(7, water);

  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Dust: ");
  lcd.setCursor(0,1);
  lcd.write(7);
  lcd.setCursor(8,1);
  lcd.write(6);
  pinMode(dustVoutPin,INPUT);
  pinMode(dustLED,OUTPUT);
  dht11.begin();
  pinMode(redPin,OUTPUT);
  pinMode(greenPin,OUTPUT);
  pinMode(bluePin,OUTPUT);
  BTserial.begin(9600); 
  pinMode(fan2,OUTPUT); 
  pinMode(INA,OUTPUT);


}
 
 
void loop()
{
  dht11_read();
  dust_measurement();
   if(dustDensity>80){
    fan_on();
  }
  else{
    fan_off();
  }
  RGB();
  showInfo();
  bt();
  btf();
  

}
 
float dust_measurement() {
 
  sumVo = 0;
 
  if (millis() - prevMillisOfVoutReadValue >= GET_DUST_VOLTAGE_INTV) {
    prevMillisOfVoutReadValue = millis();
    
    digitalWrite(dustLED, LOW);
    delayMicroseconds(280);
    dustList[darray] = analogRead(dustVoutPin);
    delayMicroseconds(40);
    digitalWrite(dustLED, HIGH);
    
    darray = (darray + 1) % dust_array;
    
    for (int i = 0; i < dust_array; i++) {
      sumVo = sumVo + dustList[i];
    }
    avgVo = sumVo / dust_array;
  }
  float dustVo=  avgVo *(5.0 / 1024.0);
  
 
  if (millis() - prevMillisOfDust >= 1000) {
    prevMillisOfDust = millis();
    if (dustVo > DUST_CLEAN_VOLTAGE) {
      dustDensity = (dustVo - DUST_CLEAN_VOLTAGE) / 0.005;
    }
    else {
      dustDensity = 0;
    }
    Serial.print("Dust Density = ");
    Serial.print(dustDensity);
    Serial.println(" [ug/m3]");
    if(dustDensity >0 && dustDensity <=30){
      Serial.println("미세먼지: 좋음");
    }
    else if (dustDensity >30 && dustDensity <=80){
      Serial.println("미세먼지: 보통");
    }
    else if (dustDensity >80 && dustDensity <=150){
      Serial.println("미세먼지: 나쁨");
    }
    else if (dustDensity >150){
      Serial.println("미세먼지: 매우나쁨");
    }
  }
 
 }

  void dht11_read(){
    if(millis() - prevMillisofdht11>=1000){
      prevMillisofdht11 = millis();
  
     humi = dht11.readHumidity();
     temp = dht11.readTemperature();
    
    if(isnan(humi) || isnan(temp)){
      Serial.println("오류");
      
    }
    Serial.print("습도: ");
    Serial.println(humi);
    Serial.print("온도: ");
    Serial.println(temp); 
  }
 }
 void fan_on(){
  analogWrite(INA,150);
  analogWrite(INB,0);
  digitalWrite(fan2,HIGH);
}
void fan_off(){
  analogWrite(INA,0);
  analogWrite(INB,0);  
  digitalWrite(fan2,LOW);
}
void RGB(){

   if(dustDensity >0 && dustDensity <=30){
      digitalWrite(redPin,LOW);
      digitalWrite(greenPin,LOW);
      digitalWrite(bluePin,HIGH);
    }
    else if (dustDensity >30 && dustDensity <=80){
     digitalWrite(redPin,LOW);
      digitalWrite(greenPin,HIGH);
      digitalWrite(bluePin,LOW);
    }
    else if (dustDensity >80){
      digitalWrite(redPin,HIGH);
      digitalWrite(greenPin,LOW);
      digitalWrite(bluePin,LOW);
    }

  }

void showInfo(){
  if(dustDensity >0 && dustDensity <=30){
      lcd.setCursor(6,0);
      lcd.write(0);
      lcd.write(1);
    }
    else if (dustDensity >30 && dustDensity <=80){
       lcd.setCursor(6,0);
      lcd.write(2);
      lcd.write(3);
    }
    else if (dustDensity >80){
      lcd.setCursor(6,0); 
      lcd.write(4);
      lcd.write(5);
    }
    lcd.setCursor(1,1);
    lcd.print(humi);
    lcd.print("%");
    lcd.setCursor(9,1);
    lcd.print(temp);
    lcd.print("C");   
}

void bt() {
  String data = String(dustDensity)+","+String(humi)+","+String(temp);
  BTserial.print(data);
  delay(1000);
  }
  void btf() {
    if(BTserial.available()>0){
    message = BTserial.read();
    message_state = message;
   Serial.println(message);
  while(message_state=='1'){
    fan_on();
 
      message = BTserial.read();
      if(message == '2'){
        break;
      }
    }
    fan_off();
 
    }
  }
