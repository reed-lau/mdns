# mdns
my dns used to report the raspberrypi ip


add the following command to `/etc/rc.local` to report the ip at 1Hz.
```sh
/home/pi/mdns/mdns 1>/dev/null 2>/dev/null &
```
