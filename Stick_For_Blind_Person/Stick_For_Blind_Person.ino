#include <SoftwareSerial.h>
#include <TinyGPS++.h>
/**Notes 
  connect RX to TX and vice versa
**/

/*Pin Configuration */
//====================================
const byte PIN_GPS_rx = 8;
const byte PIN_GPS_tx = 9;

const byte PIN_SIM_rx = 10;
const byte PIN_SIM_tx = 11;

const byte PIN_USS_trig = 4;
const byte PIN_USS_echo = 3;
const byte PIN_buzzer = 12;
const byte PIN_led = 13;
//====================================
//Play with this value to ajust the sensor
int USS_safetyDistance = 20;
//====================================
// global scoped variables
long USS_duration;
int USS_distance;
float PrevDist[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

SoftwareSerial SIM800L(PIN_SIM_rx, PIN_SIM_tx); // RX, TX pins of SIM800L module
SoftwareSerial GPS(PIN_GPS_rx, PIN_GPS_tx); // RX, TX pins of NEO-6M GPS module
TinyGPSPlus gps; // Create a TinyGPSPlus object

float calculateAproxDist(){
  float sum = 0;
  for(int i = 0;i<15;i++){
    sum = sum+ PrevDist[i];    
  }
  return sum/15;
}

int globalIter = 0;

void setup() {
  Serial.begin(9600);
  SIM800L.begin(9600);
  GPS.begin(9600);
  delay(1000);

  // Set SIM800L to SMS mode
  SIM800L.println("AT+CMGF=1");
  delay(100);
  SIM800L.println("AT+CNMI=1,2,0,0,0");
  delay(100);

  Serial.println("SIM800L and NEO-6M Initialized");

  //Setting UltraSonic Sensor
  pinMode(PIN_USS_trig, OUTPUT); // Sets the PIN_USS_trig as an Output
  pinMode(PIN_USS_echo, INPUT); // Sets the PIN_USS_echo as an Input
  pinMode(PIN_buzzer, OUTPUT);
  pinMode(PIN_led, OUTPUT);

}

void loop() {
  if(SIM800L.available()) {
    String message = SIM800L.readString();
    Serial.print("Received message: ");
    Serial.println(message);

    // If the received message contains "Hello", reply with GPS location
    if (message.indexOf("Hello") >= 0) {
      // Extract the phone number of the sender from the received message
      String sender = message.substring(message.indexOf("+"), message.indexOf("\",\""));
      Serial.print("Sender's phone number: ");
      Serial.println(sender);

      // Wait for GPS fix
      while (gps.location.isValid() == false) {
        while (GPS.available()) {
          gps.encode(GPS.read());
        }
      }
      // Get latitude and longitude from GPS
      String latitude = String(gps.location.lat(), 6);
      String longitude = String(gps.location.lng(), 6);

      // Send a reply message to the sender with the GPS location
      SIM800L.println("AT+CMGS=\"" + sender + "\"");
      delay(100);
      SIM800L.print("Latitude: ");
      SIM800L.println(latitude);
      delay(100);
      SIM800L.print("Longitude: ");
      SIM800L.println(longitude);
      delay(100);
      SIM800L.println((char)26);
      delay(100);
    }
  }
  ObjectDetection();
}

void ObjectDetection(){

  // Clears the trigPin
  digitalWrite(PIN_USS_trig, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(PIN_USS_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_USS_trig, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  USS_duration = pulseIn(PIN_USS_echo, HIGH);
  // Calculating the distance
  USS_distance = USS_duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(USS_distance);
  Serial.print("  ");
  PrevDist[(globalIter++)%15] = USS_distance;
  float mydist = calculateAproxDist();
  Serial.println(mydist);
  if (mydist <= USS_safetyDistance){
    digitalWrite(PIN_buzzer, HIGH);
    digitalWrite(PIN_led, HIGH);
  }
  else{
    digitalWrite(PIN_buzzer, LOW);
    digitalWrite(PIN_led, LOW);
  }
}

