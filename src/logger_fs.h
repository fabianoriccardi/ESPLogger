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
#ifndef LOGGER_FS_H
#define LOGGER_FS_H

#include "logger.h"

#include <FS.h>

/**
 * Implementation of Logger based on FS class provided by standard Arduino APIs.
 */
class LoggerFS : public Logger
{
public:
  LoggerFS(FS &fs, const char *file);
  LoggerFS(FS &fs, String file);

  bool begin();

  using Logger::append;
  bool append(const char *record, bool timestamp = true);

  void reset();

  bool flush();

  void print() const;

  unsigned int getActualSize() const;

  bool isFull() const;

private:
  FS &fs;
};

#endif // END LOGGER_SPIFFS_H
