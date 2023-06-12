// LISTA DE REPRODUCCION - ESP32 - ESP8266
// https://www.youtube.com/playlist?list=PLZHVfZzF2DYID9jGK8EpcMni-U2CSTrw3

// LISTA DE REPRODCUION DEL CURSO DE ARDUINO DESDE CERO
// https://www.youtube.com/playlist?list=PLZHVfZzF2DYJeLXXxz6YtpBj4u7FoGPWN

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#define SS_PIN 5
#define RST_PIN 22

// RFID Variables
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
byte block;
byte len;
String info;
const int infoLength = 3;
String data[infoLength];
String fields[] = {"Name", "ID", "Object"};

// ESP32 Variables
const char* ssid = "FAMILIALOMBANA";
const char* password = "GL0MB4N463";
IPAddress serverIP(44, 198, 77, 190); // 44.198.77.190
unsigned int serverPort = 3306; 
char* user = "admin";
char* passwordDB = "admindbs";
char* database = "Teo_Info";
char* table = "logs";
WiFiClient client;
MySQL_Connection conn((Client *)&client);

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() 
{
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  len = 18;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  Serial.println("Iniciando el Programa");
  //startWifi();  //Uncomment when database works @MirandaHpt4
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
}

void startWifi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  
  Serial.println("Conectado a WiFi");
  
  Serial.println("Conectando a la base de datos...");
  if (conn.connect(serverIP, serverPort, user, passwordDB)) {
    Serial.println("Conectado a la base de datos");
  } else {
    Serial.println("Error de conexión");
    while (1);
  }
}

void loop() 
{
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( !rfid.PICC_IsNewCardPresent() ) return;

  // Select one of the cards
  if ( !rfid.PICC_ReadCardSerial() ) return;
     
  Serial.println(F("**Card Detected:**\n"));
  read();
  Serial.println(F("\n**End Reading**\n"));
  Serial.println(F("**Sending Data to Server:**"));
  //save();   //Uncomment when database works @MirandaHpt4
  Serial.println(F("**Data Sent Successfully**\n"));

  delay(300); //change value if you want to read cards faster

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void read(){
  int jump = 1;
  for(int i=0; i<infoLength; i++){
    block = i%3 + 4*jump;
    if(i%3==2) jump++;
    Serial.print(fields[i]+": \t");
    if(fields[i].length()<=4) Serial.print("\t");
    data[i] = readTag(block, key, len); //call read method
  }
}

String readTag(byte block, MFRC522::MIFARE_Key key, byte len){
  
  byte buffer[len];

  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid)); //Authenticate to read tag
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    resetFunc();
  }

  status = rfid.MIFARE_Read(block, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
    resetFunc();
  }

  for (uint8_t i = 0; i < 16; i++){
    if (buffer[i] != 32 && buffer[i] != 0){ //If its different from space or null, print
      Serial.write(buffer[i]);
    }
  }

  Serial.println();

  return String((char *)buffer);

}

void save() {

  String fecha_hora = obtenerFechaHoraActual();
  
  char query[256];
  snprintf(query, sizeof(query), "USE %s; INSERT INTO %s (item, ubicacion, fecha_hora) VALUES ('%s', '%s', '%s')", database, table, data[2], "A", fecha_hora.c_str());
  MySQL_Cursor* cursor = new MySQL_Cursor(&conn);

  cursor->execute(query);
  
  delete cursor;
  
  Serial.println("Consulta ejecutada");
  
}

String obtenerFechaHoraActual() {
  String fecha_hora;
  
  time_t now = time(nullptr);
  struct tm* timeinfo;
  timeinfo = localtime(&now);
  
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  
  fecha_hora = buffer;
  
  return fecha_hora;
}

/* ESP 32 NODE MCU
  Vcc <-> 3V3 (o Vin(5V) según la versión del módulo)
  RST (Reset) <-> D22
  GND (Masse) <-> GND
  MISO (Master Input Slave Output) <-> 19
  MOSI (Master Output Slave Input) <-> 23
  SCK (Serial Clock) <-> 18
  SS/SDA (Slave select) <-> 5
*/
