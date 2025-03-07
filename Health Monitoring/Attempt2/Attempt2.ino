#include <Wire.h>
#include<OneWire.h>
#include<DallasTemperature.h>
#include<LiquidCrystal_I2C.h>
#include<SoftwareSerial.h>
#include<DHT.h>
#include "arduino_secrets.h"
#include "thingProperties.h"
#include "MAX30105.h"

#include "heartRate.h"

#include<WiFi.h>
#include<ESP_Mail_Client.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);
SoftwareSerial gsm(16, 17);

// Setting Up Body Temp sensor

#define ONE_WIRE_BUS 2
OneWire oneWire (ONE_WIRE_BUS);
DallasTemperature sensors (&oneWire);
float bodyTempC = 0; float  bodyTempF = 0;


MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg; byte count1 = 0;

#define DHTtype DHT11
#define DHTpin 4
DHT dht (DHTpin, DHTtype);
float roomTemp; byte humidity;

byte emergencyAlert=26; 


// WiFi Credinetials

const char* ssid = "M" ;
const char* password = "00001111" ;

//Defining touch buttons of UI

#define button1 13
#define button2 12
#define button3 14
#define button4 27

// Email Credinetials

#define SMTP_server "smtp.gmail.com"

#define SMTP_Port 465

#define sender_email "rehanaakterrumi71@gmail.com"

#define sender_password "sjuhbwjneoglsaab"

#define Recipient_email "hoquerafi727@gmail.com"

#define Recipient_name "THR"

SMTPSession smtp;

//long irValue; long delta; 
byte k; 

void setup()
{

  Serial.begin(9600);
  Serial.println("Initializing...");
  dht.begin();
  gsm.begin(9600);
  WiFi.begin(ssid , password);
  
 // Defined in thingProperties.h
  initProperties();
 // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  pinMode(emergencyAlert,OUTPUT);

  lcd.init();
  lcd.backlight();


  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO RAFI'S");
  lcd.setCursor(0, 1);
  lcd.print("SMART HEALTH CARE");
  lcd.setCursor(0, 2);
  lcd.print("UNDERGRAD STUDENT");
  lcd.setCursor(0, 3);
  lcd.print("DEPT. OF EEE, BUET");


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

  //gsm.println("AT+CMGD=1,4");
  //checkSerial();

  // Connect to Wi-Fi
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(1000);
  //    Serial.println("Connecting to WiFi...");
  //  }

  Serial.println("Code executed so far");
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());




  delay(4000);
  lcd.clear();
}

void loop()
{

  ArduinoCloud.update();

  roomTemp = dht.readTemperature();
  humidity = dht.readHumidity();

  roomTemp1 =  roomTemp; 
  humidity1 =  humidity; 

  

  lcd.setCursor(0, 0);
  lcd.print("ENTER 1 FOR BPM      ");

  lcd.setCursor(0, 1);
  lcd.print("ENTER 2 FOR BODY TP ");

  lcd.setCursor(0, 2);
  lcd.print("ENTER 4 TO SEND MSG ");

  lcd.setCursor(0, 3);
  lcd.print("T&H: ");
  lcd.print(roomTemp);
  lcd.write(223);
  lcd.print("C & ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.print("     ");

  

  if (digitalRead(button1) == HIGH)
  {
    // Initialize sensor
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
    {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
      while (1);
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");

    particleSensor.setup(); //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
    
    beatsPerMinute = 0; beatAvg = 0;

    long previousTime = (millis()) / 1000;
    long currentTime = (millis()) / 1000;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("INSTANT HR: "); //12
    lcd.setCursor(0, 2);
    lcd.print("AVG HEART BPM: ");//15

    while (millis() - previousTime * 1000 < 30000)
    {

      long irValue = particleSensor.getIR();

      if (checkForBeat(irValue) == true)
      {
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();

        beatsPerMinute = 60 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
          rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
          rateSpot %= RATE_SIZE; //Wrap variable

          //Take average of readings
          beatAvg = 0;
          for (byte x = 0 ; x < RATE_SIZE ; x++)
            beatAvg += rates[x];
          beatAvg /= RATE_SIZE;
        }
      }

      //      Serial.print("IR=");
      //      Serial.print(irValue);
      //      Serial.print(", BPM=");
      //      Serial.print(beatsPerMinute);
      //      Serial.print(", Avg BPM=");
      //      Serial.print(beatAvg);
      //
      //      if (irValue < 50000)
      //        Serial.print(" No finger?");
      //
      //      Serial.println();


      lcd.setCursor(12, 0);
      lcd.print(ceil(beatsPerMinute)); //12
      lcd.setCursor(15, 2);
      lcd.print(int(beatAvg));//15
      beatAvg1 = beatAvg; 

    }

  }

//Measuring Body Temp

  if (digitalRead(button2) == HIGH)
  {

    lcd.clear();
    //delay(500);
    long previousTime = (millis()) / 1000;
    long currentTime = (millis()) / 1000;

    Serial.println(previousTime);



    while (millis() - previousTime * 1000 < 40000)

    {

      sensors.requestTemperatures();
      bodyTempC = sensors.getTempFByIndex(0);
      bodyTempC1=  bodyTempC; 
      //Serial.println(bodyTempC);

      if ((millis() - currentTime * 1000) > 1000)
      {
        count1++;
        currentTime = millis() / 1000;
        //Serial.println(count1);

      }
      lcd.setCursor(0, 0);
      lcd.print("      MEASURING     ");
      lcd.setCursor(0, 1);
      lcd.print("PLEASE WAIT ");
      //lcd.setCursor(12, 1);
      lcd.print(count1);
      lcd.print(" SEC");
      lcd.print("  ");
      lcd.setCursor(0, 2);
      lcd.print("BODY TEMP: ");
      lcd.print(bodyTempC);
      lcd.write(223);
      lcd.print("F  ");
      lcd.setCursor(0, 3);
      lcd.print("T&H: ");
      lcd.print(roomTemp);
      lcd.write(223);
      lcd.print("C & ");
      lcd.print(humidity);
      lcd.print("%");
      lcd.print("     ");

    }

    count1 = 0;
    lcd.clear();

  }

  //Sending SMS & Email

  if (digitalRead(button4) == HIGH)

  {

    char msgHealth[100]; // Assuming a buffer size of 100 characters

    sprintf(msgHealth, "Average BPM: %d \nRoom Temperature: %.2f°C\nBody Temperature: %.2f°F \nHumidity: %d%%", beatAvg , roomTemp,bodyTempC, humidity);
   // msgHealth = ("Body Temperaure: %f .\nRoom T&H %f & %d", bodyTempC, roomTemp, humidity);

    String smsHealth = "Average BPM: " + String(beatAvg) + " \n"
                       "Body Temperature: " + String(bodyTempC) + " *F\n"
                       "Room Temperature: " + String(roomTemp) + " *C\n"
                       "Humidity: " + String(humidity) + "%" ;

    lcd.clear();
    //delay(500);
    lcd.setCursor(0, 0);
    lcd.print("SENDING FIRST ALERT");
    lcd.setCursor(0, 2);
    lcd.print("PLEASE WAIT A BIT...");

    sendmail(msgHealth);

    lcd.setCursor(0, 0);
    lcd.print("SENDING SECOND ALERT");
    //smsHealth();
    sms(smsHealth);
    lcd.clear();
    // delay(500);


  }
}


void checkSerial() {

  delay(500); // Used to ensure enough lag time between the at commands

  //  while (Serial.available())
  //    gsm.write(Serial.read());

  while (gsm.available())
    Serial.write(gsm.read());

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

  message.sender.name = "HEALTH ALERT";

  message.sender.email = sender_email;

  message.subject = "Alert From Rafi's Health Care";

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

void sms (String MSG) {

  delay(500);

  gsm.println("AT+CMGF=1");
  checkSerial();

  gsm.println("AT+CMGS=\"+8801613113688\""); // Set Destination Phone Number
  checkSerial();

  gsm.print(MSG); // Set Message Content
  delay(100);
  checkSerial();

  gsm.write(26);
  //gsm.println((char)26);
  //delay(5000);

}

void onEmergencyChange()  {
  // What happens upon Emergency change

  if (emergency==true)
    {
      digitalWrite(emergencyAlert,HIGH); 
      
      }
      
  else 
  {
      digitalWrite(emergencyAlert,LOW); 
      
      }
  
}
