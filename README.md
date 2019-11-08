# tank_sensor
Arduino sonic rain water sensor
Summer 2014:

I used a 32kb arduino which barely fit enough code to do the basics.  It can use wifi, check the internet for a time, and log to an SD card.  I installed it temporarily and confirmed that the log, once checked by removing the card, worked.  I also confirmed that the tank doesnt' seem to leak which is a load off.

My final production code was “tank_sensors2_logs_and_internet_time_works_v3”

Feb 2015:
I got a new “mega” card with 256kb.  Need to see if it can fit all the nice code I would like, and log to xively.  I need to string a network cable over to either hook up a wifi hotspot for under the deck or connect to a network line directly.

Then I need to see if I can pull the data off xively with my SP site.  Fun fun.  

Step one:  replace the arudino board-- most of the connections are on the wifi shield, so I could just remove the uno and put in the mega.  Changed the arduino code to point to “mega 256”.  and it works!  As well as it ever did that is.  Water level calc is crazy, probably always was.  Now to add features like xively.

A note about the udp library.  Go to arduino.com, and go to the time page, and download the .zip.  Install using the ardino program.  Then navigate to the C dir and modify in according to the instructions in the code that uses it.  

may25:  Starting to use the ethernet board and lost a bunch of time.  I used the "server" example to test the board and learned I cannot use the "192.168.1.167" IP address, my router doesn't allow that IP ADDRESS.  If i use DHCP or specify 192.168.0.167 it works fine.  I set that to be a fixed IP address on the router too so I can keep using it in the future.

Note the SD and the Ethernet cannot work at the same time so that will be tricky for reading out the file.  I will have to read out to ram, switch to ethernet, send out strings, go back to the same place on the card, read to ram etc...  there is 8k ram on this arduino so maybe not terrible.
