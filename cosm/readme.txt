HttpClient is a library to make it easier to interact with web servers from Arduino.
It provides a higher level interface for interacting with web services

COSM Code: https://github.com/cosm/cosm-arduino

COSM uses this library: https://github.com/amcewen/HttpClient

Downloading is was very slow, but someone found this bug:
https://github.com/cosm/cosm-arduino/issues/12

To fix, in CosmClient::get
change
while ((http.available() || http.connected()))
to
while ((http.available() && http.connected()))
