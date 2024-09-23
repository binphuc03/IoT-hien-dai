#include <Arduino.h>
#include<WiFi.h>
#include<WiFiManager.h>
#include<WiFiClientSecure.h>
#include<PubSubClient.h>
#include<ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include<DHT.h>
#include<Wire.h>
#include<BH1750.h>
#include<TimeLib.h>
const char* mqtt_server ="oe9219e1.ala.eu-central-1.emqxsl.com";
const uint16_t mqtt_port = 8883;
const char *mqtt_username = "thinh";
const char *mqtt_password = "6wbF6MbDLF8fEFT";
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

const int low_threshold=40;
const int high_threshold= 70;

const uint8_t RELAY_PIN=33;
const int DHTPIN=2;
const int DHTTYPE=DHT11;
const int SOIL_MOISTURE_PIN=35;
int mode = 1,button=0;
const int water_mililit=700;
float humidity,temperature,soil_moisture,lux;
WiFiClientSecure espClient;
PubSubClient client(espClient);
String client_id;
String status_topic,sensor_topic;
String status_output,sensor_output;
JsonDocument status_doc,sensor_doc;
BH1750 lightMeter;
DHT dht(2,DHT11);
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 25200, 60000);
char Time[] = "TIME:00:00:00";
char Date[] = "00-00-2000";

char* plant="rau muong";
// 
void CollectData()
{
    humidity=dht.readHumidity();
    temperature=dht.readTemperature();
    lux=lightMeter.readLightLevel();
    int value=analogRead(SOIL_MOISTURE_PIN);
    soil_moisture=map(value,0,4095,100,0);
  //  if (!isnan(humidity) && !isnan(temperature) && !isnan(soil_moisture) && !isnan(lux)) 
 //   {
        sensor_doc["temperature"]=temperature;
        sensor_doc["humidity"]=humidity;
        sensor_doc["lux"]=lux;
        sensor_doc["soil_moisture"]=soil_moisture;
        serializeJson(sensor_doc,sensor_output);
        client.publish(sensor_topic.c_str(),sensor_output.c_str());
//    }
 //   else
 //   {
//        Serial.println("Failed to read data from sensor");
 // }
}
int TimetoWater()
{
  if(soil_moisture<low_threshold)
  return 1;
  else return 0;
}
void automaticWatering()
{
  CollectData();
    int time=TimetoWater();
    if(time>0)
        digitalWrite(RELAY_PIN,HIGH);
    else
        digitalWrite(RELAY_PIN,LOW);
 //   delay(2000);
}
void handle() {
   CollectData();
    int time=TimetoWater();
   bool check ;
   if(button==0) check = false;else check = true;
  if (check == true && time > 0) {
    digitalWrite(RELAY_PIN, HIGH);
  //  Firebase.RTDB.setBool(&firebaseData, path + "is_watered", true);
  }
  else {
    digitalWrite(RELAY_PIN, LOW);
  //  Firebase.RTDB.setBool(&firebaseData, path + "is_watered", false);
  }
  //delay(2000);
}
void setup() 
{
    Serial.begin(115200);
    WiFiManager wm;
    if(!wm.autoConnect("ESP32-WiFi-Manager","123456789"))
    {
        Serial.println("Failed to connect to wifi");
        ESP.restart();
    }
    Serial.println("Connected to wifi ");  
    espClient.setCACert(ca_cert);

    client.setServer(mqtt_server, mqtt_port);
      client.setCallback(callback);

    timeClient.begin();
    // neu chua ket noi thi thuc hien ben duoi
    while(!client.connected())
    {
        client_id="esp32-client-"+String(WiFi.macAddress());
        if(client.connect(client_id.c_str(),mqtt_username, mqtt_password))
        {
            Serial.println("Connected to mqtt broker");
            status_topic="client/"+String(client_id)+"/status";
            sensor_topic="client/"+String(client_id)+"/sensor";
            status_doc["id"]=client_id;
            status_doc["ip_address"]=WiFi.localIP().toString();
            status_doc["status"]="connected";
            serializeJson(status_doc,status_output);
            client.publish(status_topic.c_str(),status_output.c_str());
                  client.subscribe("water_button_state");
            Wire.begin();
            lightMeter.begin();
            dht.begin();
            Serial.println("DHT11 sensor is ready");
            pinMode(RELAY_PIN,OUTPUT);
        }
        else
        {
            Serial.println("Failed to connect to mqtt broker");
            Serial.println(client_id.c_str());
            delay(2000);
        }
    }
}

void loop() 
{
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
    timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();
    CollectData();
    if (mode == 1) automaticWatering();
    else handle();
    delay(5000);
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
      client_id="esp32-client-"+String(WiFi.macAddress());
    // Attempt to connect
if(client.connect(client_id.c_str(),mqtt_username, mqtt_password))
{      Serial.println("connected");
      client.subscribe("water_button_state");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.println((char)payload[i]);
  }
      if(topic=="water_button_state") button = payload[0];
}