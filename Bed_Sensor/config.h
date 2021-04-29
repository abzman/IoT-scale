
// Wifi Settings
#define SSID                          "abz-fi"
#define PASSWORD                      "x"

// MQTT Settings
#define HOSTNAME                      "scale"
#define MQTT_SERVER                   "192.168.1.11"
#define STATE_TOPIC                   "tele/evan/inside/bathroom/scale"
#define STATE_RAW_TOPIC               "tele/evan/inside/bathroom/scale/raw"
#define AVAILABILITY_TOPIC            "tele/evan/inside/bathroom/scale/LWT"
#define TARE_TOPIC                    "cmnd/evan/inside/bathroom/scale/tare"
#define mqtt_username                 "mosquitto"
#define mqtt_password                 "hunter.2"
#define mqtt_will_qos                 2
#define mqtt_will_retain              1
#define mqtt_will_message             "Offline"

// HX711 Pins
const int LOADCELL_DOUT_PIN = 2; // Remember these are ESP GPIO pins, they are not the physical pins on the board.
const int LOADCELL_SCK_PIN = 3;
int calibration_factor = 18905; // Defines calibration factor we'll use for calibrating.
