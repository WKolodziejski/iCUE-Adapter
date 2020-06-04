# iCUE-Adapter
This repository uses the [iCUE SDK](https://github.com/CorsairOfficial/cue-sdk) to read and send data to Arduino and the [Fast LED](https://github.com/FastLED/FastLED) library to control the LEDs.

To control the LEDs I've activated the **DEMO MODE**.

## Working
The program runs on system tray and has a floating menu:

![Menu img](https://github.com/WKolodziejski/iCUE-Adapter/blob/master/Menu.PNG?raw=true)

and console to output messages:

![Console img](https://github.com/WKolodziejski/iCUE-Adapter/blob/master/Console.PNG?raw=true)

**Do not** close the console or application will terminate. To hide the console, just select it in the menu again.

The first 3 options are used to select wich mode the LEDs will be doubled (*explained in Notice 2*).

## Build
I used the VS 2019 to compile this project, under tools > developer console, using the following command:
```
cl /EHsc main.cpp iCUE.cpp Serial.cpp CUESDK_2017.lib /link user32.lib shell32.lib /subsystem:windows /out:Adapter.exe
```
If you manage to compile with gcc, please let me know.

## Notice 1
I used the Arduino to read the temperature too, in order to control a fan speed that is not PWM.
To do that, I chose the DEMO COMMANDER PRO and set the colors this way:

![Temp img](https://github.com/WKolodziejski/iCUE-Adapter/blob/master/Temp.PNG?raw=true)

But that's optional.

## Notice 2
My LED strip has 17 LEDs, so I did some tricks to the LEDs reading from SDK.
Basically, my program reads 9 LEDs from iCue, sends to Arduino and it double the leds where the first LED of iCUE strip is ignored; the second one is mapped 1:1; and the others are 1:2.

![Strip img](https://github.com/WKolodziejski/iCUE-Adapter/blob/master/Strip.PNG?raw=true)
