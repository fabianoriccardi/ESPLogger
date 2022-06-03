/**
 * Log an event every 1.5 second on the internal flash memory and flush it every 20 seconds, the log
 * is flushed over serial port. You can see that after a few records the available space ends and
 * the logger refuses to log more records until it is flushed or reset.
 *
 * NOTE: you should format the flash memory the first time you run this sketch or when you switch
 * file system. Use <YourFS>.format().
 */
#include <ESPLogger.h>

// Specify the path and the file system where the log will be placed.
// LittleFS is the default file system, but you may select different alternatives (see below, and
// remember to #include the selected file system and to initialize it).
// NOTE: the directories to the log file must exist!
ESPLogger logger("/data.log");
// ESPLogger logger("/data.log", SPIFFS);
// ESPLogger logger("/data.log", SD);   // Only for ESP32
// ESPLogger logger("/data.log", SDFS); // Only for ESP8266

// Event period, in milliseconds
unsigned int eventPeriod = 1500;
// Flush period, in milliseconds
unsigned int flushPeriod = 20000;

// Variable to be logged
static int eventCounter = 0;

void event() {
  eventCounter++;

  Serial.printf("Hey, event ->%d<- is just happened\n", eventCounter);
  char record[15];
  snprintf(record, 15, "value: %d", eventCounter);
  // the second parameter allows to prepend to the record the current timestamp
  bool success = logger.append(record, true);
  if (success) {
    Serial.println("Record stored!");
  } else {
    if (logger.isFull()) {
      Serial.println("Record NOT stored! You had filled the available space: flush or reset the "
                     "log before appending another record");
    } else {
      Serial.println("Something goes wrong, record NOT stored!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("ESPLogger - Basic example");

  // You may need to format the flash before using it
  // LittleFS.format();

  if (LittleFS.begin()) {
    Serial.println("File system mounted");
  } else {
    Serial.println("File system NOT mounted. System halted");
    while (1) delay(100);
  }

  // Set the space reserved to the log (in bytes)
  logger.setSizeLimit(100);
  logger.setFlushCallback(flushHandler);
  logger.begin();

  Serial.println("Starting to log...");
}

unsigned long previousFlushTime = 0;
unsigned long previousEventTime = 0;

void loop() {
  // Check if it's time to flush
  if (millis() - previousFlushTime > flushPeriod) {
    previousFlushTime += flushPeriod;
    Serial.println("Flushing...");
    logger.flush();
    Serial.println("Done!");
  }

  if (millis() - previousEventTime > eventPeriod) {
    previousEventTime += eventPeriod;
    event();
  }
}

/**
 * Flush a chunck of records over the serial port.
 */
bool flushHandler(const char *buffer, int n) {
  int index = 0;
  // Print a record at a time
  while (index < n && strlen(&buffer[index]) > 0) {
    Serial.print("---");
    int bytePrinted = Serial.print(&buffer[index]);
    Serial.println("---");
    // +1, since '\0' is processed
    index += bytePrinted + 1;
  }
  return true;
}
