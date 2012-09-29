# GenericMidiParser - Generic, platform independent, Midi File Parser
## by SkyWodd

This library is a generic, platform independent midi file parser.

This version can handle simultaneous tracks parsing (limit of simutaneous tracks is defined in header file).

But be aware, this version require lot of memory fetching to work.
So with low bandwidth support like serial this library may have some lags.
With SD card the result is pretty clean but depend of the SPI port frequency.

If possible considere using a buffer in RAM memory to store the midi file.
Everything in this library is callback drived, so it's pretty simple to implement ram page caching.

Another version who handle only one track at the time (do not require heavy memory fetching) is also available on my github.

---

This library is released with two examples of usage :
* one for arduino board (see ArduinoMidiParser directory)
* one for pc (see main.cpp)