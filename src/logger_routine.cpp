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
#include "logger_routine.h"

void routine(LoggerRoutine* logRun){
	logRun->logger.flush();
	logRun->ticker.attach(logRun->period,routine,logRun);
}

bool LoggerRoutine::begin(bool startNow){
	if(period>=1){
		if(startNow){
			routine(this);
		}
		ticker.attach(period, routine, this);
		return true;
	}else{
		Serial.println("[ESP LOGGER] Period has to be greater or equal to 1s.");
		return false;
	}
	
}

void LoggerRoutine::setPeriod(float period, bool force){
	this->period = period;
	if(force){
		ticker.detach();
		ticker.attach(period, routine, this);
	}
}