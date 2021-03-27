/***************************************************************************
 *   Copyright (C) 2018-2020 - Fabiano Riccardi                            *
 *                                                                         *
 *   This file is part of ESP Logger library.                              *
 *                                                                         *
 *   Dimmable Light for Arduino is free software; you can redistribute     *
 *   it and/or modify it under the terms of the GNU General Public         *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/
#include "logger_sd.h"
#include <SD.h>

bool LoggerSD::append(const char* record, bool timestamp){
  // Max 10 digits in an integer
  char timestampString[11] = {0};
  if(timestamp){
    unsigned int now=millis();
    itoa(now, timestampString, 10);
  }

  unsigned int recordLength=0;
  recordLength = strlen(record);
  // +2 because 2 bytes are required at the end of each line in this FS implementation
  recordLength += 2;

  if(timestamp){
    // Consider a blank space between the timestamp and the record
    recordLength += strlen(timestampString) + 1;
  }

  if (debugVerbosity>=DebugLevel::INFO){
    Serial.print("[ESP LOGGER] Recording message: ___");
    if(timestamp){
      Serial.print(timestampString);
      Serial.print(" ");
    }
    Serial.print(record);
    Serial.println("___");
    Serial.print("[ESP LOGGER] Record length:");
    Serial.println(recordLength);
  }

  // +1 because the terminating char of a chunk
  if(recordLength+1>sizeLimitPerChunk){
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] @@@@ FATAL ERROR: This message is too large, it can't be sent because the limitation on chunk size, please change it before continue!!!");
    return false;
  }

  if(recordLength>sizeLimit){
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] @@@@ FATAL ERROR: This message is too large, it can't be stored because the limitation on file size, please change it before continue!!!");
    return false;
  }

  // Trick to be sure that an empty file exist, and it's dimension is 0 (BUG in esp32)
  if(!SD.exists(filePath)){
    File file=SD.open(filePath, FILE_WRITE);
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
  if(!f){
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] Opening log file error!");
    return false;
  }

  unsigned int totalFileLength = f.size();
  if (debugVerbosity>=DebugLevel::INFO) Serial.println(String("[ESP LOGGER] ") + f.size() + "/" + sizeLimit + "bytes are already occupied");
  // if strict, calculate the file size comprising the actual record 
  if(strictLimit){
    totalFileLength += recordLength;
  }

  if(totalFileLength>sizeLimit){
    if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] You have reached the maximum file length, the record can't be stored. Please flush the log.");
    f.close();
    return false;
  }

  if(timestamp){
    f.print(timestampString);
    f.print(" ");
  }
  f.println(record);
  f.close();

  if (debugVerbosity>=DebugLevel::INFO) Serial.println("[ESP LOGGER] Record properly logged"); 
  return true;
}

void LoggerSD::reset(){
  if (debugVerbosity>=DebugLevel::WARN) Serial.print("[ESP LOGGER] Resetting the log file... ");
  SD.remove(filePath);
  if (debugVerbosity>=DebugLevel::WARN) Serial.println("Done!");
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
  // Since FILE_WRITE is just like an append, I need to delete the file before opening
  SD.remove(destination);
  File d=SD.open(destination, FILE_WRITE);
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

bool LoggerSD::flush(){
  if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Flushing the log file...");
  
  if(!SD.exists(filePath)){
    if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] File doesn't exist, nothing to flush..");
    return true;
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
      if(debugVerbosity>=DebugLevel::WARN) Serial.println(String("[ESP LOGGER] :::::::::::::::::::::::::::") + chunkCount);
      
      // First step: fill the buffer with a chunk of data
      if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] :::::::::::::::First step: Chunk loading...");
      
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
          if(debugVerbosity>=DebugLevel::WARN) Serial.println(String("[ESP LOGGER] Chunk buffer is almost full: ") + nBuffer + "/" + sizeLimitPerChunk + "byte, cannot store another message, it's time to send..");
          if(len>sizeLimitPerChunk){
            if (debugVerbosity>=DebugLevel::ERROR) Serial.println(String("[ESP LOGGER] @@@@ FATAL ERROR: This message is too large (") + len + "/" + sizeLimitPerChunk + "), it can't be store in the chunk, please increase it's size") ;
          } 
          bufferFull=true;
        }else{
          if(debugVerbosity>=DebugLevel::WARN) Serial.print(String("[ESP LOGGER] ###") + line.c_str() + "###");
          if(debugVerbosity>=DebugLevel::WARN) Serial.println(String(" Line length: ") + line.length() + "");
          // remove the last char, that is '\r'
          line=line.substring(0,line.length()-1);
          strcpy(&buffer[nBuffer], line.c_str());
          
          //len includes '\0'
          nBuffer += len;
        }
        
        if(oneRecordPerChunk) break;
      } // END OF WHILE - FILLING THE CHUNK
      
      if(nBuffer == 0){
        if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] No more data to send");
        break;
      }
  
      // Second step: send chunk
      if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] :::::::::::::::Second step: Chunk flushing...");
      successFlush=flusher(buffer,nBuffer);
      if(!successFlush) break;

    } // END OF FOR - ITERATING OVER THE CHUNKS

    if(!successFlush){
      if(chunkCount > 0){
        if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Partial unsuccessful sending!");
        // I have to discard the successfully sent log, and save the remainings.
        // NOTE: in some FS, the filename il limited to 8.3 charcters
        String tempFilePath = "/templog.tmp";
#ifdef ESP32        
        File tempFile=SD.open(tempFilePath,"w");
#elif ESP8266
        // Since FILE_WRITE is just like an append, I need to delete the file before opening
        SD.remove(tempFilePath);
        File tempFile=SD.open(tempFilePath, FILE_WRITE);
#endif
        if(f){
          saveChunk(tempFile,buffer,nBuffer);
          saveRemainings(tempFile, f);
          tempFile.close();
          f.close();
          // Riordino i file
          if(SD.remove(filePath)){
            if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] The old file is deleted!");
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
                if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Delete failed");
              }
            }else{
              if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] sSomething went wrong during the copy!");
            }

            if(success){
 #endif
              if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] The temp file is moved!");
            
            }else{
              if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] The temp file wasn't moved");
            }
          }else{
            if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] The temp file is NOT deleted!");
          }
          //return false; //refer https://github.com/fabiuz7/esp-logger-lib/issues/5
        }else{
          if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] Writing temp log file error!");
        }
      }else{
        // Nothing was sent, so I can close the file and exit from this function
        if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Unsuccessful sending! Nothing is flushed..");
      }
    }else{
      f.close();
      SD.remove(filePath);
    }

    // Free the memory buffer
    free(buffer);
    return successFlush;
  }else{
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] Opening log file error!");
  }
  if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] End of flushing the log file!");
  return false;
}

LoggerSD::LoggerSD(String file, DebugLevel debugVerbosity): Logger(file, debugVerbosity){
};

bool LoggerSD::begin(){
	return begin(SS);
}

bool LoggerSD::begin(int csPin){
#ifdef ESP8266
  if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] On ESP8266 this is not implemented, because multiple init are going to fail..");
  return false;
#endif  
  if(!SD.begin(csPin)){
    if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Card Mount Failed");
    return false;
  }
  return true;
}

unsigned int LoggerSD::getActualSize(){
  if(!SD.exists(filePath)){
    return 0;
  }

  unsigned int result = 0;
  File file=SD.open(filePath, FILE_READ);
  if(file){
    result=file.size();
    file.close();
  }
  return result;
}