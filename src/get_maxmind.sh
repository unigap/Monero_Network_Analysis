#!/bin/sh
# Script honekin uneko direktorioan 'geoip' karpeta sortuko da eta barnean maxmind-en datu-basea testu formatuan deskargatuko da, IP-en koordenatuak atera ahal izateko

mkdir -p geoip
cd geoip
wget https://dl.miyuru.lk/geoip/maxmind/city/maxmind4.dat.gz
gzip -d maxmind4.dat.gz  # | echo "y" # overwrite file if exists
rm $(ls | grep "maxmind4.dat.gz*") # remove compressed file
