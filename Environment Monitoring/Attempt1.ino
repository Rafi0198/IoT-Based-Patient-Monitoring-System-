#include<Wire.h>
#include<DHT.h>
#define dhtPin 4
#define dhtType DHT11
#include<SoftwareSerial.h>
#include<LiquidCrystal_I2C.h>
#include<WiFi.h>
#include<ESP_Mail_Client.h>
//#include <NewPing.h>
#include <Adafruit_BMP085.h>
#include "arduino_secrets.h"
#include "thingProperties.h"

LiquidCrystal_I2C lcd (0x27, 16, 2);

SoftwareSerial gsm (16, 17); //(3,2) Rx, Tx

DHT dht (dhtPin, dhtType);

Adafruit_BMP085 bmp;

int smoke = 13; byte smoke_alert = 14;
byte fire = 27;  byte fire_alert = 26;
byte pump = 25; byte button=12; 

byte fan=2; int spd; 

float roomTemp; byte humidity;
float pressure; float pressure1; int alt; 

//byte trigPin =25; byte echoPin = 33;
//int maxDistance = 250;
//int distance;

//NewPing sonar(trigPin, echoPin, maxDistance);

int count1 = 0; int count2 = 0;

String msgFire="Warning! Fire Detected!! " ;
String msgSmoke="Warning! Excess Smoke Detected!! " ;

float Lat= 23.72678019639063; 
float Lon=90.3884859801158;

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

// WiFi Credinetials

const char* ssid = "Rafi" ;
const char* password = "qnh1013hg" ;

// Email Credinetials

#define SMTP_server "smtp.gmail.com"

#define SMTP_Port 465

#define sender_email "rehanaakterrumi71@gmail.com"

#define sender_password "sjuhbwjneoglsaab"

#define Recipient_email "hoquerafi727@gmail.com"

#define Recipient_name "THR"

SMTPSession smtp;

void setup() {
  // put your setup code here, to run once:

  pinMode(smoke, INPUT);
  pinMode(smoke_alert, OUTPUT);
  pinMode(fire, INPUT);
  pinMode(fire_alert, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(fan, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(fan, ledChannel);

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  dht.begin();
  bmp.begin(); 
  Serial.begin(9600);
  gsm.begin(9600);
  WiFi.begin(ssid , password);

  Serial.println("Initializing Netwrok...");

  gsm.println("AT");
  checkSerial();

  gsm.println("AT+CSQ");
  checkSerial();

  gsm.println("AT+CCID");
  checkSerial();

  gsm.println("AT+CREG?");
  checkSerial();

  gsm.println("AT+CBC");
  checkSerial();

  gsm.println("AT+COPS?");
  checkSerial();

  gsm.println("AT+CMGF=1"); // Initializes the text mode
  checkSerial();

  gsm.println("AT+CNMI=2,2,0,0,0"); // Decides how newly arrived messages will be handled
  checkSerial();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome 2 Rafi's");
  lcd.setCursor(0, 1);
  lcd.print("Smart Alert");
  delay(3000);

  // Connect to Wi-Fi
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(1000);
  //    Serial.println("Connecting to WiFi...");
  //  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();


  fireAlert=HIGH; 
  smokeAlert=HIGH; 
  digitalWrite(fire_alert,LOW); 
  digitalWrite(smoke_alert,LOW);

 // location1={23.72678019639063, 90.3884859801158}; 

}

void loop() {
  // put your main code here, to run repeatedly:

  ArduinoCloud.update();

  //location1=Location(23.72678019639063, 90.3884859801158); 

  location1=Location(Lat,Lon); 

  roomTemp = dht.readTemperature();
  humidity = dht.readHumidity();
  pressure1=bmp.readPressure(); 
  pressure=(pressure1/101325)*760 ; 
  alt=bmp.readAltitude(); 

  airPressure1=pressure; 

  /*Serial.print("Temperature is " );
  Serial.print(roomTemp);
  Serial.write(227);
  Serial.println("C");

  Serial.print("Humidity is " );
  Serial.print(humidity);
  Serial.println("%");

  Serial.print("Pressure is " );
  Serial.print(pressure);
  Serial.println("Pa");

  Serial.print("Altitude is " );
  Serial.print(alt);
  Serial.println("meter");*/

  
  //Fire


  if (digitalRead(fire) == LOW) {

    digitalWrite(fire_alert, HIGH);
    digitalWrite(pump, HIGH);

    fireAlert=LOW;
    ArduinoCloud.update();

    lcd.setCursor(0, 0);
    lcd.print("    CAUTION!    ");
    lcd.setCursor(0, 1);
    lcd.print("  FIRE! FIRE!   ");

    byte i; 

    for(i=0;i<5;i++)
      {
        digitalWrite(fire_alert, LOW);
        delay(800); 
        digitalWrite(fire_alert, HIGH);
        delay(800);
        
        }


    sendmail(msgFire);
    sms(msgFire);
    count1=1; 
  }

  else
  
   {
    
    if(count1==1)
    {
       digitalWrite(pump, LOW);
       //distance = readPing();

       //if (distance>5 && distance<10)
       if (digitalRead(button))
        {
          digitalWrite(fire_alert, LOW);
          fireAlert=HIGH;
           count1=0; 
        } 
    }

  }


  if (digitalRead(smoke) ==HIGH) {

    digitalWrite(smoke_alert, HIGH);
    smokeAlert=LOW; 
    ArduinoCloud.update();
    

    lcd.setCursor(0, 0);
    lcd.print("    WARNING!    ");
    lcd.setCursor(0, 1);
    lcd.print(" EXCESS  SMOKE!   ");

    byte j; 

    for(j=0;j<5;j++)
      {
        digitalWrite(smoke_alert, LOW);
        delay(800); 
        digitalWrite(smoke_alert, HIGH);
        delay(800);
        
        }
      
        sms(msgSmoke);
        sendmail(msgSmoke);
        count2=1;     
  }

  else  {

    
    if(count2==1)
    {
       //distance = readPing();

       //if (distance>5 && distance<10)
       if (digitalRead(button))
        {
          digitalWrite(smoke_alert, LOW);
          smokeAlert=HIGH; 
           count2=0; 
        } 
    }
  }

  
  //Serial.print("RT: "); 
  //Serial.println(roomTemp);


  if(roomTemp<29)
    {
      ledcWrite(ledChannel, ceil(0.5*255));
      //Serial.println("50% Speed"); 
      spd=50; 
      fanSpeed1=spd;
      
      
      }

  else if(roomTemp>=29 && roomTemp<29.5 )
    {
      ledcWrite(ledChannel, ceil(0.6*255));
     // Serial.println("60% Speed"); 
      spd=60; 
      fanSpeed1=spd;
      
      }
  else if(roomTemp>=29.5 && roomTemp<30)
    {
      ledcWrite(ledChannel, ceil(0.7*255));
      //Serial.println("70% Speed");
      fanSpeed1=0.7*255; 
      spd=70; 
      fanSpeed1=spd;
      
      }

  else if(roomTemp>=30 && roomTemp<31)
    {
      ledcWrite(ledChannel, ceil(0.8*255));
      //Serial.println("80% Speed"); 
      fanSpeed1=0.8*255;
      spd=80; 
      fanSpeed1=spd;
      
      }
  else if(roomTemp>=31 && roomTemp<32)
    {
      ledcWrite(ledChannel, ceil(0.9*255));
      //Serial.println("90% Speed"); 
      spd=90; 
      fanSpeed1=spd;
      
      }
  else if(roomTemp>=32)
    {
      ledcWrite(ledChannel, ceil(1*255));
      //Serial.println("100% Speed"); 
      spd=100; 
      fanSpeed1=spd;
      
      }

  if ((count1 == 0) & (count2 == 0)) {

    lcd.setCursor(0, 0);
    lcd.print("T&S:");
    lcd.print(roomTemp);
    lcd.write(223);
    lcd.print("C&");
    lcd.print(fanSpeed1); 
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("P:");
    lcd.print(pressure);
    lcd.print("mmHg");
    lcd.print("    ");

  }

 
}

void checkSerial() {

  delay(500); // Used to ensure enough lag time between the at commands

  //  while (Serial.available())
  //    gsm.write(Serial.read());

  while (gsm.available())
    Serial.write(gsm.read());

}

void sms(String MSG)  {

  gsm.println("ATD+8801613113688;"); // Dials the Destination Number ***Make Call Prior to Sending SMS
  checkSerial();

  delay(15000);

  gsm.println("ATH"); // Hangs up the call after 20 Seconds
  checkSerial();

  gsm.println("AT+CMGF=1");
  checkSerial();

  gsm.println("AT+CMGS=\"+8801613113688\""); // Set Destination Phone Number
  checkSerial();

  gsm.println(MSG); // Set Message Content
  checkSerial();

  gsm.write(26);

}

// Send email

void sendmail(String msg) {

  smtp.debug(1);

  ESP_Mail_Session session;

  session.server.host_name = SMTP_server ;

  session.server.port = SMTP_Port;

  session.login.email = sender_email;

  session.login.password = sender_password;

  session.login.user_domain = "";

  /* Declare the message class */

  SMTP_Message message;

  message.sender.name = "Patient Alert";

  message.sender.email = sender_email;

  message.subject = "Alert From Patient's Room";

  message.addRecipient(Recipient_name, Recipient_email);


  //Send simple text message

  String textMsg = msg; // "Door has been opened. Thank You."

  message.text.content = textMsg.c_str();

  message.text.charSet = "us-ascii";

  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  if (!smtp.connect(&session))

    return;

  if (!MailClient.sendMail(&smtp, &message))

    Serial.println("Error sending Email, " + smtp.errorReason());
}

/*int readPing() {
  //delay(70);
  int cm = sonar.ping_cm();
  if (cm == 0) {
    cm = 250;
  }
  return cm;
}*/
