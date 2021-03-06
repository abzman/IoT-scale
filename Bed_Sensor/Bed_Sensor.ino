#include <Arduino.h>
#include <HX711.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x3F for a 16 chars and 2 line display

HX711 scale;                          // Initiate HX711 library
WiFiClient wifiClient;                // Initiate WiFi library
PubSubClient client(wifiClient);      // Initiate PubSubClient library
float last_reading = 0; // Float for last reading
void setup() {
  lcd.init(); // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("IoT Scale");
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print("Wifi / MQTT");
  delay(500);

  Serial.begin(74880);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting...");
  lcd.setCursor(0, 0);
  lcd.print("Wifi Connecting");
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print("                ");
  lcd.setCursor(0, 1); //(position on line, line)

  while (WiFi.status() != WL_CONNECTED) {       // Wait till Wifi connected
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  Serial.println();

  lcd.setCursor(0, 0);
  lcd.print("Connected, IP:  ");
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print("                ");
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print(WiFi.localIP());
  delay(500);
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());                     // Print IP address

  client.setServer(MQTT_SERVER, 1883);                // Set MQTT server and port number
  client.setCallback(callback);                       // Set callback address, this is used for remote tare
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);   // Start scale on specified pins
  scale.wait_ready();                                 //Ensure scale is ready, this is a blocking function
  scale.set_scale();
  Serial.println("Scale Set");
  lcd.setCursor(0, 0);
  lcd.print("Scale Set       ");
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print("                ");
  lcd.setCursor(0, 1); //(position on line, line)
  scale.wait_ready();
  scale.tare();                                       // Tare scale on startup
  scale.wait_ready();
  Serial.println("Scale Zeroed");
  lcd.print("Scale Zeroed");
  delay(500);
  lcd.noBacklight();
}

void loop() {
  float reading; // Float for reading
  float raw; // Float for raw value which can be useful
  scale.wait_ready(); // Wait till scale is ready, this is blocking if your hardware is not connected properly.
  scale.set_scale(calibration_factor);  // Sets the calibration factor.

  // Ensure we are still connected to MQTT Topics
  if (!client.connected()) {
    reconnect();
  }

  Serial.print("Reading: ");            // Prints weight readings in .2 decimal kg units.
  scale.wait_ready();
  reading = scale.get_units(10);        //Read scale in g/Kg
  reading = -1.0 * reading;             //did I wire this wrong? -abzman
  raw = scale.read_average(5);          //Read raw value from scale too
  Serial.print(reading, 2);
  Serial.println(" kg");
  Serial.print("Raw: ");
  Serial.println(raw);
  Serial.print("Calibration factor: "); // Prints calibration factor.
  Serial.println(calibration_factor);

  if (reading < 0) {
    reading = 0.00;     //Sets reading to 0 if it is a negative value, sometimes loadcells will drift into slightly negative values
  }
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print("Kg: ");
  lcd.print(reading);
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print("                ");
  lcd.setCursor(0, 1); //(position on line, line)
  lcd.print("lbs: ");
  lcd.print(reading * 2.21);

  String value_str = String(reading);
  String value_raw_str = String(raw);

  if ((abs(reading - last_reading) < 1) && reading > 80) // measure my weight
  {
    client.publish(STATE_TOPIC, (char *)value_str.c_str());               // Publish weight to the STATE topic
    lcd.backlight();
  }
  else
  {
    lcd.noBacklight();
  }
  client.publish(STATE_RAW_TOPIC, (char *)value_raw_str.c_str());       // Publish raw value to the RAW topic
  client.publish(AVAILABILITY_TOPIC, "Online");         // Once connected, publish online to the availability topic



  last_reading = reading;
  client.loop();          // MQTT task loop
  scale.power_down();    // Puts the scale to sleep mode for 3 seconds. I had issues getting readings if I did not do this
  delay(3000);
  scale.power_up();
}

void reconnect() {
  while (!client.connected()) {       // Loop until connected to MQTT server
    Serial.print("Attempting MQTT connection...");
    //boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
    //if (client.connect(HOSTNAME, mqtt_username, mqtt_password)) {       //Connect to MQTT server
    if (client.connect(HOSTNAME, mqtt_username, mqtt_password, AVAILABILITY_TOPIC, mqtt_will_qos, mqtt_will_retain, mqtt_will_message)) {       //Connect to MQTT server
      Serial.println("connected");
      client.publish(AVAILABILITY_TOPIC, "Online");         // Once connected, publish online to the availability topic
      client.subscribe(TARE_TOPIC);       //Subscribe to tare topic for remote tare
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Will attempt connection again in 5 seconds
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TARE_TOPIC) == 0) {
    Serial.println("starting tare...");
    scale.wait_ready();
    scale.set_scale();
    scale.tare();       //Reset scale to zero
    Serial.println("Scale reset to zero");
  }
}
