#include <logger_spiffs.h>
#include <logger_routine.h>

// flush period, in second
const int period = 30;
// Path where the log is placed
const String filepath = "/log/mylog.log";

LoggerSPIFFS myLog(filepath);

// This class takes care of flushing the log file every "period"
LoggerRoutine logRun(myLog, period);

// Event generator, this could be implemented in the main loop
Ticker somethingHappens;

// Event generation period, in seconds
float eventPeriod = 1.5;

// Variable to be logged
int counter = 0;

void somethingHappening(){
  counter++;
  
  // counter is a multiple of 3, log it!
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
  Serial.println("ESP Logger - Log on internal flash memory (with logger routine)");
  
  myLog.begin();
  myLog.setSizeLimit(1000);
  myLog.setSizeLimitPerChunk(60);
  myLog.setFlusherCallback(senderHelp);
  
  Serial.println("Starting to log...");
  somethingHappens.attach(eventPeriod, somethingHappening);
  logRun.begin(true);
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
