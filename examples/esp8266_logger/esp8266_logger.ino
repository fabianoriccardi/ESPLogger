#include "FS.h"
#include "Ticker.h"
#include "logger.h"

Logger loggg("/log/mylog.log",2);

// Event generator, this could be implemented in the main loop
Ticker somethingHappens;

// in seconds
float eventFrequency = 1.5;

// This is the variable event to log
int counter = 0;

void somethingHappening(){
  counter++;
  
  // counter is a multiple of 4, log it!
  if(counter%3==0){
    //Serial.println(String("Event happens: ") + counter);
    loggg.append(String("val:") + counter);
  }
  somethingHappens.attach(eventFrequency, somethingHappening);
}



void setup() {
  Serial.begin(115200);
  while(!Serial);

  Serial.println("Logger test booting.. ");

  Serial.print("Filesystem initialization... ");
  if(!SPIFFS.begin()){
    Serial.println("Error in starting file system");
  }else{
    Serial.println("Done!");
  }
  
  //loggg.setSizeLimit(100,false);
  loggg.setSizeLimitPerChunk(60);
  loggg.setFlusherCallback(senderHelp);
  somethingHappens.attach(eventFrequency, somethingHappening);
}

// in millisecond
int period = 30000;

unsigned int nextTime = 0;

// Pensando allo smartifier, possiamo dire che 
// il main loop è il controller
void loop() {
  if (millis()>nextTime){
    nextTime+=period;
    loggg.flush();
  }
}

void senderHelp(char* buffer, int n){
  int index=0;
      // Check if there is another string to print
      while(index<n && strlen(&buffer[index])>0){
        Serial.print("---");
        int bytePrinted=Serial.print(&buffer[index]);
        Serial.println("---");
        //Serial.println(String("Ho stampato:") + bytePrinted + "byte");
        // +1, the '\0' is processed
        index += bytePrinted+1;
      }
}
