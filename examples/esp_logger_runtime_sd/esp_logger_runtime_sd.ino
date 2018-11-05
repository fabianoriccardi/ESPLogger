#include <logger_sd.h>
#include <logger_routine.h>

// Required to call the File System init!
#include "SD.h"

#ifdef ESP8266 
const int csPin = D8;
#else
const int csPin = 16;
#endif

// in second
const int period = 30;

const String filepath = "/myLog.log";

LoggerSD myLog(filepath, 2);

// This class takes care of flushing the myLog file every period 
LoggerRoutine myLogRun(myLog, period);


/** 
 * Event generation and management 
 */

// Event generator, this could be implemented in the main loop
Ticker somethingHappens;

// in seconds
float eventFrequency = 1.5;

// This is the variable event to myLog
int counter = 0;

void somethingHappening(){
  counter++;
  
  // counter is a multiple of 4, myLog it!
  if(counter%3==0){
    Serial.println(String("Oh, ->") + counter + "<- is just happend");
    myLog.append(String("val:") + counter);
  }
  somethingHappens.attach(eventFrequency, somethingHappening);
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Log on SD Example");


  Serial.print("Initializing SD card...");
  if (!SD.begin(csPin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Effectively working only on ESP32
  myLog.begin(csPin);
  
  myLog.setSizeLimit(1000,false);
  myLog.setSizeLimitPerChunk(60);
  myLog.setFlusherCallback(senderHelp);
  somethingHappens.attach(eventFrequency, somethingHappening);
  
  myLogRun.begin(true);
}

void loop() {}

/**
 * Callback to support the flushing.
 * In this case the flushing is performed on the Serial interface, but you can easily 
 * replace this behaviour with a wireless connection or whatever function. 
 */
bool senderHelp(char* buffer, int n){
  static int failPacketCounter = 0;
  static int failPacket=random(3,5);

  // This part is the example about the management of sending failure.
  // The failure is "randomic"
  if(failPacketCounter==failPacket){
    failPacket=random(3,5);
    failPacketCounter = 0;
    Serial.println(String("Sending chunk failed. Setting the next send failure between: ") + failPacket + " chunks");
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
