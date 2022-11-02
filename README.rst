This repo contains Python libraries and programs for reading sdvx exceed gear virtual memory data to get certain in-game values. 

TODO
####
I plan searching for and adding more values to the program plus cleaning up the code a bit.

How to use
##########
* memory folder contains library for reading virtual memory data with fair ease
* sdvx.py implements the memory library specifically for Sound Voltex EXCEED GEAR
* the discordipc library is used for communicating with the discord ipc on Windows using pipes. I plan on implementing much more capability to that library and releasing it separate from this repo.
* rich_presence.py implements sdvx.py and the discordipc library to create a rich presence that displays what song you're playing. I would like to display more info, but first I have to find the memory offsets so that I can actually get that data. If you want an exe version of this program then you can get it here: https://github.com/Sheepposu/sdvx_memory_reader/releases/tag/rp-v1.0.0
