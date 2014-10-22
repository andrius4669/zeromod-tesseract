#!/bin/sh
# simple script for server compilation
# for advanced features, cd to src folder and use make

ostype=`uname -s`
case "$ostype" in
	*"Linux"*) numjobs="-j"`grep processor /proc/cpuinfo | wc -l`;;
	*"BSD"*) numjobs="-j"`sysctl -n hw.ncpu`;;
	*) numjobs='';;
esac
unset ostype

makecmd='make'
if type gmake > /dev/null; then
	makecmd='gmake'
fi

$makecmd $numjobs -C src install_server

unset numjobs
unset makecmd
