#include <Ticker.h>
#include <logger_spiffs.h>

LoggerSPIFFS myLog("/log/mylog.log",1);

/** 
 * Event generation and management 
 */

// Event generator, this could be implemented in the main loop
Ticker somethingHappens;

// in seconds
float eventFrequency = 1.5;

// This is the variable event to log
int counter = 0;

void somethingHappening(){
  counter++;
  
  // counter is a multiple of 3, log it!
  if(counter%3==0){
    Serial.println(String("Oh, ->") + counter + "<- is just happend");
    myLog.append(String("val:") + counter);
  }
  somethingHappens.attach(eventFrequency, somethingHappening);
}

void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println("Basic log example");

  myLog.begin();  
  myLog.setFlusherCallback(senderHelp);
  somethingHappens.attach(eventFrequency, somethingHappening);
}

// in millisecond
int period = 30000;

unsigned int nextTime = 0;

// This loop is the logger controller, it decides when it's time to flush
// You can see a more elegant management in other examples. 
void loop() {
  if (millis()>nextTime){
    nextTime += period;
    myLog.flush();
  }
}

/**
 * Callback to support the flushing.
 * In this case the flushing is performed on the Serial interface, but you can easily 
 * replace this behaviour with a wireless connection or whatever function. 
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
