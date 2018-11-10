/***************************************************************************
 *   Copyright (C) 2018 - Fabiano Riccardi                                 *
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
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

/**
 * This class is responsible for a log file.
 */
class Logger{
  public:
  
  Logger(String file, int debugVerbosity = 1);

  /**
   * Activate the Filesystem, call before starting to append or flush,
   * return true is case of successfull init, false otherwise
   */
  virtual bool begin() = 0;

  /**
   * Impose a limit the log file.
   * This is very important to avoid the memory saturation.
   * The param strict is used to enforce the relation
   * actual size <= total size
   */
  void setSizeLimit(unsigned int size, bool strict = true);

  /**
   * Set the maximum byte that can be carried on a single chunk.
   * Useful in case of few RAM available (remember that the data 
   * have to live in RAM for a moment before they are sent
   * over the network)
   */
  void setSizeLimitPerChunk(unsigned int size);

  /**
   * Sets if the logger shoud prepare chunk with at most one record.
   */
  void setOneRecordPerChunk(bool one);

  /**
   * Sets the callback used during the flushing.
   */
  void setFlusherCallback(bool (*foo)(char*, int) );

  /** 
   * Append a line to the target file. 
   * Return true if the record is succefully stored, otherwise false.
   */
  virtual bool append(String message, bool timestamp = true) = 0;

  /**
   * Delete the current log file.
   */
  virtual void reset() = 0;
  
  /**
   * Send all the data through the callback function.
   */
  virtual void flush() = 0;

  /**
   * Get actual log size.
   */
  virtual unsigned int getActualSize() = 0;

  /**
   * Get maximum log dimension
   */
  unsigned int getSizeLimit();

  virtual ~Logger();
  
  protected:
  String filePath;

  /**
   * Actual dimension used by log file
   */
  unsigned int actualSize;

  /**
   * Maximum dimension of log size. It includes the terminator chars.
   */
  unsigned int sizeLimit;

  /**
   * Strict limit with respect to the file dimension.
   */
  bool strictLimit;

  /**
   * Physical dimension of a single chunk, in byte
   * This limit is ALWAYS respected! It includes the terminator chars.
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
   * This fucntion returns a boolean: true means that the chunk was correclty 
   * sent over the network (or properly managed by the user), false means that 
   * there was an error, so the chunk should be preserved for future re-sending, moreover,
   * returning false stops the flushing function.
   */
  bool (*flusher)(char*, int);

};

#endif // END LOGGER_H
