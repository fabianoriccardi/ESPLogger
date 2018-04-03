#ifndef LOGGER_ROUTINE_H
#define LOGGER_ROUTINE_H

#include <Ticker.h>

#include "logger.h"

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