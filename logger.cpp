#include "logger.h"

void Logger::setSizeLimit(int size, bool strict){
  sizeLimit=size;
  strictLimit=strict;
}

void Logger::setSizeLimitPerPacket(int size){
  sizeLimitPerPacket=size;
}

void Logger::append(String message, bool timestamp){
  int total=0;
  
  if(timestamp){
    int now=millis();
    String nowString(now);
    message = nowString + " " + message;
  }
    
  // if strict, calculate the next file size
  // (including the line to be recorded)
  if(strictLimit){  
    total = message.length();
    // +2 because 2 bytes are required at the end of each line in this FS implementation
    total += 2;
  }

  if(debugVerbosity) Serial.println(String("Message to record: ---") + message + "---");
  
  File f=SPIFFS.open(filePath,"a");
  if(f!=NULL){
    total += f.size();
    if (debugVerbosity>1) Serial.println(String(total) + "/" + sizeLimit + "bytes occupied");
    if(total>sizeLimit){
      f.close();
      if (debugVerbosity>0) Serial.println("You have reached the maximum file length, the record can't be stored. ");
      return ;
    }
    f.println(message);
    f.close();
  }else{
    if (debugVerbosity>0) Serial.println("Opening log file error!");
  }
}

void Logger::reset(){
  SPIFFS.remove(filePath);
}

void Logger::sendAll(){
  if(debugVerbosity>1) Serial.println("Flushing the log file..");
  File f=SPIFFS.open(filePath,"r");
  String line;
  while(f.available()){
    line=f.readStringUntil('\n');
    if(line!=""||line!="\n"){
      Serial.println(line);
    }
  }
  f.close();
  SPIFFS.remove(filePath);
  if(debugVerbosity>1) Serial.println("End of flushing the log file!");
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
void Logger::sendAll2(){
  if(debugVerbosity>1) Serial.println("Flushing the log file..");
  
  // First step: fill the buffer with a chunk
  File f=SPIFFS.open(filePath,"r");
  if(f!=NULL){
    String line;
    char* buffer = (char*) malloc(sizeLimitPerPacket);

    int chunkCount = 0;
    bool bufferFull = false;
    while(1){
      Serial.println(String(":::::::::::::::::::::::::::") + chunkCount);
      chunkCount++;
      Serial.println(":::::::::::::::First step");
      
      int nBuffer = 0;
      bool doRead = true;
      if(bufferFull){
        // This means that we have a "pending" line, no need to read from file
        doRead = false;  
      }
      bufferFull = false;
      while((f.available() || !doRead) && !bufferFull){
        // '\n' is not included in the returned string, but the last char is '\r'
        if(doRead){
          line=f.readStringUntil('\n');
        }else{
          doRead=true;
        }
        
        // l contains the number of byte required by a line (we have to keep into account the '\0')
        // In this case, +1 isn't needed because the _line_ contains the useless '\r'
        int len=line.length();
        if(len+nBuffer>sizeLimitPerPacket){
          Serial.println("Buffer Full");
          bufferFull=true;
        }else{
          Serial.println(String("Line length: ") + len + " ###" + line.c_str() + "###");
          strcpy(&buffer[nBuffer], line.c_str());
          // replace the '\r' with '\0'
          buffer[nBuffer+len-1]='\0';
          nBuffer += len;
        }
      }
      
      if(nBuffer == 0){
        Serial.println("No more data to send");
        break;
      }
  
      // Second step: send chunk
      Serial.println(":::::::::::::::Second step");
      senderHelp(buffer,nBuffer);
    }
    
    // Free the memory buffer
    free(buffer);
    f.close();
    SPIFFS.remove(filePath);
  }
  
  
  if(debugVerbosity>1) Serial.println("End of flushing the log file!");
}


