#define RXD2 16
#define TXD2 17
char c;
String readString;
void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.println("serial2test");
  Serial.println("Serial Txd is on pin: " + String(TX));
  Serial.println("Serial Rxd is on pin: " + String(RX));

}

void loop() {
  while (Serial2.available()) {
    c = Serial2.read();
    readString += c;
  }
  if (readString.length() > 0) {
    Serial.print(readString);
    Serial2.print(readString);
    //server.print(readString);
    readString = "";
  }
}
