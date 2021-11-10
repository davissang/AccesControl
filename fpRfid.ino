#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY 7   //relay pin
#define ACCESS_DELAY 2500

SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
//String UID = ;

String read_rfid;
String ok_rfid_1 = "d73e893f";
//String ok_rfid_2="deae805c";
/*
   Initialize.
*/

void setup() {

  Serial.begin(9600);  // Initialize serial communications with the PC

  while (!Serial);            // Do nothing if no serial port is opened
  SPI.begin();                // Init SPI bus
  finger.begin(57600);        // Init Fingerprint
  mfrc522.PCD_Init();         // Init MFRC522 card

  digitalWrite(RELAY, LOW);
  pinMode(RELAY, OUTPUT);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  }
  else {
    Serial.println("Did not find fingerprint sensor :(");

    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger or Card");

}
/*
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  read_rfid = "";
  for (byte i = 0; i < bufferSize; i++) {
    read_rfid = read_rfid + String(buffer[i], HEX);
  }
}
void open_lock() {
  //Use this routine when working with Relays and Solenoids etc.
  digitalWrite(RELAY, HIGH);
  delay(ACCESS_DELAY);
  digitalWrite(RELAY, LOW);
}
void loop() {
  getFingerprintIDez();
  getRfid();
  delay(50); 
}

// returns -1 if failed, otherwise returns ID #

int getFingerprintIDez() {
//  getRfid();
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)
    return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  
    return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    return -1;
  }
  open_lock();
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  getRfid();
  return finger.fingerID;
}
void getRfid() {
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  // Look for new cards
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println(read_rfid);
  if (read_rfid == ok_rfid_1) {
    Serial.println("Open Door");
    open_lock(); 
  }
  else {
    Serial.println("Access Denied");
  }
  getFingerprintIDez();
}
