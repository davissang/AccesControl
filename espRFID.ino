
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_Fingerprint.h>

#define SS_PIN    5  // ESP32 pin GIOP5 
#define RST_PIN   27 // ESP32 pin GIOP27 
#define RELAY 22 // ESP32 pin GIOP22 connects to relay
#define ACCESS_DELAY 2500

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

MFRC522 rfid(SS_PIN, RST_PIN);
String read_rfid;
String ok_rfid_1 = "d73e893f";

void setup (){
  Serial.begin(57600);
  Serial2.begin(115200);
  
  while (!Serial);
  SPI.begin();
  finger.begin(57600);
  rfid.PCD_Init();
  
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
void loop (){
  getFingerprintIDez();
  getRfid();
  delay(50);
}
int getFingerprintIDez() {
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
  Serial.print("Door opened by  ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  getRfid();
  return finger.fingerID;
}
void getRfid() {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  // Look for new cards
  dump_byte_array(rfid.uid.uidByte, rfid.uid.size);
  
  if (read_rfid == ok_rfid_1) {
   Serial.print(read_rfid); Serial.println(" Opened the Door");
    open_lock();
  }
  else {
    Serial.println("Access Denied");
  }
  getFingerprintIDez();
}
