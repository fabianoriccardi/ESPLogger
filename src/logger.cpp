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
#include "logger.h"

String Logger::translate(DebugLevel level){
  switch(level){
    case DebugLevel::QUIET:
      return "QUIET";
    case DebugLevel::FATAL:
      return "FATAL";
    case DebugLevel::ERROR:
      return "ERROR";
    case DebugLevel::WARN:
      return "WARN";
    case DebugLevel::INFO:
      return "INFO";
    case DebugLevel::DEBUG:
      return "DEBUG";
    case DebugLevel::TRACE:
      return "TRACE";
    default:
      return "";
  }
};

bool Logger::append(String record, bool timestamp){
  return append(record.c_str(), timestamp);
}

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

Logger::Logger(String file, DebugLevel debugVerbosity):
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

Logger::~Logger(){}
