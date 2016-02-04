#! /bin/sh

tmp=`ps | grep 'ntpd -p' |  grep 137.138 | grep -v ps `
#tmp=`ps | grep 'ntpd -p' |  grep 10.10 | grep -v ps `
pid=`echo $tmp | awk '{print $1}'`
kill -9 $pid
ntpd -p 137.138.16.69
#ntpd -p 10.10.10.10

