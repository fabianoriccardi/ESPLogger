/**
 * Log on internal flash memory an event every 1.5 seconds. Every 30 seconds,
 * the log is flushed over serial port.
 * For more information about the available file systems and differences,
 * look at the readme.
 *
 * NOTE: the first time you run this sketch or when changing the file system
 *       layout, you should explicitly format the flash memory:
 *
 *          SPIFFS.format()
 */
#include <logger_fs.h>
#ifdef ESP32
#include <SPIFFS.h>
#endif

// Specify the target file system and the path where the log is placed
// NOTE: the path to log must exist!
LoggerFS myLogger(SPIFFS, "/data.log");

// Event generation period, in millisecond
unsigned int periodEvent = 1500;
// Flush period, in millisecond
unsigned int periodFlush = 30000;

int counter = 0;

void event()
{
  counter++;

  Serial.print("Hey, event ->");
  Serial.print(counter);
  Serial.println("<- is just happened");

  String record = String("val:") + counter;
  myLogger.append(record.c_str());
}

unsigned int prevTimeFlush = 0;
unsigned int prevTimeEvent = 0;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("ESP Logger - Basic Example");

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

  myLogger.setFlushCallback(flushHandler);
  myLogger.begin();

  Serial.println("Starting to log...");
}

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
 * Flush a chunck of logged records.
 * In this example, the records are trivially flushed on Serial.
 */
bool flushHandler(const char *buffer, int n)
{
  int index = 0;
  // Check if there is another string to print
  while (index < n && strlen(&buffer[index]) > 0)
  {
    Serial.print("---");
    int bytePrinted = Serial.print(&buffer[index]);
    Serial.println("---");
    // +1, the '\0' is processed
    index += bytePrinted + 1;
  }
  return true;
}
