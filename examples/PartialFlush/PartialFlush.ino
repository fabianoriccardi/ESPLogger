/**
 * Log an event every second on the internal flash memory. You can observe that after a few logs,
 * the available space ends, and the logger refuses to log more records until it is flushed or
 * reset. Moreover, the flush callback simulates a random failure, so the log will be only partially
 * flushed. In this case, the library discards only the chunks marked as "flushed", freeing some
 * space on memory and preserving the not flushed record for the next attempt.
 *
 * NOTE: you should format the flash memory the first time you run this sketch or when you switch
 * file system. Use <YourFS>.format().
 */
#include <ESPLogger.h>

// NOTE: the directories to the log file must exist!
ESPLogger logger("/data.log");

// Event generation period, in milliseconds
unsigned int eventPeriod = 1000;
// Flush period, in milliseconds
unsigned int flushPeriod = 20000;

void event() {
  // Variable to be logged
  static int counter = 0;
  counter++;

  Serial.printf("Hey, event ->%d<- is just happened\n", counter);
  char buffer[15];
  snprintf(buffer, 15, "value: %d", counter);
  // the second parameter allows to prepend to the record the current timestamp
  bool success = logger.append(buffer, true);
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
  Serial.println("ESPLogger - Partial flush");

  // You may need to format the flash before using it
  // LittleFS.format();

  if (LittleFS.begin()) {
    Serial.println("File system mounted");
  } else {
    Serial.println("File system NOT mounted. System halted");
    while (1) delay(100);
  }

  logger.setSizeLimit(200);
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
 * Flush a chunk of records over the serial port.
 * This function simulates randomly a failure which prevents the complete flush.
 * When this happens, ESPLogger retains the remaining log for the next flush.
 */
bool flushHandler(const char *buffer, int n) {
  static int failPacketCounter = 0;
  static int failPacket = random(3, 5);

  // Check if it is time to simulate a failure
  if (failPacketCounter == failPacket) {
    failPacket = random(3, 5);
    failPacketCounter = 0;
    Serial.println(String("Cannot send chuck. Next failure happens in: ") + failPacket + " chunks");

    // Tell to the logger that current chunk wasn't properly handled
    return false;
  } else {
    Serial.println("Elaborate chunk:");
    int index = 0;
    // Check if there is another string to print
    while (index < n && strlen(&buffer[index]) > 0) {
      Serial.print("  ---");
      int bytePrinted = Serial.print(&buffer[index]);
      Serial.println("---");
      // +1, the '\0' is processed
      index += bytePrinted + 1;
    }

    failPacketCounter++;
    // Tell to the logger that current chunk was properly handled
    return true;
  }
}