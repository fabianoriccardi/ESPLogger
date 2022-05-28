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
#ifndef LOGGER_H
#define LOGGER_H

#include <FS.h>
#include <LittleFS.h>

class Logger
{
public:
  typedef bool (*CallbackFlush)(const char *buffer, int n);

  Logger(String file, FS &fs = LittleFS);

  virtual ~Logger(){};

  /**
   * Check the logger configuration.
   * Return true if logger can start, false otherwise.
   */
  virtual bool begin();

  /**
   * Set a limit to the log size.
   * This is very important to avoid the memory saturation.
   * The param strict is used to enforce the relation
   * actual maxLogSize <= size, otherwise if false, the
   * following relation is applied:
   * maxLogSize <= size+recordSize
   */
  void setSizeLimit(unsigned int size, bool strict = true);

  /**
   * Get maximum log size.
   */
  unsigned int getSizeLimit() const;

  /**
   * Set the maximum byte that can be inserted in a single chunk.
   * Useful when few RAM is available (remember that the data
   * has to live in RAM for a moment before they are flushed).
   */
  void setSizeLimitPerChunk(unsigned int size);

  /**
   * Sets if the logger must prepare single-record chunks.
   */
  void setOneRecordPerChunk(bool one);

  /**
   * Sets the callback used during the flushing.
   */
  void setFlushCallback(CallbackFlush callback);

  /**
   * Append a record to the log file. This method is zero-copy.
   * Return true if the record is successfully stored, otherwise false.
   */
  bool append(const String &record, bool timestamp = true);
  virtual bool append(const char *record, bool timestamp = true);

  /**
   * Send all the data through the callback function.
   */
  virtual bool flush();

  /**
   * Delete the current log file.
   */
  virtual void reset();

  /**
   * Send all to Serial. It doesn't delete any record.
   */
  virtual void print() const;

  /**
   * Get actual log size.
   */
  virtual unsigned int getActualSize() const;

  /**
   * Tell if the log is full.
   * This value will altered only by append(), flush() or reset().
   */
  virtual bool isFull() const;

protected:
  String filePath;
  FS &fs;

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
   * Callback called during the flushing. The first parameter is the buffer
   * containing one or more records, separated by '\0' char. The second parameter
   * is the content's length, '\0' included.
   *
   * The return value is used to determine if the flush process should continue.
   * True means that the chunk was correctly flushed and the process can continue;
   * false means that there was an error, so flush process must stop.
   */
  CallbackFlush onFlush;

  bool full;

  /**
   * A brief enumaration to classify the message's severity.
   */
  enum class DebugLevel
  {
    QUIET = 0,
    FATAL = 1,
    ERROR = 2,
    WARN = 3,
    INFO = 4,
    DEBUG = 5,
    TRACE = 6
  };

  /**
   * A function to translate the enum value to human friendly string.
   */
  static const char *translate(DebugLevel level);

  /**
   * Debug level.
   */
  static const DebugLevel debugVerbosity = DebugLevel::QUIET;
};

#endif // END LOGGER_H
