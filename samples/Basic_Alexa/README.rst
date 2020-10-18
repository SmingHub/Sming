Basic Alexa
===========

If you have an Echo Dot or other Amazon Alexa device, this shows how to provide simple support
by emulating a Philips Hue lighting bridge.

Pairing is permanently enabled in this sample application so all you need to do is ask Alexa to
discover devices and the lights should appear.

You can verify this is working by requesting a list of registered lights via HTTP::

   http://IP_ADDRESS/api/user/lights


Turning the ESP LED on and off can be done by sending a ``POST`` request to light 101.
The RESTED plugin for firefox is very useful for this sort of thing. The endpoint URL is::

   http://IP_ADDRESS/api/user/lights/101/state

To turn the LED ON, the body of the request would contain::

   {"on":true}

And to turn it off again::

   {"on":false}

Remember to set the ``Content-Type`` header to ``application/json``.

Here's how to do it with CURL::

   curl -X POST -H "Content-Type: application/json" -d "{on: true}" http://IP_ADDRESS/api/user/lights/101/state

