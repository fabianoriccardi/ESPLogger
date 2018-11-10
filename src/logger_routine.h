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
#ifndef LOGGER_ROUTINE_H
#define LOGGER_ROUTINE_H

#include <Ticker.h>

#include "logger.h"

/**
 * Class for logger routine, that means an helper to manage a periodic
 * flush of the log file.
 */
class LoggerRoutine{
public:
	LoggerRoutine(Logger& logger, float period): 
						logger(logger),
						period(period){}; 

	/**
	 * Starts the periodic logging routine. If startNow is set, 
	 * the flushing will be performed immediately, then the interrupt 
	 * period will be set. It returns false is case of messy period.
	 */
	bool begin(bool startNow=false);

	/**
	 * Sets the period. If the force parameter is true,
	 * the actual setting is immediately suspended and 
	 * replaced with the new period
	 */
	void setPeriod(float period, bool force);

private:
	Logger& logger;
	Ticker ticker;

	float period;

	friend void routine(LoggerRoutine* logRun);
};

#endif // END LOGGER_ROUTINE_H