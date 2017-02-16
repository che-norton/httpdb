#!/bin/sh

export LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib:/tmp/asus
export PATH=/koolshare/bin:/sbin:/bin:/usr/sbin:/usr/bin
LD_PRELOAD="hook.so" httpd -S -E /usr/sbin/ca.pem /usr/sbin/httpsd.pem
#LD_PRELOAD="hook.so" httpdb -h
