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