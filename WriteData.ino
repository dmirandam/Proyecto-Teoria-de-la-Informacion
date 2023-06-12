#include <SPI.h>
#include <MFRC522.h>

/*Using Hardware SPI of Arduino */
/*MOSI (11), MISO (12) and SCK (13) are fixed */
/*You can configure SS and RST Pins*/
#define SS_PIN 5
#define RST_PIN 22

/* Create an instance of MFRC522 */
MFRC522 mfrc522(SS_PIN, RST_PIN);
/* Create an instance of MIFARE_Key */
MFRC522::MIFARE_Key key;          

/* Set the block to which we want to write data */
/* Be aware of Sector Trailer Blocks */
int block;  
int correct;
/* Create an array of 16 Bytes and fill it with data */
/* This is the actual data which is going to be written into the card */
byte blockData [16];

/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
int infoLength = 3;
String fields[] = {"Name", "ID", "Object"};

MFRC522::StatusCode status;

void setup(){
  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  /* Initialize SPI bus */
  SPI.begin();
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  Serial.println("Scan a MIFARE 1K Tag to write data...");
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop()
{
  
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( !mfrc522.PICC_IsNewCardPresent()) return;

  // Select one of the cards
  if ( !mfrc522.PICC_ReadCardSerial()) return;

  Serial.println(F("**Card Detected:**\n"));
         
  /* Call 'WriteDataToBlock' function, which will write data to the block */
  Serial.println("Writing to Data Block...\nPlease fill all the fields with max 15 char\n");
  
  int jump = 1;
  for(int i=0; i<infoLength; i++){
    block = i%3 + 4*jump;
    if(i%3==2) jump++;
    Serial.print(fields[i]+": \t");
    if(fields[i].length()<=4) Serial.print("\t");
    String info;
    while(Serial.available() == 0){}
    info = Serial.readString();
    Serial.print(info);
    info.trim();
    info.getBytes(blockData, bufferLen-2);
    correct = WriteDataToBlock(block, blockData, info.length());
    if(correct < 0) resetFunc();  //call reset;
  }
  

  Serial.println("\nAuthentication success");
  Serial.println("Data was written into Block successfully");
  Serial.println(F("\n**End Writing**"));

  delay(300); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}



int WriteDataToBlock(int blockNum, byte blockData[], int len) 
{
  /* Authenticating the desired data block for write access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }

  int size = len;
  if(size > 16) size = 16;

  byte blank[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  for (uint8_t i = 0; i < size; i++) if (blockData[i] != 32 && blockData[i] != 0) blank[i] = blockData[i];
  
  /* Write data to the block */
  status = mfrc522.MIFARE_Write(blockNum, blank, 16);
  if (status != MFRC522::STATUS_OK){
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return -1;
  }

  return 0;
  
}
