#!/bin/sh

rm -f GeoIP.dat.gz
wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz
rm -f GeoIP.dat
gunzip GeoIP.dat.gz

rm -f GeoLiteCity.dat.gz
wget http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz
rm -f GeoLiteCity.dat
gunzip GeoLiteCity.dat.gz
