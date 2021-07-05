#!/bin/sh
# nagusia exekuzioaren pid-a lortu eta SIGINT seinalea bidali (Keyboard Interrupt), bestela SIGTERM (-15) bidali
# Exekuzioa bukatzeko, logbzb fitxategian lortutako nodoen informazioa idatziz eta 'q' eragiketa pantailaratzen mapa modu egokian ixteko (azken honek test.svg irudia sortuko du 'q' irakurtzean uneko mapa irudi bihurtuz)

pidn=$(ps -aux | grep "main" | grep -v "grep" | awk '{print $2}')
#echo $pidn
kill -2 $pidn || kill $pidn

