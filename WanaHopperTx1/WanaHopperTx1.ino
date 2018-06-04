/*
  中継機からメッセージを受け取り、LoRa電波を送信します。
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
  rf95.setFrequency(frequencyTx);
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
  for(int i = 0; i < 19 ; i++){
    Serial.print(F(" 0x"));Serial.print(data[i], HEX);
  }
  Serial.println();
}

void loop()
{
  while(mySerial.available()){
    uint8_t data[19];//receive data buffer
    uint8_t buf[256];

    for (int cnt = 0;cnt<256 && mySerial.available(); cnt++){
      buf[cnt] = mySerial.read();
      Serial.print(F(" cnt="));Serial.print(cnt);
      Serial.print(F(" 0x"));Serial.print(buf[cnt],HEX);
    }
    Serial.println();

    //最初のバイトと2番目のバイトが同じでない場合は、多重で入力されたもの
    //(送信側のArduinoから一回のTxにもかかわらず、2回受信される場合がある)
    if(buf[0]!=buf[1])break;

    for (int cnt = 0;cnt<19; cnt++)data[cnt]=buf[cnt+1];
    print19((char*)data);

    Serial.println(F("Send LoRa start"));
    rf95.send(data, sizeof(data));
    rf95.waitPacketSent();
    delay(5);
    Serial.println(F("Send LoRa end"));
    mySerial.write('0');//成功の場合は0を返す
  }
}
