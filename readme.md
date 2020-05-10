# ESP Logger for Arduino IDE

ESP Logger is an Arduino library offering a very simple but complete interface to log events on ESP32 and ESP8266. Given the connection-oriented applications making use of these MCUs, ESP Logger offers a simple way to buffer data and send them efficiently through the most appropriate communication protocol.

## Motivations

Nowadays, tons of projects are related to the IoT world: this implies network connection and data collection. ESP8266 and ESP32 were born with native WiFi paired with effective libraries to communication. These libraries are very mature both in NONOS framework as well as in Arduino environment. Unfortunately, I noticed that often there is the need to rewrite everything about the data collection. For example, the existing libraries support perfectly raw files, but for every project, you should implement every basic control such as check if the file is open, decide the open mode, check lines terminator, check if the file is ended, check if there is enough space and so on. In my personal opinion, managing all these details every time is boring and sloppy: avoiding them would be plus feature in many projects. I think that raising the abstraction level is an unquestionable benefit for Makers interested in prototyping advanced connected objects. Moreover, abstractions always increase code sharing and reusing, without forgetting that is a key point to promote communication among people, fundamental in Arduino-like communities. Before ending up with my library, I searched on the Internet to discover a library answering to the aforementioned requirements. I didn't find anything matching all of them because, usually, data collection projects implement their custom solution, and often, searching for "log" term, you will found a lot of results regarding software development and debugging, not concrete data collection.

For all these reasons, I have developed ESP Logger library, that is built on top of flash memory and files, leaving you the decision on when logging, when flushing data, and how to flush data.

## Features

1. Log on internal flash and or on SD card*
2. Methods to monitor and limit log size
3. Support for multiple log files to store different information
4. Callback to flush your data over the network. 
5. Full control on when and how to flush your data
6. Agnostic to the data format, the atomic measurement unit is *Record*

* On ESP8266, you can decide if using SPIFFS or the newer LittleFS file system for flash memory.

## Installation

This is a regular Arduino library, so installation is straightforward from the Library Manager. If you desire the latest Github version, you can download this repo and copy the entire folder in your Arduino Library directory. More info at [Arduino Library](https://www.arduino.cc/en/Guide/Libraries).

## Usage

There are 2 main functions to be aware of: *append* and *flush*.

#### Append

    bool append(String data)

it creates and stores a *Record* containing the input data. You cannot log data containing non-printable characters, nor new line or carriage return. Return true if the data is saved, false otherwise.

#### Flush

    bool flush()

it calls your callback function which has the following prototype:

    bool flusher(char* buffer, int n);

where char* is a buffer that contains one or more records, *n* tells how many bytes is long this buffer. This function should return true is the buffer flush has succeeded, false otherwise (e.g. the server wasn't reachable). If true, the library deletes the flushed data and, if other data are available, it calls flusher() again, otherwise it stops. If false, the library stops the flushing process and preserves the unflushed data for the next flush(). 

This kind of packetization can be useful in various scenarios, especially when the log is very large and you cannot send everything in one shot. The max buffer size can be arbitrarily set at run-time, though a dedicated method.

Please note that the flush() method guarantees that: 

    * The data are sent in the order they were recorded 
    * During a single flush(), a record is sent at most once
    * At the first failure, the flush() method ends
    
It returns true if flush succeeds (hence the log file is emptied), false otherwise.

### Other APIs and examples

To complete the overview of the library, you may look at the *examples* folder, in which you can find some working examples to understand how easy is to master ESP Logger library. For advanced usage, the full library specifications can be found directly in *commented* header (*.h) files.

## Limitations

ESP Logger is designed to log human-readable data, this means that data should be composed only by readable characters (excluding also carriage return and newline).

Data corruption is not managed, hence there is no CRC or other integrity system to check that your log file is not altered. It assumes that the flash memory is reliable and no other code will access the log file.
