Thanks to: https://raw.githubusercontent.com/g30r93g/homebridge-panasonic/master/README.md

Or this one:
https://forums.indigodomo.com/viewtopic.php?f=134&t=17875&start=15

## App List

This is a partial list of apps that are on Viera TV's. Make sure that the app exists on your TV.

|App Name|App ID|
|:---|:---------------:|
|Netflix|`0010000200000001`|
|YouTube|`0070000200170001`|
|Amazon Prime Video|`0010000100170001`|
|Plex|`0076010507000001`|
|BBC iPlayer|`0020000A00170010`|
|BBC News|`0020000A00170006`|
|BBC Sport|`0020000A00170007`|
|ITV Hub|`0387878700000124`|
|TuneIn|`0010001800000001`|
|AccuWeather|`0070000C00000001`|
|All 4|`0387878700000125`|
|Demand 5|`0020009300000002`|
|Rakuten TV|`0020002A00000001`|
|CHILI|`0020004700000001`|
|STV Player|`0387878700000132`|
|Digital Concert Hall|`0076002307170001`|
|Apps Market|`0387878700000102`|
|Browser|`0077777700160002`|
|Calendar|`0387878700150020`|
|VIERA Link|`0387878700000016`|
|Recorded TV|`0387878700000013`|
|Freeview Catch Up|`0387878700000109`|

If you want to find the App ID for an app yourself, follow these steps:

1.  Install Wireshark
2.  Install Panasonic TV Remote on your mobile device
3.  Use your mobile device as a network interface in Wireshark
4.  Use the Panasonic TV Remote app launcher to open the application you want
5.  Filter results by 'http' and find the request to open YouTube (It will be in the XML with a tag called `X_LaunchApp`)
6.  Find the app ID. It will look something like `<X_LaunchKeyword>product_id=000000000</X_LaunchKeyword>`. `product_id` is the app ID