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
          strictLimit(true),
          sizeLimitPerChunk(100),
          oneRecordPerChunk(false),
          debugVerbosity(debugVerbosity),
          flusher([](char*,int){
          					Serial.println("Default flusher, please define your own flusher"); 
          					return true;
          				}){
    
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
  void setSizeLimitPerChunk(int size);

  /**
   * Sets if the logger shoud prepare chunk with at most one record.
   */
  void setOneRecordPerChunk(bool one);

  /**
   * Sets the send chunk callback.
   */
  void setFlusherCallback(bool (*foo)(char*, int) );

  /** 
   * Append a line to the target file. 
   * Return true if the record is succefully stored, otherwise false.
   */
  bool append(String message,bool timestamp=true);

  /**
   * Delete the current log file
   */
  void reset();
  
  /**
   * Send all the data through the network.
   * The parameter allows to force the sending a record per chunk
   */
  void flush();
  
  private:
  String filePath;

  /**
   * Maximum dimension of log size.
   */
  unsigned int sizeLimit;

  /**
   * Strict limimt than respect the file dimension.
   */
  bool strictLimit;

  /**
   * Physical dimension of a single chunk, in byte
   * This limit is ALWAYS respected!
   */
  unsigned int sizeLimitPerChunk;

  /**
   * Sets if the logger shoud prepare chunk with at most one record.
   */
  bool oneRecordPerChunk;

  /**
   * Print
   * 0 - no messages
   * 1 - error and important messages
   * 2 - all the messages
   */
  int debugVerbosity;

  /**
   * Callback called during the flush function. When a chunk is filled,
   * it is passed to this callback. The first paramenter is the buffer
   * containing a bunch of records, '\0' separated. The second parameter 
   * is the logic size of the buffer (i.e. the content's length, '\0' included).
   */
  bool (*flusher)(char*, int);

};

#endif // END LOGGER_H
