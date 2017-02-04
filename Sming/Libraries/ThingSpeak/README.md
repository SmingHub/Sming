# Sming 
## ThingSpeak driver libary
### Usage
#### Declatration

First way set write APIKey in the constructor.
```C++
ThingSpeak thingSpeak("APIKey"); //String of ThingSpeak APIKey
```
second way is set in the function:

```C++
ThingSpeak thingSpeak; //free constructor
thingSpeak.setWriteAPIKey( APIKey ); // APIKey - string varible of Write API TS Key
```
You can use **setWriteAPIKey** whenever you need, while changing API by user etc.
