//Created by Geeoon Chung
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(192, 168, 0, 42);

unsigned int localPort = 1337;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

EthernetUDP Udp;

int throttle = 0;
Servo talon1;
Servo steer;
int steerVal = 0;
int driveVal = 0;
bool enabled = false;
bool shutOff = false;
int blink = 0;
/*
LIST OF PINS USED:
D6, for talon1
D8, for status LED
D9, for turning servo
*/


void setup(void)
{
  talon1.attach(6);
  talon1.writeMicroseconds(1500);
  steer.attach(9);
  steer.write(90);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  Udp.begin(localPort);
  digitalWrite(8, HIGH);
  Serial.println("Ethernet shield was found.");
}



void loop(void)
{
  if (shutOff == false) {
    int packetSize = Udp.parsePacket();
    if (packetSize) { //All data is sent in the size of one byte 
      Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
      uint8_t recieved = packetBuffer[0];
      if (recieved <= 100) {
        throttle = recieved;
      } else if (recieved > 150) {
        throttle = -256 + recieved;
      } else if (recieved == 101) { //enable
        enabled = true;
      } else if (recieved == 102) { //disable
        enabled = false;
      } else if (recieved == 105) { //shut down
        enabled = false;
        shutOff = true;
      } else if (recieved >= 110) { //110-150
        steerVal = (recieved - 112) * 5;
        if (steerVal < 0) {
          steerVal = 0;
        } else if (steerVal > 180){
          steerVal = 180;
        }
      }
      Serial.print("recieved: ");
      Serial.println(recieved);
    }
    if (enabled == true) {
      if (blink == 50) {
        digitalWrite(8, !digitalRead(8));
        blink = 0;
      }
      blink++;
      driveVal = throttle * 5 + 1500;
      talon1.writeMicroseconds(driveVal);//sends updated value to talon
      steer.write(steerVal); //0-180
    } else {
      digitalWrite(8, HIGH);
      talon1.writeMicroseconds(1500);
    }
  } else {
    digitalWrite(8, LOW);
    talon1.detach();
    steer.detach();
    
  }
  delay(10);//delay cause it's there; cant remember why though; not gonna mess with it.
}
