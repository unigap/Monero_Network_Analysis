#!/bin/sh
# Monero nodoekin edukitako komunikazioa aztertu ondoren, script honek bidali zaizkigun IP helbideak fitxategi batean idatziko ditu
# IPv4 helbideak eta IPv6 ondo (baina IPv4 mapeatuak dira)


# Handshake response (1001+Q) motako mezuak dituzten komunikazio fitxategien izenak ipfiles fitxategian idatzi
grep  "Command: e9 03" em* | grep "Expect Response: 00" | cut -d ':' -f 1 > ipfiles

# Irakurri fitxategien izenak banan-banan
while read -r lerroa
do
    echo "$lerroa prozesatzen...\n"
    # IPv4 helbideak harrapatu fitxategiaren edukitik (...gero 07 portua 04)
    cat $lerroa | cut -d ' ' -f 2-17 | tr -s '\n' ' ' | grep -oE '04 61 64 64 72 0c 08 04 6d 5f 69 70 06.+[0-9a-f].*06 6d 5f 70 6f 72 74' | sed 's/04 61 64 64 72 0c 08 04 6d 5f 69 70 /\r\n/g' > iphex.dat
    # IPv6 helbideak harrapatu fitxategiaren edukitik (...gero 07 portua 04)
    cat $lerroa | cut -d ' ' -f 2-17 | tr -s '\n' ' ' | grep -oE '04 61 64 64 72 0c 08 04 61 64 64 72 0a 40.+[0-9a-f].*06 6d 5f 70 6f 72 74' | sed 's/04 61 64 64 72 0c 08 04 61 64 64 72 0a /\r\n/g' > iphex6.dat

    # IPv4 formateatu ping egin eta idatzi
    while read -r line
    do

        #echo "LINEA: "$line
        echo $line | cut -c 4-14 > iphexx
        iph0=$(cat iphexx | cut -c 1-2)
        iph1=$(cat iphexx | cut -c 4-5)
        iph2=$(cat iphexx | cut -c 7-8)
        iph3=$(cat iphexx | cut -c 10-11)
        # Hutsa bada ez egin bihurketa
        if [ -z $iph0 ];
        then
            echo "HAU EZ" > /dev/null
        else
            ip0=$(printf "%d\n" "0x$iph0")
            ip1=$(printf "%d\n" "0x$iph1")
            ip2=$(printf "%d\n" "0x$iph2")
            ip3=$(printf "%d\n" "0x$iph3")

            ip=$(printf "%d.%d.%d.%d" $ip0 $ip1 $ip2 $ip3)
            #ping -q -W 1 -c 1 $ip # ez du quiet mode egin...
            ping -W 1 -c 1 $ip 1>/dev/null 2>/dev/null
            err=$?
            if [ $err -eq 0 ];
            then
                echo "OK: "$ip # ping komandoarekin lortu da erantzuna
            else
                echo "..: "$ip # ez da erantzunik jaso ip honengandik segundu bat igaro ondoren
            fi

        fi
    done < iphex.dat


    # IPv6 formateatu ping egin eta idatzi
    while read -r lv6
    do
        if [ -z "$lv6" ];
        then
            echo "EZ DAGO" > /dev/null
        else
            lv6=$(echo $lv6 | cut -c 4-)
            l0=$(echo $lv6 | cut -c 1-2,4-5)
            l1=$(echo $lv6 | cut -c 7-8,10-11)
            l2=$(echo $lv6 | cut -c 13-14,16-17)
            l3=$(echo $lv6 | cut -c 19-20,22-23)
            l4=$(echo $lv6 | cut -c 25-26,28-29)
            l5=$(echo $lv6 | cut -c 31-32,34-35)
            l6=$(echo $lv6 | cut -c 37-38,40-41)
            l7=$(echo $lv6 | cut -c 43-44,46-47)

            ipv6=$(printf "%s:%s:%s:%s:%s:%s:%s:%s" $l0 $l1 $l2 $l3 $l4 $l5 $l6 $l7)

#            ping -W 1 -c 1 $ipv6 1>/dev/null 2>/dev/null # network is unreachable erantzuten du honek.... -6 aukerarekin ere... IPv4 mapped dira guztiak
#            err=$?
#            if [ $err -eq 0 ];
#            then
#                echo "OK: "$ipv6 # ping komandoarekin lortu da erantzuna... inoiz ez
#            else

                # IPv4 mapped izan daiteke
                map0=$(echo $l6 | cut -c 1-2)
                if [ -z $map0 ];
                then
                    echo "HUTSA" > /dev/null
                else
                    map0=$(printf "%d\n" "0x$map0")
                    map1=$(echo $l6 | cut -c 3-4)
                    map1=$(printf "%d\n" "0x$map1")
                    map2=$(echo $l7 | cut -c 1-2)
                    map2=$(printf "%d\n" "0x$map2")
                    map3=$(echo $l7 | cut -c 3-4)
                    map3=$(printf "%d\n" "0x$map3")
                    ipm=$(printf "%d.%d.%d.%d" $map0 $map1 $map2 $map3)

                    ping -W 1 -c 1 $ipm 1>/dev/null 2>/dev/null

                    err=$?
                    if [ $err -eq 0 ];
                    then
                        echo "OK: "$ipm "["$ipv6"]" # ping komandoarekin lortu da erantzuna
                    else
                        echo "..: "$ipm "["$ipv6"]" # ez da erantzunik jaso
                    fi
                fi

#            fi

        fi

    done < iphex6.dat


done < ipfiles

# Portua ez da interesekoa oraingoz
echo "\r\nProzesua bukatuta."
exit 0

