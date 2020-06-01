#include <logger_sd.h>
#include <logger_routine.h>

// SD.h is required to init the file system directly in the setup
// othewise you can call the method begin(..) (only for ESP32)
#include <SD.h>

// Specify CS (chip select) pin
const int csPin = 5;

// flush period, in seconds
const int period = 30;
// Path where the log is placed
const String filepath = "/myLog.log";

LoggerSD myLog(filepath);

// This class takes care to flush "myLog" object every "period" 
LoggerRoutine myLogRun(myLog, period);

// Event generator, this could be implemented in the main loop
Ticker somethingHappens;

// Event generation period, in seconds
float eventPeriod = 1.5;

// Variable to be logged
int counter = 0;

void somethingHappening(){
  counter++;
  
  // counter is a multiple of 3, myLog it!
  if(counter%3==0){
    Serial.println(String("Hey, event ->") + counter + "<- is just happened");
    myLog.append(String("val:") + counter);
    Serial.println(String("Now the log takes ") + myLog.getActualSize() + "/" + myLog.getSizeLimit());
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("ESP Logger - Log on SD  (with logger routine)");

  // Check if the microSD is connected
  Serial.print("Initializing SD card... ");
  if (!SD.begin(csPin)) {
    Serial.println("Failed!");
    while(1) delay(100);
  }
  Serial.println("Done!");

  // Effectively working only on ESP32
  myLog.begin(csPin);
  myLog.setSizeLimit(1000);
  myLog.setSizeLimitPerChunk(60);
  myLog.setFlusherCallback(senderHelp);
  
  Serial.println("Starting to log...");
  somethingHappens.attach(eventPeriod, somethingHappening);
  myLogRun.begin(true);
}

void loop() {}

/**
 * Flush a chuck of logged records. To exemplify, the records are
 * flushed on Serial, but you are free to modify it to send data
 * over the network.
 *
 * NOTE: This example simulates a channel that sometimes may fail.
 */
bool senderHelp(char* buffer, int n){
  static int failPacketCounter = 0;
  static int failPacket=random(3,5);

  // This part is the example about the management of sending failure.
  // The failure is "randomic"
  if(failPacketCounter==failPacket){
    failPacket=random(3,5);
    failPacketCounter = 0;
    Serial.println(String("Sending chunk failed. The next sending failure is simulated in: ") + failPacket + " chunks");
    return false;
  }else{
    int index=0;
    // Check if there is another string to print
    while(index<n && strlen(&buffer[index])>0){
      Serial.print("---");
      int bytePrinted=Serial.print(&buffer[index]);
      Serial.println("---");
      // +1, the '\0' is processed
      index += bytePrinted+1;
    }
    
    failPacketCounter++;
    return true;
  }
}
