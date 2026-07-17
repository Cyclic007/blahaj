#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SparkFun_ADXL345.h>

/*********** COMMUNICATION SELECTION ***********/
// ADXL345 adxl = ADXL345(15);     // SPI mode (your original code)
ADXL345 adxl = ADXL345();   // I2C mode if needed

/****************** INTERRUPT ******************/
int interruptPin = 2;

/****************** WiFi ******************/
const char *ssid = "test";
const char *password = "testest";

WiFiServer server(80);

/******************** SETUP ********************/
void setup() {

  Serial.begin(115200);
  delay(2000);
  Serial.println("SparkFun ADXL345 + ESP32 Webserver Example\n");
  Serial.flush();
  delay(1000);

  /* ----------- ADXL345 CONFIG ----------- */
  adxl.powerOn();

  adxl.setRangeSetting(16);
  adxl.setSpiBit(0);

  adxl.setActivityXYZ(1, 0, 0);
  adxl.setActivityThreshold(75);

  adxl.setInactivityXYZ(1, 0, 0);
  adxl.setInactivityThreshold(75);
  adxl.setTimeInactivity(10);

  adxl.setTapDetectionOnXYZ(0, 0, 1);

  adxl.setTapThreshold(50);
  adxl.setTapDuration(15);
  adxl.setDoubleTapLatency(80);
  adxl.setDoubleTapWindow(200);

  adxl.setFreeFallThreshold(7);
  adxl.setFreeFallDuration(30);

  adxl.InactivityINT(1);
  adxl.ActivityINT(1);
  adxl.FreeFallINT(1);
  adxl.doubleTapINT(1);
  adxl.singleTapINT(1);

  /* ----------- WiFi ----------- */
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

/****************** MAIN LOOP ******************/
void loop() {

  WiFiClient client = server.available();


  if (client) {
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected()) {
      //if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n') {

          if (currentLine.length() == 0) {

            /* ----------- READ ADXL345 ----------- */
            int x, y, z;
            adxl.readAccel(&x, &y, &z);
            char buf[100];
            int len = snprintf(buf, sizeof(buf), "{\"x\": %d,\"y\": %d,\"z\": %d}", x, y, z);

            /* ----------- SERVE WEBPAGE ----------- */
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type: text/plain");
            client.printf("Content-length: %u\r\n", len);
            client.println();


            client.println(buf);


            // client.print("{\"x\": "); client.print(x);
            // client.print(",\"y\": "); client.print(y);
            // client.print(",\"y\": "); client.print(z); client.println("}");
            break;

          } else {
            currentLine = "";
          }

        } else if (c != '\r') {
          currentLine += c;
        }
      //}
    }

    client.stop();
    Serial.println("Client Disconnected.");
  }
}

/********************* ISR *********************/
void ADXL_ISR() {

  byte interrupts = adxl.getInterruptSource();

  if (adxl.triggered(interrupts, ADXL345_FREE_FALL))
    Serial.println("*** FREE FALL ***");

  if (adxl.triggered(interrupts, ADXL345_INACTIVITY))
    Serial.println("*** INACTIVITY ***");

  if (adxl.triggered(interrupts, ADXL345_ACTIVITY))
    Serial.println("*** ACTIVITY ***");

  if (adxl.triggered(interrupts, ADXL345_DOUBLE_TAP))
    Serial.println("*** DOUBLE TAP ***");

  if (adxl.triggered(interrupts, ADXL345_SINGLE_TAP))
    Serial.println("*** TAP ***");
}