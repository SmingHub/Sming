SMTP Client
===========

Purpose
-------

To show the basics of sending an email via SMTP

SMTP
----

When an IOT device detects a malfunction, it is good to be able to
notify the user. We can do that either by sending them an email or via a
service like MQTT.

This sample shows how to send an email via SMTP directly from the
ESP8266.

smtp2go conveniently provides a free account.

Create an account here: https://www.smtp2go.com/setupguide/arduino/ .

It needs some configuration: \* the name of the SMTP server, eg
“mail.smtp2go.com” \* a username and password \* the name and email of
the person from whom the email will be sent \* the name and email of the
person to send the email to

Edit the sample to replace these values and the SSID etc.
