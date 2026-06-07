// Phase 2: Arduino CAN Shield to PC Serial interface
// Outputs CAN data and DHT11 data to Serial port for the C++ backend.

#include <df_can.h>
#include <SPI.h>
#include "DHT.h"

#define DHTPIN 3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);  
const int SPI_CS_PIN = 10;  
MCPCAN CAN(SPI_CS_PIN); 

unsigned char len = 0;
unsigned char buf[8];
unsigned long lastTempTime = 0;

void setup() {
  Serial.begin(115200); // 115200 is faster for CAN processing

  int count = 50;
  do {
    CAN.init();     
    CAN.init_Mask(MCP_RXM0, 0, 0x7ff);
    CAN.init_Mask(MCP_RXM1, 0, 0x7ff);
    CAN.init_Filter(MCP_RXF0, 0, 0x03);
    
    if(CAN_OK == CAN.begin(CAN_500KBPS)) {  
      Serial.println("SYSTEM,CAN BUS Shield init ok!");
      break;
    } else {
      delay(100);
      if (count == 0) Serial.println("SYSTEM,CAN BUS Shield init failed");
    }
  } while(count--);

  dht.begin();  
}

void loop() {
  // 1. Read CAN messages and forward to Serial
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    unsigned long id = CAN.getCanId();

    // Parse the first byte of data as an integer for our simple simulation
    // In a real car, data parsing depends on the specific CAN ID payload structure.
    int parsedData = 0;
    if (len > 0) {
      parsedData = buf[0]; 
    }

    // Output Format: 0xID,DATA
    // Example: 0x100,45
    Serial.print("0x");
    Serial.print(id, HEX);
    Serial.print(",");
    Serial.println(parsedData);
  }

  // 2. Read Temperature every 2 seconds and send as CAN ID 0x400
  unsigned long currentMillis = millis();
  if (currentMillis - lastTempTime >= 2000) {
    lastTempTime = currentMillis;
    float t = dht.readTemperature();
    
    if (!isnan(t)) {
      int tempInt = (int)t;
      // Send it over Serial as if it's a CAN message 0x400
      Serial.print("0x400,");
      Serial.println(tempInt);
    }
  }
}
