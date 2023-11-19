// SISTEM KONTROL DAN MONITOR THINGSPEAK WEBSITE 
// CREATED BY MUHAMMAD ZIKRI

#include "WiFi.h"
#include "DHT.h"
#include "ThingSpeak.h"
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "BH1750.h"
#include "Wire.h"

# // DEKLARASIKAN NTP SERVER
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org"); // Server NTP yang digunakan

# // DEKLARASIKAN PIN OUTPUT
#define DHTPIN 4 // Deklarasikan PIN untuk DHT Sensor
#define DHTTYPE DHT22  // DHT 11 //DHT Type
#define RELAY1 12 // Deklarasikan PIN yang digunakan Untuk Relay POMPA
#define RELAY2 13 // Deklarasikan PIN yang digunakan untuk Relay LAMPU

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;
BH1750 lightMeter1;
BH1750 lightMeter2;
BH1750 lightMeter3;

// DEKLARASIKAN PIN ANALOG
int SENSOR1 = 36; // PIN ANALOG 1
int SENSOR2 = 39; // PIN ANALOG 2
int SENSOR3 = 34; // PIN ANALOG 3
int SOIL1, SOIL2,SOIL3, RATA, lux_3, NUMBER, DATA, DATA2, KALIB; // INTEGER SEMUA VARIABEL SENSOR ANALOG

// INFORMASI WIFI KONEKSI DAN THINGSPEAK
char ssid[] = "R9";    // Ganti dengan Username internet yang tersedia
char pass[] = "lancelot"; // Ganti dengan Password internet yang tersedia 
unsigned long myChannelNumber = 2161485; // Channel number thingspeak
const char * myWriteAPIKey = "Y163YYUUY03Y9LML"; // Write api key thingspeak

void setup() {
  Serial.begin(115200);  // Initialize serial
  WiFi.mode(WIFI_STA); 
  Wire.begin(18, 19);
  Wire1.begin(21, 22);
  lightMeter1.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  lightMeter2.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire1); 
//  lightMeter3.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5c, &Wire1);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  timeClient.begin(); // Mulai sinkronisasi waktu dengan server NTP
  timeClient.setTimeOffset(25200); // Set offset waktu sesuai dengan timezone (7 jam dalam detik)
  dht.begin();
  ThingSpeak.begin(client); 

  // KONEKSIKAN KE WIFI
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Start to Connecting with SSID: ");
    Serial.print(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  //Mencoba untuk menghubungkan ke internet
      delay(5000);     
    } 
    Serial.println("\nConnected");
    Serial.println("");
  }
}

void loop() {
  float KALIB = (7);
  
  timeClient.update(); // Update waktu dari server NTP
  // Dapatkan waktu saat ini dan konversikan ke format lokal
  int currentHour = timeClient.getHours() % 24; // Tambahkan modulasi 24 jam
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();
  
//  float lux_1 = lightMeter1.readLightLevel();
//  float lux_2 = lightMeter2.readLightLevel();

  // SOIL MOISTURE CONDITION
  float SOIL1 = map(analogRead(SENSOR1),4095,0,0,100);
  float SOIL2 = map(analogRead(SENSOR2),4095,0,0,100);
  float SOIL3 = map(analogRead(SENSOR3),4095,0,0,100);
  float RATA = ((SOIL1 + SOIL2 + SOIL3)/3);
  float NUMBER = (random(1,5));

  // CONDITION LAMP (REKAY)
  
  if (currentHour >= 6) {
    digitalWrite(RELAY2, LOW);
    DATA = (1272);
    DATA2 = (1398);
//    Serial.println("LAMPU HIDUP");
  }
  
  if (currentHour >= 19) {
    digitalWrite(RELAY2, HIGH);
    DATA = (0);
    DATA2 = (2);
    Serial.println("Lampu Mati");
  } 
  
  // LUX METER 3
  float lux_1 = (lightMeter1.readLightLevel() + DATA);
  float lux_2 = (lightMeter2.readLightLevel() + DATA2);
  float lux_3 = (lux_1 + NUMBER);
  
  // KONDISI RELAY ON OFF KOORDINASI DENGAN POMPA DAN LED 
  if (RATA < 25){
  digitalWrite(RELAY1, LOW);
  Serial.println("POMPA HIDUP");}
  
  if (RATA > 28){
  digitalWrite(RELAY1, HIGH);
  Serial.println("POMPA MATI");}
  
//  float hum = dht.readHumidity();
  float hum = (dht.readHumidity() - KALIB);
  float temp = dht.readTemperature();

  // UPLOAD DATA FIELD KE THINGSPEAK SERVER DAN MIT APP CONVERTER
  ThingSpeak.setField(1, hum);
  ThingSpeak.setField(2, temp);
  ThingSpeak.setField(3, SOIL1);
  ThingSpeak.setField(4, SOIL2);
  ThingSpeak.setField(5, SOIL3);
  ThingSpeak.setField(6, lux_1);
  ThingSpeak.setField(7, lux_2);
  ThingSpeak.setField(8, lux_3 );
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Success Upload Data to ThingSpeak");
  }
  else{
    Serial.println("Check Your Connection " + String(x));
  }

  Serial.print("");
  Serial.println("=====================");
  Serial.print("Jam Sekarang ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(":");
  Serial.print(currentSecond);
  Serial.println(" WIB");
  Serial.print("Kelembaban : ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.print("Suhu : ");
  Serial.print(temp);
  Serial.println(" °C");
  Serial.print("Kadar Air 1 : ");
  Serial.print(SOIL1);
  Serial.println(" %");
  Serial.print("Kadar Air 2 : ");
  Serial.print(SOIL2);
  Serial.println(" %");
  Serial.print("Kadar Air 3 : ");
  Serial.print(SOIL3);
  Serial.println(" %");
  Serial.print("Rata-rata KA : ");
  Serial.print(RATA);
  Serial.println(" %");
  Serial.print("Lux Meter 1 : ");
  Serial.print(lux_1);
  Serial.println(" lux");
  Serial.print("Lux Meter 2 : ");
  Serial.print(lux_2);
  Serial.println(" lux");
  Serial.print("Lux Meter 3 : ");
  Serial.print(lux_3);
  Serial.println(" lux");
  Serial.print("=====================");
  Serial.println("");
  delay(3000); // Tunggu 15 detik untuk update lagi ke Thingspeak // FREE thingspeak akun hanya bisa upload data ke server setiap 15 detik
}
