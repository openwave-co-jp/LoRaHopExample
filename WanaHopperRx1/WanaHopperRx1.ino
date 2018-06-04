/*
  ノードからLoRa電波を受信し、送信モジュールにデータの内容を渡します。
  Support Devices: Dragino LoRa mini-JP 
  
  株式会社オープンウェーブ
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <Bridge.h>
#include <SoftwareSerial.h>

// Singleton instance of the radio driver
RH_RF95 rf95;

//If you use Dragino IoT Mesh Firmware, uncomment below lines.
//For product: LG01. 
#define BAUDRATE 115200

float frequencyRx = 923.6;
float frequencyTx = 920.6;
#define TX_DOUT A0
#define RX_DIN A1
SoftwareSerial mySerial(RX_DIN, TX_DOUT); // RX, TX

void setup()
{
  Serial.begin(BAUDRATE);
  //while(!Serial);

  if (!rf95.init())
    Serial.println("init failed");
  // Setup ISM frequency
  rf95.setFrequency(frequencyRx);
  // Setup Power,dBm
  rf95.setTxPower(13);
  // キャリアセンス5ms
  rf95.setCADTimeout((unsigned long)5);
  // Defaults after init are Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  RH_RF95::ModemConfig config;
  config.reg_1d = 0b01110000 | 0b0010; // Bw125        | Cr4/5
  config.reg_1e = 0b10100000 | 0b0100; // Sf1024(SF10) | CRCon
  config.reg_26 = 0b00000000; // default.
  rf95.setModemRegisters(&config);

  mySerial.begin(9600);
  Serial.println("Start Listening ");
}


void print19(char *data){
  for(int i = 1; i < 20 ; i++){
    Serial.print(F(" 0x"));Serial.print(data[i], HEX);
  }
  Serial.println();
}

void loop()
{
  if (rf95.available()){
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];//receive data buffer
    uint8_t len = sizeof(buf);//data buffer length
    if (rf95.recv(buf, &len)){
      len=strlen((char*)buf);
      char data[20];
      data[0]=buf[0];//同期の1バイトを追加(送信する値の最初のバイトを入れないと化ける（経験則）)
      for(int i=0; i<19; i++)data[i+1] = buf[i];
      Serial.println(F("Get Data from LoRa Node"));
      print19((char*)data);

      Serial.print(F("RSSI: "));
      Serial.println(rf95.lastRssi(), DEC);
            
      mySerial.write(data,20);
      char rx = '9';
      unsigned long start = millis();
      while (!mySerial.available() && start+1000>millis());//1秒以内にレスポンスがあること
      rx = mySerial.read();
      Serial.print(F("ret : "));
      Serial.println(rx);
   }else{
      Serial.println(F("recv failed"));
    }
  }
}
