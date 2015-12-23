#Rest Test Server

Deploy the Rest Test server on a machine that has Node.js installed.

Move to the directory where resttest.js and package.json are copied.

Do a: npm install

to install dependencies.

Run the Server with:  node resttest.js

The REST API will be available at: http://mymachine_address:3003/api/test

The Sming App running on the ESP8266 will call the API and check each REST verb.
