/**
 * Log on internal flash memory an event every 1 second.
 * You can see that after some logs, the available space ends, and the logger
 * refuses to log more records until it is flushed. Moreover, the flush process
 * simulates a random failure, so the log may partially be emptied,
 * discarding only the chunks marked as "flushed".
 * For more information about the available file systems and differences,
 * look at the readme.
 *
 * NOTE: the first time you run this sketch or when changing the file system
 *       layout, you should explicitly format the flash memory:
 *
 *          SPIFFS.format()
 */
#include <logger.h>
#ifdef ESP32
#include <SPIFFS.h>
#endif

// Specify the target file system and the path where the log is placed
// NOTE: the path to log must exist!
Logger myLogger("/data.log", SPIFFS);

// Event generation period, in millisecond
unsigned int periodEvent = 1000;
// Flush period, in millisecond
unsigned int periodFlush = 20000;

void event()
{
  // Variable to be logged
  static int counter = 0;
  counter++;

  Serial.printf("Hey, event ->%d<- is just happened\n", counter);
  char buffer[15];
  snprintf(buffer, 15, "value: %d", counter);
  bool success = myLogger.append(buffer);
  if (success)
  {
    Serial.println("Event stored!");
  }
  else
  {
    if (myLogger.isFull())
    {
      Serial.println("Event NOT stored! You had filled the available space, flush or reset the log");
    }
    else
    {
      Serial.println("Event NOT stored!");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("ESP Logger - Handle Failure");

  // Maybe you need to format the flash before using it
  // SPIFFS.format();

  if (SPIFFS.begin())
  {
    Serial.println("Filesystem mounted successfully");
  }
  else
  {
    Serial.println("Filesystem NOT mounted. System halted");
    while (1)
      delay(100);
  }

  myLogger.setSizeLimit(200);
  myLogger.setFlushCallback(flushHandler);
  myLogger.begin();

  Serial.println("Starting to log...");
}

unsigned int prevTimeFlush = 0;
unsigned int prevTimeEvent = 0;

void loop()
{
  // This loop is the logger controller, it decides
  // when it's time to flush.
  if (millis() - prevTimeFlush > periodFlush)
  {
    prevTimeFlush += periodFlush;
    Serial.println("Time to flush");
    myLogger.flush();
  }

  if (millis() - prevTimeEvent > periodEvent)
  {
    prevTimeEvent += periodEvent;
    event();
  }
}

/**
 * Flush a chunk of logged records.
 * This function simulate a random failure. When this happen, ESP Logger
 * retains the remaing log for the next flush.
 */
bool flushHandler(const char *buffer, int n)
{
  static int failPacketCounter = 0;
  static int failPacket = random(3, 5);

  // This part is the example about the management of sending failure.
  if (failPacketCounter == failPacket)
  {
    failPacket = random(3, 5);
    failPacketCounter = 0;
    Serial.println(String("Cannot send chuck. Next failure happens in: ") + failPacket + " chunks");

    // Tell to logger that current chunk wasn't properly handled
    return false;
  }
  else
  {
    Serial.println("Elaborate chunk:");
    int index = 0;
    // Check if there is another string to print
    while (index < n && strlen(&buffer[index]) > 0)
    {
      Serial.print("  ---");
      int bytePrinted = Serial.print(&buffer[index]);
      Serial.println("---");
      // +1, the '\0' is processed
      index += bytePrinted + 1;
    }

    failPacketCounter++;
    // Tell to logger that current chunk was properly handled
    return true;
  }
}