# uwsgi-alarm-socket
uWSGI simple plugin for monitoring sockets

Just list the addresses (TCP or UNIX) to monitor, and a uWSGI alarm will be triggered whenever the connections to them fails.

Installation
============

The plugin is 2.x frienfly:

```sh
uwsgi --build-plugin https://github.com/unbit/uwsgi-alarm-socket
```

Usage
=====

Use the ```--alarm-socket <alarm> <socket>``` option to trigger a monitor

```ini
[uwsgi]
; load the alarm_socket plugin
plugin = alarm_socket

; define an alarm named 'danger' simply printing a line in the log
alarm = danger log:

; monitor a tcp socket (a postgres one) and trigger the danger alarm on error
alarm-socket = danger 127.0.0.1:5432
; monitor a unix socket too
alarm-socket = danger /var/run/foo.socket

...
```
