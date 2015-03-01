# Rescued by Pebble
A Pebble watchface that checks your productivity levels using the data from [RescueTime](https://rescuetime.com) and optionally gives you a "vibe" if you're not focused on your work.

![](http://0f8f28fe275e3a043777-67ab80ec00c7299bd1255995bf933a71.r1.cf2.rackcdn.com/rescued-by-pebble-explanation-full3.png)

## Description
The watchface connects to the [RescueTime](https://rescuetime.com) API every 5 minutes and get information about how productive you were in the last 5 minutes, based on the data collected by their apps. [RescueTime](https://rescuetime.com) provides the number of seconds you spent on each productivity level in the last 5 minutes. This data is transformed into a number between 0 and 100, using a weighted average and then displayed on the screen of the watch.

## Next to implement

* ~~Show when you have some quality offline time~~ - testing
* ~~Show more explicitly that you're disconnected from bluetooth or simply when you cannot retrieve the productivity data~~ - testing
* Vibration when productivity levels are low

## Version history
1.2 Fixed bugs about applying the inverted layer or not at startup, based on the settings

1.1 Fixed division by 0 bug

1.0 First release
