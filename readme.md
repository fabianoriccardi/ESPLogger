# ESP Logger for Arduino IDE
ESP Logger is an Arduino library designed to standardize the basic operations on log files on ESP8266/ESP32 SoC.

### Motivations
Nowadays, tons of projects are related to IoT world: this implies network connection and data collection. ESP8266 and ESP32 were born with native WiFi connection paired with effective libraries to support developers. These libraries are very mature both in NONOS framework as well as in Arduino environment. Unfortunately, the software counterpart about data collection is not so effective and ready to use, despite the fact that every breakout board embeds a flash memory writable at runtime. Actually, the existing libraries supports raw files, forcing developers to implement every basic control like checking if the file is open, deciding the open mode, checking lines terminator, checking if file is ended, and so on. In my personal opinion, managing all these details everytime is boring and sloppy: avoiding them would be plus feature in many projects. A drawback in this approach is about performaces, a crucial point in microcontroller environment. However, I think that raising the abstraction level is an unquestionable benefit for makers and programmers interested in learning and prototyping advanced connected objects. Moreover, abstractions always increase code sharing and reusing, without forgetting that is a key point to promote communication among people, foundamental in Arduino-like communities. Of course, before ending up with my personal library, I searched on Internet to discover a piece of code putting together ease of use and flexibility to support data collection. I figured out that such a library doesn't exist: usually data collection projects implements their own custom solution and often, searching for "log" term, you will found a lot of results regarding software development and monitoring, not concrete data collection.

For all these reasons, I have developed ESP Logger library, that is build on top of flash memory and files, embracing the connectivity to easily shoot data over the network.

### Features
1. Support to SPIFFS (the internal Flash) and SD card
2. Methods to monitor log dimension, to avoid collision with other files
3. Support to multiple log files to store different information
4. Callback function to throw away or, better, to flush your data over the network. Three facts:
    * The data are sent in the order they are gathered 
    * While flushing, records are always sent once and only once
    * Assumption: flash memory is reliable, the data corruption is not managed
5. Full control on when and how flush your data
6. Agnostic with respect to data format, the atomic measurement unit is *Record*

### Installation
This is a regular Arduino library, so installiation is straightforward from Library Manager. If you desire the lastest Github version, you can download this repo and copy the entire folder in your Arduino Library directory. More info at [Arduino Library](https://www.arduino.cc/en/Guide/Libraries).

### API and Examples
After installing, you can see the examples to understand how easy is mastering this library. For advacend usages (and advanced users), the full library specifications can be found directy in *commented* header (*.h) files.