// @author PoofyPloop

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "ThingSpeak.h"
#include "mqtt_secrets.h"
#include "wifi_secrets.h"
// MPU 6050 sensor object
Adafruit_MPU6050 mpu;

// Wifi login details
char ssid[] = SECRET_SSID;                    // Change to your network SSID (name).
char pass[] = SECRET_PASS;                    // Change to your network password.

// interface to ThingSpeak MQTT interface
const char* mqttServer  = "mqtt3.thingspeak.com";
const uint16_t mqttPort = 1883;
char mqttUserName[] = SECRET_MQTT_USERNAME;      
char mqttPass[]     = SECRET_MQTT_PASSWORD;     
char clientID[]     = SECRET_MQTT_CLIENT_ID;    

// ThingSpeak ChannelID.
int  channelID      = /* Add */;

// this is our MQTT topic 
String  mqttTopic   = "channels/" + String(channelID) + "/publish";

// WiFi client
WiFiClient client;                                 // Initialize the Wi-Fi client library. Uncomment for nonsecure connection.

// MQTT publish/subscribe client
PubSubClient mqttClient( client );

// ***************************************************************************
// function to setup MPU6050.  configures acceleration range, angular rate change
// and filter bandwidth.
// NOTE:  This does not calibrate the sensor, which is required in a real application!
void configureMPU6050() {

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
}

void updateThingSpeak(int fieldId, String acceleration){
  String mqttData;

  if ( mqttClient.connect(clientID, mqttUserName, mqttPass)) {
    Serial.println("Connection made to MQTT broker!");
    
    // assemble data for broker in the form "field1=<data>&field2=<data>&field3=<data>..."
    // just use random numbers for this test...
    mqttData = "&field" + String(fieldId) + "= " + acceleration;
    Serial.println("  Publish data=" + mqttData);

    // publish data
    if ( mqttClient.publish(mqttTopic.c_str(), mqttData.c_str() )) {
      Serial.println("  Publish was successful!");
    } else {
      Serial.println("  Publish failed..");
    }

    // disconnect from broker.  if we were also a subscriber, we would not do this.
    mqttClient.disconnect();
  } else {
    // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
    Serial.print("  Failed with state: ");
    Serial.println(mqttClient.state());
  }
}


// ***************************************************************************
void setup(void) {
  Serial.begin(115200);

// connect to WiFi
  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  Serial.println("MPU6050 accel/gyro sesor test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // set up accel/gyro sensor
  configureMPU6050();
  
  Serial.println("");
  delay(100);

  // set MQTT broker info
  mqttClient.setServer(mqttServer, mqttPort);

  Serial.println("\n\n");
  Serial.println("Cloud");
}

// ***************************************************************************
void loop() {

  // Get new sensor events with the readings
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  updateThingSpeak(1, String(a.acceleration.x));
  updateThingSpeak(2, String(a.acceleration.y));
  updateThingSpeak(3, String(a.acceleration.z));

  // MQTT libraries need this
  mqttClient.loop();

  Serial.println("");
  delay(500);
}
