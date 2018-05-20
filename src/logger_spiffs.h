#ifndef LOGGER_SPIFFS_H
#define LOGGER_SPIFFS_H

#include <Arduino.h>
#include "logger.h"

/**
 * For further details look at the base class Logger
 */
class LoggerSPIFFS : public Logger{
  public:
  
  LoggerSPIFFS(String file, int debugVerbosity = 1);

  bool begin();

  bool append(String message, bool timestamp=true);

  void reset();
  
  void flush();

};

#endif // END LOGGER_SPIFFS_H
