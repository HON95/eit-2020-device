#include <TelenorNBIoT.h>
#include <Udp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>


SoftwareSerial ublox(10, 11);
TelenorNBIoT nbiot("mda.ee", 242, 01);
IPAddress remoteIP(172, 16, 15, 14);
int REMOTE_PORT = 1234;

unsigned long INTERVAL_MS = (unsigned long) 60 * 60 * 1000;
char* identity = "2d1042a3-2eea-43f4-a3d3-1f1992e6ad7f";

#define PH_PORT A3
#define TEMP_PORT 7

#define PH_OFFSET -1.03
#define PH_GRADIENT 4.183

OneWire oneWire_in(TEMP_PORT);
DallasTemperature temp_sensor(&oneWire_in);

char json[255];
float pH;
float temperature;
String msg;

//Buffers for parsing strings to chars
char tempBuf[10];
char phBuf[10];



void setup() {

  pinMode(PH_PORT, INPUT);

  //For debug only
  Serial.begin(9600);
  while (!Serial); 

  //Start temperature serial
  temp_sensor.begin();

  //Connect to module
  ublox.begin(9600);
  Serial.print("Connecting to NB-IoT module...\n");
  while (!nbiot.begin(ublox)) {
    Serial.println("Failed to connect. Retrying...");
    delay(1000);
  }

  Serial.println("Connection established!");
  Serial.print("IMSI: ");
  Serial.println(nbiot.imsi());
  Serial.print("IMEI: ");
  Serial.println(nbiot.imei());

  while (!nbiot.createSocket()) {
    Serial.print("Error creating socket. Error code: ");
    Serial.println(nbiot.errorCode(), DEC);
    delay(1000);
  }
  
}

int sort_cmp(const void *cmp1, const void *cmp2)
{
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  
  return a > b ? -1 : (a < b ? 1 : 0);
}

int aggregateSamples(int samples[]){
  qsort(samples, sizeof(samples)/sizeof(samples[0]), sizeof(samples[0]), sort_cmp);

  int sampleSum = 0;
  for(int i = 2; i < 8; i++){
    sampleSum += samples[i];
  }

  return sampleSum;
}

float measurePH(){

  int samples[10] = {};
  for(int i = 0; i < 10; i++){
    samples[i] = analogRead(PH_PORT);
    delay(5000);
  }

  int sampleSum = aggregateSamples(samples);

  return PH_OFFSET + (PH_GRADIENT * (float)sampleSum * 5.0) / (1024 * 6);
}

float measureTemperature(){
  temp_sensor.requestTemperatures();
  return temp_sensor.getTempCByIndex(0);
}

String compileJson(){
  dtostrf(pH, 2, 3, phBuf);
  dtostrf(temperature, 2, 3, tempBuf);
  sprintf(json, "{ \"device\" : \"%s\", \"ph\" : %s, \"temperature\" : %s }", identity, phBuf , tempBuf);
  
  return json;
}

bool sendMessage() {
  if (nbiot.isConnected()) {
    if (true == nbiot.sendString(remoteIP, REMOTE_PORT, msg)) {
      Serial.println("Successfully sent data");
      Serial.println(msg);
      return true;
    } 
    else {
      Serial.println("Failed sending data");
      return false;
    }     
  } 
  else {
    // Not connected yet. Wait 5 seconds before retrying.
    Serial.println("Connecting...");
    delay(5000);
    return false;
  }
}

void loop() {

  pH = measurePH();
  temperature = measureTemperature();

  msg = compileJson();
  while(!sendMessage());

  delay(INTERVAL_MS);
}
