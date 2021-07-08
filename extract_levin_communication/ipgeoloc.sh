#!/bin/sh
# Script honek ip_conv.sh scriptak sortutako irteerako fitxategian dauden IP helbideen kokapenaren xehetasunak lortzen ditu
# Zehaztasuna arazoa izan daitekeenez, 6 zerbitzu desberdinekin lortu dira datu horiek (doan baina 2 kontu sortu dira #1 eta #5 moduetarako)
# $ sh geoloc.sh ipeak
# Argumentu bezala fitxategiaren izena pasa behar da
# 1. moduan 50k eskaera astean erabilgarri (API key beharrezkoa)
# 5. moduan webgunearen autentifikazioa behar da
# 6. moduan datu-basea deskargatu behar da, inplementazioan agertzen den bezalaxe eta geoip karpetan deskonprimitu
# Irteera modu bakoitzerako: <IPv4>\t Country:<country>, City:<city>, Latitude:<latitude>, Longitude:<longitude>


# Argumentu bat duela ziurtatu
if [ $# -eq 1 ];
then
    file=$1
else
    echo "Sarrerako fitxategia beharrezkoa da"
fi

exec 2>/dev/null

# IP bakoitza lerro batean irakurri
while read -r line
do
    ip=$(echo $line | cut -d ' ' -f 2) # gainerako karaktereak baztertu
    echo $ip | grep ^[0-9] > /dev/null
    if [ $? -eq 0 ]; # IPv4 guztiak zenbaki batekin hasten dira
    then

        # 1. modua (orain API key-arekin)
#        resp=$(curl https://ipinfo.io/$ip | jq '.country, .city, .loc') # null: rate limit exceeded

        # REPLACE {ACCOUNT ID} & {API KEY}
        #resp=$(curl https://ipinfo.io/$ip?token={TOKEN} | jq '.country, .city, .loc') # REPLACE {TOKEN}
        #cou=$(echo $resp | cut -d '"' -f 2)
        #cit=$(echo $resp | cut -d '"' -f 4)
        #lat=$(echo $resp | cut -d '"' -f 6 | cut -d ',' -f 1)
        #lon=$(echo $resp | cut -d '"' -f 6 | cut -d ',' -f 2)
        #echo $ip"\t" Country:$cou, City:$cit, Latitude:$lat, Longitude:$lon       # Informazio esanguratsua idatzi


        # 2. modua (batzuetan city hutsik)
        resp=$(curl https://freegeoip.app/json/$ip) # json formatuan lortu ip horren kokapenaren xehetasun batzuk
        cou=$(echo $resp | cut -d ':' -f 3  | cut -d ',' -f 1 | cut -d '"' -f 2)  # Herrialdearen izena
        cit=$(echo $resp | cut -d ':' -f 7  | cut -d ',' -f 1 | cut -d '"' -f 2)  # Hiria baldin badago
        lat=$(echo $resp | cut -d ':' -f 10 | cut -d ',' -f 1) # Latitudea
        lon=$(echo $resp | cut -d ':' -f 11 | cut -d ',' -f 1) # Longitudea
        echo $ip"\t" Country:$cou, City:$cit, Latitude:$lat, Longitude:$lon       # Informazio esanguratsua idatzi


        # 3. modua (motelagoa)
        resp=$(curl https://json.geoiplookup.io/$ip | jq ' .country_code, .city, .latitude, .longitude, .org') # org interesekoa izan daiteke
        cou=$(echo $resp | cut -d '"' -f 2)
        cit=$(echo $resp | cut -d '"' -f 4)
        lat=$(echo $resp | cut -d '"' -f 5 | cut -d ' ' -f 2)
        lon=$(echo $resp | cut -d '"' -f 5 | cut -d ' ' -f 3)
        org=$(echo $resp | cut -d '"' -f 6)
        echo $ip"\t" Country:$cou, City:$cit, Latitude:$lat, Longitude:$lon       # Informazio esanguratsua idatzi


        # 4. modua (xml formatuan batzuetan city hutsik)
        resp=$(curl http://api.geoiplookup.net/?query=$ip)
        cou=$(echo $resp | grep -oE '<countrycode>.+.*</countrycode>' | sed 's/<countrycode>//g' | sed 's/<\/countrycode>//g')
        cit=$(echo $resp | grep -oE '<city>.+.*</city>' | sed 's/<city>//g' | sed 's/<\/city>//g')
        lat=$(echo $resp | grep -oE '<latitude>.+.*</latitude>' | sed 's/<latitude>//g' | sed 's/<\/latitude>//g')
        lon=$(echo $resp | grep -oE '<longitude>.+.*</longitude>' | sed 's/<longitude>//g' | sed 's/<\/longitude>//g')
        echo $ip"\t" Country:$cou, City:$cit, Latitude:$lat, Longitude:$lon       # Informazio esanguratsua idatzi


        # 5. modua (geoip doan: geolite API key) agian  city null... https://www.maxmind.com
        # REPLACE {ACCOUNT ID} & {API KEY}
        #resp=$(curl -u {ACCOUNT ID}:{API KEY} https://geolite.info/geoip/v2.1/city/$ip | jq '.country.iso_code, .location.latitude, .location.longitude, .city.names.en') # REPLACE {ACCOUNT ID} & {API KEY}
        #cou=$(echo $resp | cut -d '"' -f 2)
        #cit=$(echo $resp | cut -d '"' -f 4)
        #lat=$(echo $resp | cut -d ' ' -f 2)
        #lon=$(echo $resp | cut -d ' ' -f 3)
        #echo $ip"\t" Country:$cou, City:$cit, Latitude:$lat, Longitude:$lon       # Informazio esanguratsua idatzi


        # 6. modua (maxmind-ren GeoLite2-City databasea .dat formatura pasata: https://www.miyuru.lk/geoiplegacy) hau ez dago aurrekoa bezain eguneratuta... modu honek ez du sarerako konexiorik behar
        resp=$(geoiplookup -f ../src/geoip/maxmind4.dat $ip) # Datu-basearen path-arekin lortu kokapena (wget https://dl.miyuru.lk/geoip/maxmind/city/maxmind4.dat.gz)
        cou=$(echo $resp | cut -d ',' -f 2 | cut -d ':' -f 2 | cut -d ' ' -f 2)
        cit=$(echo $resp | cut -d ',' -f 5 | cut -d ' ' -f 2-)
        lat=$(echo $resp | cut -d ',' -f 7 | cut -d ' ' -f 2)
        lon=$(echo $resp | cut -d ',' -f 8 | cut -d ' ' -f 2)
        echo $ip"\t" Country:$cou, City:$cit, Latitude:$lat, Longitude:$lon"\r\n" # Informazio esanguratsua idatzi


#        geoiplookup -f /usr/share/GeoIP/GeoLiteCity.dat $ip      # datu-base hau ez dago eskuragarri... https://blog.maxmind.com/2019/12/18/significant-changes-to-accessing-and-using-geolite2-databases/


    fi

done < $file

