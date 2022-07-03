# ESPLogger

[![arduino-library-badge](https://www.ardu-badge.com/badge/ESPLogger.svg)](https://www.ardu-badge.com/badge/ESPLogger.svg) ![Compile Library Examples](https://github.com/fabianoriccardi/ESPLogger/actions/workflows/LibraryBuild.yml/badge.svg)

ESPLogger is an Arduino library offering a simple but complete interface to log events on ESP32 and ESP8266. Given the connection-oriented applications using these MCUs, ESPLogger provides a simple way to buffer data and efficiently send them through the most appropriate communication protocol.

## Motivations

Nowadays, tons of projects are related to the IoT world: this implies network connection and data collection. ESP8266 and ESP32 were born with native Wi-Fi paired with effective libraries to communicate. These libraries are very mature both in NONOS framework and in Arduino environment. Unfortunately, I noticed that often there is the need to rewrite everything about the data collection. For example, the existing libraries support perfectly raw files, but for every project, you should implement every basic control such as check if the file is open, decide the open mode, check lines terminator, check if the file is ended, check if there is enough space and so on. In my personal opinion, managing all these details every time is tedious and sloppy: avoiding them would be a plus feature in many projects. Raising the abstraction level is an unquestionable benefit for Makers interested in prototyping advanced connected objects. Moreover, abstractions always increase code sharing and reuse, without forgetting that it is a key point to promote communication among people, fundamental in Arduino-like communities. Before ending up with my library, I searched on the Internet to discover a library answering the requirements above. I didn't find anything matching all of them because, usually, data collection projects implement their custom solution, and often, searching for "log" term, you will find a lot of results regarding software development and debugging, not concrete data collection.

For all these reasons, I have developed ESPLogger library, which is built on top of flash memory and files, leaving you the decision on when logging, when flushing data, and how to flush data.

## Features

1. Log on internal flash (using LittleFS or SPIFFS) and/or on SD card
2. Monitor and limit log size
3. Support for multiple log files
4. Flush data when and how you need it on different channels (HTTP, Serial, ...)

## Requirements

ESPLogger v2 is compatible with the following versions of Arduino Core (PIO Platform version between parentheses):

| ESP8266           | ESP32             |
|-------------------|-------------------|
| ^2.6.0 (^2.3.0)   |   ^2.0.0 (^4.0.0) |
| ^3.0.0 (^3.0.0)   |                   |

## Installation

The latest version of ESPLogger is available on Arduino Library Manager and on [PlatformIO registry](https://platformio.org/lib/show/5879/ESPLogger).

## Usage

There are 2 main functions to be aware of: `append` and `flush`.

### Append

    bool append(const char* record)

it creates and stores a *Record*. You cannot log data containing non-printable characters, nor new line or carriage return. It returns true if the data is saved, false otherwise.

### Flush

    bool flush()

it calls your callback function with the following prototype:

    bool flusher(const char* chunk, int n);

where *chunk* is a pointer to a buffer that contains one or more records separated by '\0' char; *n* tells the effective data size in the chunk buffer, including '\0's. This value is always <= `chuckSize`. This function should return true if the flush process succeeds, false otherwise (e.g. the server wasn't reachable). If true is returned, the library deletes the flushed data and, if other data are available, it calls `flusher()` again, otherwise it stops. If false, the library stops the flushing process and preserves the unflushed data for the next flush().

This kind of packetization can be useful in various scenarios, especially when the log is very large and you cannot send everything in a single shot. The maximum size of a chunk can be set at run-time through `setChuckSize()`.

Please note that the flush() method guarantees that:

    * The data are sent in the order they were recorded 
    * During a single flush(), a record is sent at most once
    * At the first failure signalled by `flusher` callback, the flush() method ends

It returns true if flush succeeds (hence the log file is emptied), false otherwise.

### File systems

ESPLogger requires a file system to read and write the log. At the time of writing, there are few supported filesystems with minor differences between ESP32 and ESP8266:

* `LittleFS`: the preferred file system for ESP8266 and ESP32 and the default for ESPLogger.
* `SPIFFS`: the previous default file system before LittleFS. On ESP32, you have to `#include <SPIFFS.h>`.
* `SDFS` (only for ESP8266): the standard file system for micro SD cards on ESP8266. It supports both FAT and FAT32. You have to `#include <SDFS.h>`. Do not use *SD* since it has an incompatible interface!
* `SD` (only for ESP32): the standard file system for micro SD cards on ESP32. It supports both FAT and FAT32. You have to `#include <SD.h>`.

Be sure that the directory tree to the log file exists before calling `begin()`. Some file systems create the necessary path when opening a file (like SPIFFS), other won't (like SD for esp32).

### Other APIs and examples

Look at the *examples* folder for working sketches and at the *commented* header files (*.h) for the details.

## Limitations

ESPLogger is designed to log human-readable data, this means that data should be composed only by readable characters (excluding also carriage return (`\r`) and newline (`\n`)).

Data corruption is not managed, hence there is no CRC or other integrity system to check that your log file is not altered. It assumes that the flash memory is reliable and no other code will access the log file.
