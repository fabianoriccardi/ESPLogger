#ifndef LOGGER_SD_H
#define LOGGER_SD_H

#include "logger.h"

/**
 * For further details look at the base class Logger
 */
class LoggerSD : public Logger{
  public:
  
  LoggerSD(String file, int debugVerbosity = 1);
  
  bool begin();
  bool begin(int csPin);

  bool append(String message, bool timestamp=true);

  void reset();
  
  void flush();

};

#endif // END LOGGER_SD_H
