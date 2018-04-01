#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <FS.h>

/**
 * This class is responsible for a single log file.
 * This file should be access only by this class,
 */
class Logger{
  public:
  
  Logger(String file, int debugVerbosity = 1): 
          filePath(file),
          sizeLimit(1000),
          sizeLimitPerPacket(100),
          strictLimit(true),
          debugVerbosity(debugVerbosity){
    
  };

  /**
   * Impose a limit the log file.
   * This is very important to avoid the memory congestion. 
   */
  void setSizeLimit(int size, bool strict);

  /**
   * Set the maximum byte that can be carried on a single packet.
   * Useful in case your protocol is limited or in case of few RAM available
   * (Remember that the data has to live in RAM for a moment before they are sent
   * over the network)
   */
  void setSizeLimitPerPacket(int size);

  /** 
   * Append a line to the target file 
   */
  void append(String message,bool timestamp=true);

  /**
   * Delete the current log file
   */
  void reset();
  
  /**
   * Send all the data through the network
   */
  void sendAll();

  /**
   * 
   */
  void sendAll2();
  
  private:
  String filePath;

  /**
   * Measured in bytes
   */
  unsigned int sizeLimit;
  unsigned int sizeLimitPerPacket;

  bool strictLimit;

  /**
   * Print
   * 0 - no messages
   * 1 - error and important messages
   * 2 - all the messages
   */
  int debugVerbosity;
};

#endif // END LOGGER_H
