/**
 * Log on flash memory an event every 1.5 seconds. Every 30 seconds,
 * the log file is flushed over serial port.
 *
 * NOTE: the first time you run this sketch or when changing the file system
 *       layout, explicit formatting is recommended.
 */
#include <Ticker.h>
#include <logger_spiffs.h>

// Specify the path where the log is placed
LoggerSPIFFS myLog("/log/mylog.log");

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
    String record = String("val:") + counter;
    myLog.append(record.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println();
  Serial.println("ESP Logger - Log on internal flash memory");

  myLog.begin();
  myLog.setFlusherCallback(senderHelp);

  Serial.println("Starting to log...");
  somethingHappens.attach(eventPeriod, somethingHappening);
}

// flush period, in millisecond
int period = 30000;

unsigned int nextTime = 0;


void loop() {
  // This loop is the logger controller, it decides
  // when it's time to flush. In other examples I will use Ticker library. 
  if (millis()>nextTime){
    nextTime += period;
    myLog.flush();
  }
}

/**
 * Flush a chuck of logged records. To exemplify, the records are
 * flushed on Serial, but you are free to modify it to send data
 * over the network.
 */
bool senderHelp(char* buffer, int n){
  int index=0;
  // Check if there is another string to print
  while(index<n && strlen(&buffer[index])>0){
    Serial.print("---");
    int bytePrinted=Serial.print(&buffer[index]);
    Serial.println("---");
    // +1, the '\0' is processed
    index += bytePrinted+1;
  }
  return true;
}
