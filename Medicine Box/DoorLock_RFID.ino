#include <SPI.h>
#include <MFRC522.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
#include<WiFi.h>
#include<ESP_Mail_Client.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);

#define SS_PIN 21
#define RST_PIN 22  //2
//byte card_pin = 6;
//byte ring_pin = 7;


const byte relay = 4; 
const byte alert1 = 13; const byte alert2 = 12; // Red & Green
byte state=0;

#define card "DA9ADFB"
#define tag  "C3349DD"
#define nfc "04DE90CA792B80"

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// WiFi Credinetials

const char* ssid = "Mi" ;
const char* password = "00001111" ;

// Email Credinetials

#define SMTP_server "smtp.gmail.com"

#define SMTP_Port 465

#define sender_email "rehanaakterrumi71@gmail.com"

#define sender_password "sjuhbwjneoglsaab"

#define Recipient_email "hoquerafi727@gmail.com"

#define Recipient_name "THR"

SMTPSession smtp;

String msg1 = "Medicines have been delivered to the patient.";
String msg2 = "Suspicious Unlocking Attempt Detected on the Medical Box.";

String tagID = "";

byte flag1 = 0; byte flag2 = 0;


byte lock[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b11111
};

byte unlock[8] = {
  0b00110,
  0b00001,
  0b00001,
  0b11111,
  0b11011,
  0b11011,
  0b11111,
  0b11111
};

byte angry[8] = {
  0b11111,
  0b11111,
  0b01110,
  0b11111,
  0b11011,
  0b11111,
  0b10001,
  0b11111
};

byte tick[8] = {
  0b00000,
  0b00000,
  0b00001,
  0b00010,
  0b10100,
  0b01000,
  0b00000,
  0b00000
};

byte bell[] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B00000,
  B00100,
  B00000
};


void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();
  WiFi.begin(ssid , password);
  Wire.begin(27, 26, 100000);
 
  // Initiate  SPI bus

  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  lcd.init();
  lcd.backlight();

  lcd.createChar(0, lock);
  lcd.createChar(1, unlock);
  lcd.createChar(2, angry);
  lcd.createChar(3, tick);
  lcd.createChar(4, bell);

  lcd.setCursor(0, 0);
  lcd.print("SECURED MEDICINE");
  lcd.setCursor(0, 1);
  lcd.print("DELIVERY BOX");

  delay(4000);
  lcd.clear();

  // Connect to Wi-Fi
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi...");
//  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  pinMode(relay, OUTPUT);
  pinMode(alert1, OUTPUT);
  pinMode(alert2, OUTPUT);
  digitalWrite(relay, LOW);
  digitalWrite(alert1, LOW);
  digitalWrite(alert2, LOW);

}

void loop()
{

  lcd.setCursor(0, 0);
  lcd.print("BOX IS LOCKED");
  lcd.write(byte(0)); lcd.write(byte(0)); lcd.write(byte(0));

  lcd.setCursor(0, 1);
  lcd.print("SCAN YOUR ID... ");

  
  // Look for new cards

  //state= getID(); 

 // Serial.println(state);
  
  while (getID())
  {
    //Serial.println(getID()); 

    Serial.println("Card Detected!");
    Serial.print("Tag ID: ");
    Serial.println(tagID);

    if ((tagID == card) ||( tagID==tag)) //change here the UID of the card/cards that you want to give access
    {

      digitalWrite(relay, HIGH);
      digitalWrite(alert2, HIGH);
      //digitalWrite(card_pin, HIGH);

      Serial.println("Authorized access to this card");
      Serial.println();



      lcd.setCursor(0, 0);
      lcd.print("ID VERIFIED ");
      lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));
      lcd.print(" ");

      lcd.setCursor(0, 1);
      lcd.print("BOX UNLOCKED ");
      lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));

      delay(4000);
      digitalWrite(relay, LOW);
      digitalWrite(alert2, LOW);
      lcd.clear();

      delay(500);
      lcd.setCursor(0, 0);
      lcd.print("SENDING EMAIL...");
      lcd.setCursor(0, 1);
      lcd.print("PLEASE WAIT...  ");
      sendmail(msg1);

      lcd.clear();
      delay(500);

      //flag1=1; 

    }
//    else if (tagID == tag)
//    {
//      digitalWrite(relay, HIGH);
//      //digitalWrite(card_pin, HIGH);
//
//      Serial.println("Authorized access to this card");
//      Serial.println();
//
//
//
//      lcd.setCursor(0, 0);
//      lcd.print("ID VERIFIED ");
//      lcd.write(byte(4)); lcd.write(byte(4)); lcd.write(byte(4));
//      lcd.print("    ");
//
//      lcd.setCursor(0, 1);
//      lcd.print("        ");
//      lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3)); lcd.write(byte(3));
//      lcd.print("        ");
//
//      lcd.setCursor(0, 2);
//      lcd.print("                    ");
//
//      lcd.setCursor(0, 3);
//      lcd.print("DOOR UNLOCKED ");
//      digitalWrite(alert2, HIGH);
//      lcd.write(byte(1)); lcd.write(byte(1)); lcd.write(byte(1));
//
//      delay(3000);
//      digitalWrite(relay, LOW);
//      digitalWrite(alert2, LOW);
//      lcd.clear();
//
//      delay(500);
//      lcd.setCursor(0, 0);
//      lcd.print("SENDING FIRST ALERT");
//      lcd.setCursor(0, 2);
//      lcd.print("PLEASE WAIT A BIT...");
//      sendmail(msg1);
//
//      lcd.setCursor(0, 0);
//      lcd.print("SENDING SECOND ALERT");
//      sms(msg1);
//
//      lcd.clear();
//      delay(500);
//      flag1=1;
//    }
    else
    {

        Serial.println(" Access denied");
        
        byte k;
        
        lcd.setCursor(0, 0);
        lcd.print("UNAUTHORIZED ID!");

        lcd.setCursor(0, 1);
        lcd.print("PLEASE TRY AGAIN");

        for (k=0; k<=5;k++)
          {
            digitalWrite (alert1,HIGH); 
            delay(400); 
            digitalWrite (alert1,LOW);
            delay(400);
          }
 
        //digitalWrite(alert1, LOW);

        lcd.clear();
        delay(500);

        lcd.setCursor(0, 0);
        lcd.print("SENDING EMAIL...");
        lcd.setCursor(0, 1);
        lcd.print("PLEASE WAIT...");

        sendmail(msg2);
        
        lcd.clear();
      }
    }
    
}
 

boolean getID()
{
  //Serial.println("Check1");
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return false;
  }

  //Serial.println("Check");
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) { // The MIFARE PICCs that we use have 4 byte UID
    //readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading
  return true;
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

  message.sender.name = "DELIVERY ALERT";

  message.sender.email = sender_email;

  message.subject = "Alert From Rafi's Automation";

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
