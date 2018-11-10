#include "logger.h"

void Logger::setSizeLimit(unsigned int size, bool strict){
  sizeLimit=size;
  strictLimit=strict;
}

void Logger::setSizeLimitPerChunk(unsigned int size){
  sizeLimitPerChunk=size;
}

void Logger::setOneRecordPerChunk(bool one){
  oneRecordPerChunk=one;
}

void Logger::setFlusherCallback(bool (*callback)(char*, int)){
  flusher=callback;
}

unsigned int Logger::getSizeLimit(){
    return sizeLimit;
}

Logger::Logger(String file, int debugVerbosity): 
          filePath(file),
          sizeLimit(1000),
          strictLimit(true),
          sizeLimitPerChunk(100),
          oneRecordPerChunk(false),
          debugVerbosity(debugVerbosity),
          flusher([](char*,int){
                    Serial.println("[ESP LOGGER] Default flusher, please define your own flusher"); 
                    return true;
                  })
{
};