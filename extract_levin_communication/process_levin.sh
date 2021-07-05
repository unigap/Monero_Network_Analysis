#!/bin/sh
# Levin protokoloa jarraituz bidaltzen diren mezuen goiburukoa komentatu hobeto ulertu ahal izateko mezuaren esanahia; eskaera, erantzuna...
# Exekutatu: sh process_levin.sh <tcpflow_created_file> <id>
# Mezuen informazio osatua (hexdump-ekin) + Goiburukoak komentatuta
# em$<id> fitxategian idatzi (em1, em2, em3...)
# Komandoen zenbakiak ateratzen dituen scriptari deitzen dio (sh command_conv.sh em<id> > comm_em<id> )
# Goiburukoen informazio esanguratsuena idazten du comm_em1 bezalako fitxategietan
# Monero sareari konektatuta egonda, hasi.sh scriptarekin hasieratu daiteke iristen diren mezu guztien harrapaketa (tcpflow 18080 portuan) eta ondoren script honi deitzen zaio paketeen goiburukoak komentatzeko

# 2 argumentu: komentatu nahi den sarrera fitxategia eta irteera fitxategiaren identifikadorea
if [ $# -gt 1 ];
then
    file=$1
    outp=em$2
else
    echo "Arguments: filename and id"
    exit 1
fi

# Lortu hamasaitarrez fitxategiaren edukia, levin protokoloaren header-ak lortu eta non dauden (byte offset, extended regular expression, only matching)
#headers=$(hexdump -C $file | cut -d ' ' -f 3-19 | tr -s '\n' ' ' |  grep -bEo "01 21 01 01 01 01 01 01.{0,75}")
pos=$(hexdump -C $file | cut -d ' ' -f 3-19 | tr -s '\n' ' ' |  grep -bEo "01 21 01 01 01 01 01 01.{0,75}" | cut -d ':' -f 1)
file2="ir0"
hexdump -C $file > $file2 # ir0 fitxategian lortu hexdump-en irteera osoa testua prozesatzeko (hamaseitarrez eta ASCII)


lenpart=0
iter=0
berria=0
segi=0
faltan=0
junta=0
buk=0
lin=1
echo "" > $outp # Emaitza fitxategia sortu

while read -r lerroa;
do

     if [ $buk -eq 0 ]; # header berria aztertu beharra
     then
        line=$(echo $pos | cut -d ' ' -f $lin)
        line=$((line / 48)) # Lerro zenb: posizio absolutua zati lerro bakoitzean dauden karaktere kopurua
        next=$((line+1))
        next2=$((line+2)) # header bakoitza 3 lerrotan egongo da

        offs=$(echo $pos | cut -d ' ' -f $lin)
        offs=$((offs % 48)) # Lerroan posizioa (offset)

        buk=1          # aztertu header-a
        lin=$((lin+1)) # zenbatgarren header
        #echo $line $offs
     fi

     printf "%s " $lerroa >> $outp # lerroa zegoen bezala idatzi irteerako fitxategian

     if [ -z $line ];
     then
         line=0
     fi


     if [ $iter -gt 0 ];
     then
         junta=0
         if [ $berria -eq 1 ];
         then
             berria=0
             if [ $offs -lt 24 ]; # length zati bat aurreko lerroan
             then
                 lenzar=$(echo $aurreko | cut -c $((offs+33))- | cut -d '|' -f 1)
                 lenber=$(printf "$lenzar " && echo $lerroa  | cut -c 10- )
             else
                 lenber=$(echo $lerroa | cut -c $((10+offs-24))-$((offs+8)))

             fi
             printf "# Signature: 01 21 01 01 01 01 01 01 (8 bytes), Length: $lenber (8 bytes), " >> $outp
             junta=1
         fi
     fi

     # lerroa header baten hasiera bada.   LEVIN: (signature*, length*)
     if [ $line -eq $iter ];
     then
        aiter=0
        info=$(echo $lerroa | grep -Pzob "01 21 01 01 01 01 01 01(.*\n)*")
        lenpos=$(echo $info | cut -d ':' -f 1)
        #echo $lenpos
        info2=$(echo $info | cut -d ':' -f 2)
        lenpos=$((lenpos+16)) # sign. hasi baino 16 karaktere geroago hasiko da length parametroa

        # sign and length in one line
        if [ $offs -eq 0 ]; # hasieran hasten da sign. (length osoa lerroan)
        then
            len=$(echo $info2 | cut -c $lenpos-$((lenpos+23)))
            #echo $len
            echo "# Signature: 01 21 01 01 01 01 01 01 (8 bytes), Length: $len (8 bytes)" >> $outp # lerroa komentatu

        # sign and part of length in one line
        elif [ $offs -lt 24 ];
        then
            lenpos=$((lenpos-offs))
            lenpart=$(echo $info2 | cut -c $lenpos-$((lenpos+22-offs)) | cut -d '|' -f 1)
            lenlen=$(echo $lenpart | awk '{print NF}')
            falta=$((8*3-$lenlen*3)) # length-ri falta zaizkion karaktere kop.
            segi=2 # length osatu hurrengo lerroarekin
            echo "# Signature: 01 21 01 01 01 01 01 01 (8 bytes) " >> $outp # lerroa komentatu

        # only sign is in one line
        elif [ $offs -eq 24 ];
        then
            falta=24
            lenpart="0"
            segi=2
            echo "# Signature: 01 21 01 01 01 01 01 01 (8 bytes) " >> $outp # lerroa komentatu

        # only part of sign
        elif [ $offs -gt 24 ];
        then
            echo "" >> $outp
            segi=1 # signature osatu hurrengo lerroarekin
        fi


     # header-aren erdiko lerroa bada.   LEVIN: (signature*, length*, e-resp, command*, return-code*, reserved*)
     elif [ $next -eq $iter ];
     then
         comd=$(echo $lerroa | cut -d '|' -f 1 | cut -c $((offs+13))-$((offs+24)) || echo 0)
         rcod=$(echo $lerroa | cut -d '|' -f 1 | cut -c $((offs+25))-$((offs+36)) || echo 0)
         resv=$(echo $lerroa | cut -d '|' -f 1 | cut -c $((offs+37))-$((offs+48)) || echo 0)
         #echo $comd % $rcod % $resv ============== $offs
         #echo " >>>>>>>>>>>>>>> $lerroa"
         if [ $segi -eq 1 ];
         then
             len=$(echo $lerroa | cut -c $((offs-23+9))-$((offs+9)))
             exresp=$(echo $lerroa | cut -c $((offs+10))-$((offs+11)))
             if [ $offs -lt 34 ];
             then
                 echo "# Signature: 01 21 01 01 01 01 01 01 (8 bytes), Length: $len (8 bytes), Expect Response: $exresp (1 byte), Command: $comd (4 bytes)" >> $outp
             else
                 # $comd osatzeke
                 echo "# Signature: 01 21 01 01 01 01 01 01 (8 bytes), Length: $len (8 bytes), Expect Response: $exresp (1 byte)" >> $outp
             fi
             #echo $offs
             #echo $lerroa
             segi=0
         elif [ $segi -eq 2 ];
         then
             lenbuk=$(echo $lerroa | cut -c 10-$((10+falta-2)))
             eresp=$(echo $lerroa | cut -c $((10+falta))-$((11+falta)))
             if [ X"$lenpart" = X"0" ];
             then
                 if [ $offs -lt 22 ];
                 then
                     echo "# Length: $lenbuk (8 bytes), Expect Response: $eresp (1 byte), Command: $comd (4 bytes), Return Code: $rcod (4 bytes)" >> $outp
                 else
                     echo "# Length: $lenbuk (8 bytes), Expect Response: $eresp (1 byte), Command: $comd (4 bytes)" >> $outp
                 fi
             else
                 if [ $offs -lt 22 ];
                 then if [ $offs -lt 10 ];
                      then
                          echo "# Length: $lenpart $lenbuk (8 bytes), Expect Response: $eresp (1 byte), Command: $comd (4 bytes), Return Code: $rcod (4 bytes), Reserved: $resv (4 bytes)" >> $outp
                      else
                          echo "# Length: $lenpart $lenbuk (8 bytes), Expect Response: $eresp (1 byte), Command: $comd (4 bytes), Return Code: $rcod (4 bytes)" >> $outp
                      fi
                 else
                     echo "# Length: $lenpart $lenbuk (8 bytes), Expect Response: $eresp (1 byte), Command: $comd (4 bytes)" >> $outp
                 fi
             fi
             segi=0
         else
             eres=$(echo $lerroa | cut -c 10-11)
             if [ $junta -eq 0 ];
             then
                 echo "# Expect Response: $eres (1 byte), Command: $comd (4 bytes), Return Code: $rcod (4 bytes), Reserved: $resv (4 bytes)" >> $outp
             else
                 eres=$(echo $lerroa | cut -c $((offs+10))-$((offs+11)))
                 echo "Expect Response: $eres (1 byte), Command: $comd (4 bytes), Return Code: $rcod (4 bytes), Reserved: $resv (4 bytes)" >> $outp
                 junta=0
             fi
         fi


     # header-aren azken lerroa (command*, return-code*, reserved*, end)
     elif [ $next2 -eq $iter ];
     then
         flag=$(echo $resv | cut -c 1-2 || echo 3)

         if [ $offs -gt 9 ];
         then
             resv2=$(echo $lerroa | cut -d '|' -f 1 | cut -c $((offs-11))-$((offs-1)) | sed -e s'/[A-Fa-f0-9]\{5,\}//g' | cut -f 2- || echo Z)
             if [ $offs -ge 21 ];
             then
                 flag=$(echo $resv2 | cut -c 1-2 || echo 3)
             elif [ $offs -eq 18 ];
             then
                 resv2=$(echo $resv2 | cut -c 3- )
             fi

             if [ $offs -gt 21 ];
             then
                 rcod2=$(echo $lerroa | cut -d '|' -f 1 | cut -c $((offs-23))-$((offs-13)) |  sed -e s'/[A-Fa-f0-9]\{5,\}//g' | cut -f 2- || echo Z)

                 if [ $offs -gt 33 ];
                 then
                     comd2=$(echo $lerroa | cut -d '|' -f 1 | cut -c 10-$((offs-25)) || echo Z)
                     echo "# Command: $comd$comd2 (4 bytes), Return Code: $rcod2 (4 bytes), Reserved: $resv2 (4 bytes), Ending chars: 01 00 00 00 (4 bytes).   Flag=$flag" >> $outp
                 else
                     echo "# Return Code: $rcod$rcod2 (4 bytes), Reserved: $resv2 (4 bytes), Ending chars: 01 00 00 00 (4 bytes).   Flag=$flag" >> $outp
                 fi
             else
                 echo "# Reserved: $resv$resv2 (4 bytes), Ending chars: 01 00 00 00 (4 bytes).   Flag=$flag" >> $outp
             fi
         else
             echo "# Ending chars: 01 00 00 00 (4 bytes).   Flag=$flag" >> $outp
         fi
         #echo "" >> $outp
         buk=0 # header berri bat aztertzera
         hurr=$(echo $pos | cut -d ' ' -f $lin)
         hurr=$((hurr / 48 ))
         if [ $hurr -eq $iter ];
         then
             berria=1
         fi


     # beste edozein lerro bukatu (newline)
     else
         echo "" >> $outp
     fi
     aurrekoa=$(echo $lerroa)
     iter=$((iter+1))


done < $file2

#echo "$outp fitxategiaren goiburukoak eta komandoak: \n" 

#cat $outp | grep -P "Signature|Length|Expect Response|Command|Return Code|Ending chars|Flag"
#cat $outp | grep -P "Signature|Length|Expect Response|Command|Return Code|Ending chars|Flag -A 1 -B 1"
#echo '\n --->>> cat em0 | grep -P "Signature|Length|Expect Response|Command|Return Code|Ending chars|Flag -A 1 -B 1"'

# Komandoen zenbakiak bihurtu fitxategien bukaeran, komentatutako goiburukoak ondoren
sh command_conv.sh $outp > comm_$outp 2> /dev/null
sed -i 's/Command number: 0//g' comm_$outp
#cat comm_$outp
#echo "\n====================================================================================================================\n\n"
