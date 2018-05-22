#include "logger_sd.h"
#include "SD.h"

bool LoggerSD::append(String message, bool timestamp){
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

  if(message.length()+1>sizeLimitPerChunk){
    if (debugVerbosity>0) Serial.println("@@@@ FATAL ERROR: This message is too large, it can't be neither stored nor sent due to limitation on chunk size, please change it before continue!!!");
    return false;
  }
  
  // Trick to be sure that an empty file exist, and it's dimension is 0 (BUG in esp32)
  if(!SD.exists(filePath)){
    File file=SD.open(filePath,FILE_WRITE);
    if(file){
      file.close();
    }
  }

#ifdef ESP32
  // In ESP32 I can use to usual notation "a" for append
  File f=SD.open(filePath,"a");
#elif ESP8266
  // Note the esp8266 interprets this constant as an apped, i.e. O_TRUNC flag is not set
  File f=SD.open(filePath, FILE_WRITE);
#endif
  if(f){
    total += f.size();
    if (debugVerbosity>1) Serial.println(String(total) + "/" + sizeLimit + "bytes occupied");
    if (debugVerbosity>1) Serial.println(String("Recording message: ___") + message + "___");
    if(total>sizeLimit){
      if(message.length()+2>sizeLimit){
      	if (debugVerbosity>0) Serial.println("@@@@ FATAL ERROR: This message is too large, it can't be stored nor sent due to limitation on file size, please change it before continue!!!");
      }else{
      	if (debugVerbosity>0) Serial.println("You have reached the maximum file length, the record can't be stored. Please flush the log.");
      }
      f.close();
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

void LoggerSD::reset(){
  if (debugVerbosity>1) Serial.println("Resetting the log file...");
  SD.remove(filePath);
}

static void saveChunk(File& file, char* buffer, int nBuffer){
  int bufferIndex=0;
  // Check if there is another string to print
  while(bufferIndex<nBuffer && strlen(&buffer[bufferIndex])>0){
      int bytePrinted = file.println(&buffer[bufferIndex]);
      // -2, the '\r' and '\n' are printed automatically in the println
      // +1, the '\0' in the buffer is automatically processed but not printed
    bufferIndex += bytePrinted - 2 + 1;
  }
}

static void saveRemainings(File& destination, File& source){
  while(source.available()>2){
    String s=source.readStringUntil('\n');
    // first param included, second paramenter excluded
    // this should remove '\r'
    s=s.substring(0,s.length()-1);
    destination.println(s);
  }
}

#ifdef ESP8266
static bool copyFile(String source, String destination){
  File s=SD.open(source,FILE_READ);
  File d=SD.open(destination, FILE_WRITE | O_TRUNC);
  if(!s){
    if(d){
      d.close();
    }
    return false;
  }
  if(!d){
    if(s){
      s.close();
    }
    return false;
  }

  byte buf[512];
  while(s.available()){
    int n=s.read(buf,512);
    d.write(buf,n);
  }
  s.close();
  d.close();
  return true;
}
#endif

void LoggerSD::flush(){
  if(debugVerbosity>1) Serial.println("Flushing the log file...");
  
  if(!SD.exists(filePath)){
    if (debugVerbosity > 1) Serial.println("File doesn't exist, nothing to flush..");
    return;
  }
  
  File f=SD.open(filePath,FILE_READ);
  if(f){
    bool successFlush=true;
    String line;
    char* buffer = (char*) malloc(sizeLimitPerChunk);

    
    bool bufferFull = false;
    int chunkCount;
    unsigned int nBuffer;
    for(chunkCount = 0;;chunkCount++){
      if(debugVerbosity > 1) Serial.println(String(":::::::::::::::::::::::::::") + chunkCount);
      
      // First step: fill the buffer with a chunk of data
      if(debugVerbosity > 1) Serial.println(":::::::::::::::First step: Chunk loading...");
      
      nBuffer = 0;
      bool doRead = true;
      if(bufferFull){
        // This means that we have a "pending" line, no need to read from file
        doRead = false;  
      }
      bufferFull = false;
      while((f.available() || !doRead) && !bufferFull){
        // '\n' is not included in the returned string, but the last char '\r' is
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
          if(len>sizeLimitPerChunk){
          	if (debugVerbosity>0) Serial.println(String("@@@@ FATAL ERROR: This message is too large (") + len + "/" + sizeLimitPerChunk + "), it can't be store in the chunk, please increase it's size") ;
          } 
          bufferFull=true;
        }else{
          if(debugVerbosity > 1) Serial.print(String("###") + line.c_str() + "###");
          if(debugVerbosity > 1) Serial.println(String(" Line length: ") + line.length() + "");
          // remove the last char, that is '\r'
          line=line.substring(0,line.length()-1);
          strcpy(&buffer[nBuffer], line.c_str());
          
          //len includes '\0'
          nBuffer += len;
        }
        
        if(oneRecordPerChunk) break;
      } // END OF WHILE - FILLING THE CHUNK
      
      if(nBuffer == 0){
        if(debugVerbosity > 1) Serial.println("No more data to send");
        break;
      }
  
      // Second step: send chunk
      if(debugVerbosity > 1) Serial.println(":::::::::::::::Second step: Chunk flushing...");
      successFlush=flusher(buffer,nBuffer);
      if(!successFlush) break;

    } // END OF FOR - ITERATING OVER THE CHUNKS

    if(!successFlush){
      if(chunkCount > 0){
        if(debugVerbosity > 1) Serial.println("Partial unsuccessful sending!");
        // I have to discard the successfully sent log, and save the remainings.
        // NOTE: in some FS, the filename il limited to 8.3 charcters
        String tempFilePath = "/templog.tmp";
#ifdef ESP32        
        File tempFile=SD.open(tempFilePath,"w");
#elif ESP8266
        File tempFile=SD.open(tempFilePath, FILE_WRITE|O_TRUNC);
#endif
        if(f){
          saveChunk(tempFile,buffer,nBuffer);
          saveRemainings(tempFile, f);
          tempFile.close();
          f.close();
          // Riordino i file
          if(SD.remove(filePath)){
            if (debugVerbosity>1) Serial.println("The old file is deleted!");
 #ifdef ESP32
            if(SD.rename(tempFilePath, filePath)){
 #elif ESP8266
            // Simulating the file rename operation: 
            // 1. copy the old file into the new,
            // 2. then delete the old one
            // NOTE: this approach is heavily inefficient
            bool success=false;
            if(copyFile(tempFilePath,filePath)){
              if(SD.remove(tempFilePath)){
                success=true;
              }else{
                if (debugVerbosity>1) Serial.println("Delete failed");
              }
            }else{
              if (debugVerbosity>1) Serial.println("Something went wrong during the copy!");
            }

            if(success){
 #endif
              if (debugVerbosity>1) Serial.println("The temp file is moved!");
            
            }else{
              if (debugVerbosity>0) Serial.println("The temp file wasn't moved");
            }
          }else{
            if (debugVerbosity>0) Serial.println("The temp file is NOT deleted!");
          }
          return;
        }else{
          if (debugVerbosity>0) Serial.println("Writing temp log file error!");
        }
      }else{
        // Nothing was sent, so I can close the file and exit from this function
        if(debugVerbosity > 1) Serial.println("Unsuccessful sending! Nothing is flushed..");
      }
    }else{
      f.close();
      SD.remove(filePath);
    }

    // Free the memory buffer
    free(buffer);
  }else{
    if (debugVerbosity>0) Serial.println("Opening log file error!");
  }
  if(debugVerbosity>1) Serial.println("End of flushing the log file!");
}

LoggerSD::LoggerSD(String file, int debugVerbosity): Logger(file,debugVerbosity){
};

bool LoggerSD::begin(){
#ifdef ESP8266
  Serial.println("On ESP8266 this is not implemented, because multiple init are going to fail..");
  return false;
#endif  
  if(!SD.begin(16)){
    Serial.println("Card Mount Failed");
    return false;
  }
  return true;
}

bool LoggerSD::begin(int csPin){
#ifdef ESP8266
  Serial.println("On ESP8266 this is not implemented, because multiple init are going to fail..");
  return false;
#endif  
  if(!SD.begin(csPin)){
    Serial.println("Card Mount Failed");
    return false;
  }
  return true;
}