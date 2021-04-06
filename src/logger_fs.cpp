/***************************************************************************
 *   Copyright (C) 2018-2021  Fabiano Riccardi                             *
 *                                                                         *
 *   This file is part of ESP Logger.                                      *
 *                                                                         *
 *   ESP Logger is free software; you can redistribute                     *
 *   it and/or modify it under the terms of the GNU General Public         *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   ESP Logger is distributed in the hope that it will be useful,         *
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
 *   along with ESP Logger; if not, see <http://www.gnu.org/licenses/>     *
 ***************************************************************************/
#include "logger_fs.h"

LoggerFS::LoggerFS(FS& fs, const char* file):
    Logger(file), fs(fs) {}

LoggerFS::LoggerFS(FS& fs, String file):
    Logger(file), fs(fs) {}

bool LoggerFS::begin(){
  if(filePath.lastIndexOf('/') == 0 || fs.exists(filePath.substring(0, filePath.lastIndexOf('/')))){
    return true;
  }
  if (debugVerbosity >= DebugLevel::ERROR) Serial.println("[ESP LOGGER] File path to log not exist!");
  return false;
}

bool LoggerFS::append(const char* record, bool timestamp){
  // Max 10 digits in an integer
  char timestampString[11] = {0};
  if(timestamp){
    unsigned int now=millis();
    itoa(now, timestampString, 10);
  }

  unsigned int recordLength = strlen(record);
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
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] ERROR: This message is too large, it can't be sent because the limitation on chunk size, please change it before continue!!!");
    return false;
  }

  if(recordLength>sizeLimit){
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] ERROR: This message is too large, it can't be stored because the limitation on file size, please change it before continue!!!");
    return false;
  }
  
  // Trick to be sure that an empty file exist, and it's dimension is 0 (BUG in esp32)
  if(!fs.exists(filePath)){
    File file=fs.open(filePath,"w");
    if(file){
      file.close();
    }
  }

  File f=fs.open(filePath,"a");
  if(!f) {
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
    full = true;
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

void LoggerFS::reset(){
  if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Resetting the log file...");
  if(fs.exists(filePath)){
    if(fs.remove(filePath)){
      full = false;
    }
  }else{
    full=false;
  }
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

bool LoggerFS::flush(){
  if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Flushing the log file...");
  
  if(!fs.exists(filePath)){
    if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] File doesn't exist, nothing to flush..");
    return true;
  }

  if(onFlush == nullptr){
    if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] No Flush callback");
    return false;
  }
  
  File f=fs.open(filePath,"r");
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
      // Build the chunk
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
            if (debugVerbosity>=DebugLevel::ERROR) Serial.println(String("[ESP LOGGER] ERROR: This message is too large (") + len + "/" + sizeLimitPerChunk + "), it can't be store in the chunk, please increase it's size") ;
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
      successFlush=onFlush(buffer,nBuffer);
      if(!successFlush) break;

    } // END OF FOR - ITERATING OVER THE CHUNKS

    if(!successFlush){
      if(chunkCount > 0){
        if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Partial unsuccessful sending!");
        // I have to discard the successfully sent log, and save the remainings.
        String tempFilePath = filePath+".temp";
        File tempFile=fs.open(tempFilePath,"w");
        if(f){
          saveChunk(tempFile,buffer,nBuffer);
          saveRemainings(tempFile, f);
          tempFile.close();
          f.close();
          // Riordino i file
          if(fs.remove(filePath)){
            if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] The old file is deleted!");
            if(fs.rename(tempFilePath, filePath)){
              if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] The temp file is moved!");
              full = false;
            }else{
              if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] The temp file wasn't moved");
            }
          }else{
            if (debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] The temp file is NOT deleted!");
            full = false;
          }
          //return false; //refer https://github.com/fabiuz7/esp-logger-lib/issues/5
        }else{
          if (debugVerbosity>=DebugLevel::ERROR) Serial.println("[ESP LOGGER] Writing temp log file error!");
        }
      }else{
        // Nothing was sent, so I can close the file and exit from this function
        if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] Nothing was flushed..");
      }
    }else{
      f.close();

      if(fs.remove(filePath)){
        full = false;
      }else{
        // Technically this should not happen
        if (debugVerbosity>=DebugLevel::FATAL) Serial.println("[ESP LOGGER] Flush successfully completed, but I cannot remove the file...");
      }
    }

    // Free the memory buffer
    free(buffer);
    return successFlush;
  }else{
    if (debugVerbosity>DebugLevel::ERROR) Serial.println("[ESP LOGGER] Opening log file error!");
  }
  if(debugVerbosity>=DebugLevel::WARN) Serial.println("[ESP LOGGER] End of flushing the log file!");
  return false;
}

unsigned int LoggerFS::getActualSize(){
  if(!fs.exists(filePath)){
    return 0;
  }

  unsigned int result=0;
  File file=fs.open(filePath, "r");
  if(file){
    result=file.size();
    file.close();
  }
  return result;
}

bool LoggerFS::isFull(){
  return full;
}
