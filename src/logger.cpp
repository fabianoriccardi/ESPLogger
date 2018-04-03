#include "logger.h"

void Logger::setSizeLimit(int size, bool strict){
  sizeLimit=size;
  strictLimit=strict;
}

void Logger::setSizeLimitPerChunk(int size){
  sizeLimitPerChunk=size;
}

void Logger::setOneRecordPerChunk(bool one){
  oneRecordPerChunk=one;
}

void Logger::setFlusherCallback(bool (*callback)(char*, int)){
  flusher=callback;
}

bool Logger::append(String message, bool timestamp){
  unsigned int total=0;
  
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

  if(debugVerbosity > 1) Serial.println(String("Recording message: ___") + message + "___");
  
  File f=SPIFFS.open(filePath,"a");
  if(f){
    total += f.size();
    if (debugVerbosity>1) Serial.println(String(total) + "/" + sizeLimit + "bytes occupied");
    if(total>sizeLimit){
      f.close();
      if (debugVerbosity>0) Serial.println("You have reached the maximum file length, the record can't be stored. ");
      return false;
    }
    f.println(message);
    f.close();
    return true;
  }else{
    if (debugVerbosity>0) Serial.println("Opening log file error!");
  }
  return false;
}

void Logger::reset(){
  SPIFFS.remove(filePath);
}

void Logger::flush(){
  if(debugVerbosity>1) Serial.println("Flushing the log file...");
  
  // First step: fill the buffer with a chunk
  File f=SPIFFS.open(filePath,"r");
  if(f){
    String line;
    char* buffer = (char*) malloc(sizeLimitPerChunk);

    int chunkCount = 0;
    bool bufferFull = false;
    while(1){
      if(debugVerbosity > 1) Serial.println(String(":::::::::::::::::::::::::::") + chunkCount);
      chunkCount++;
      if(debugVerbosity > 1) Serial.println(":::::::::::::::First step: Chunk loading...");
      
      unsigned int nBuffer = 0;
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
        unsigned int len=line.length();
        if(len+nBuffer>sizeLimitPerChunk){
          if(debugVerbosity>1) Serial.println(String("Chunk buffer is almost full: ") + nBuffer + "/" + sizeLimitPerChunk + "byte, cannot store another message, it's time to send..");
          bufferFull=true;
        }else{
          if(debugVerbosity > 1) Serial.println(String("###") + line.c_str() + "###");
          strcpy(&buffer[nBuffer], line.c_str());
          // replace the '\r' with '\0'
          buffer[nBuffer+len-1]='\0';
          nBuffer += len;
        }
        
        if(oneRecordPerChunk) break;
      }
      
      if(nBuffer == 0){
        if(debugVerbosity > 1) Serial.println("No more data to send");
        break;
      }
  
      // Second step: send chunk
      if(debugVerbosity > 1) Serial.println(":::::::::::::::Second step: Chunk flushing...");
      bool res=flusher(buffer,nBuffer);
    }
    
    // Free the memory buffer
    free(buffer);
    f.close();
    SPIFFS.remove(filePath);
  }else{
     if (debugVerbosity>0) Serial.println("Opening log file error!");
  }
  
  if(debugVerbosity>1) Serial.println("End of flushing the log file!");
}


