#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5,15);  // No more use of pin 13 to avoid problem to use the microphone


const char* ssid = "MSI 8588";
const char* password = "Hello123";
const char* mqtt_server = "192.168.137.1";

String dataString = "";
String subdataString = "";
String orgdataString = "";

float lightIntensity = 0.0; // Lichtintensiteit
String temp1;            // Temperature
int temp = 0.0;          // Temperature
String hum1;             // Humidity
int humid = 0.0;         // Humidity
int co2 = 0;             // CO2
int tvoc = 0;            // tVOC
int pr = 0;              // Druk

DynamicJsonDocument doc(1024);
char json_data_out[1024];

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void get_uart_data() {
  // Read data from UART bus
  while (mySerial.available() > 0) {
    dataString += char(mySerial.read());
  }

  if (dataString.length() >= 44) {  
    orgdataString = dataString;

    Serial.print("Raw UART data: ");  // Print the exact data we get from the uart connection with the stm to see if we get something usefull 
    Serial.println(dataString);

    for (int ctr = 0; ctr < 6; ctr++) {  // Split incoming data in different parts (1 part = 1 sensor)
      int startIdx = dataString.indexOf(':'); // Data in is like T:20.5,H:10.3, ....... so split on every : for a new value, end at a , 
      int stopIdx = dataString.indexOf(',');

      if (stopIdx == -1) {
        stopIdx = dataString.length();
      }
      
      subdataString = dataString.substring(startIdx + 1, stopIdx);  // make a substring with the startIdx and stopIdx as borders
      dataString = dataString.substring(stopIdx + 1);
      
      if (ctr == 0) {
        lightIntensity = subdataString.toFloat();
        Serial.println("0: Light = " + String(lightIntensity) + " lux");
      } else if (ctr == 1) {
        temp1 = subdataString; //.toInt();
        Serial.println("1: Temp = " + temp1 + "˚C");
      } else if (ctr == 2) {
        hum1 = subdataString;
        Serial.println("2: RH = " + hum1 + "%");
      } else if (ctr == 3) {
        co2 = subdataString.toInt();
        Serial.println("3: CO2 = " + String(co2) + " ppm");
      } else if(ctr == 4) {
        tvoc = subdataString.toInt();
        Serial.println("4: tVOC = " + String(tvoc) + " ppb");
      } else  {
        pr = subdataString.toInt();
        Serial.println("5: Pressure = " + String(pr) + " Pa");
      }  
      }
    }
    dataString = "";
  }


void create_json() {
  doc["light"] = lightIntensity;
  doc["temperature"] = temp1;
  doc["humidity"] = hum1;
  doc["co2"] = co2;
  doc["tvoc"] = tvoc;
  doc["pr"] = pr;
  doc["noise"] = -1;
  doc["lokaal"] = "E116";
  doc["string"] = orgdataString;
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  mySerial.begin(115200); 
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  get_uart_data();
  create_json();

  serializeJson(doc, json_data_out);
  client.publish("sensor", json_data_out);
  Serial.println(json_data_out);

  delay(1000);
}