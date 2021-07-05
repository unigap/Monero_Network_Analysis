#!/bin/sh
# Monero sareari konektatuta egonda hau exekutatzeko:
# $ sh hasi.sh <tcpflow_denb> [<interfazea>]
# Monero-ko nodoekin dauden komunikazioen trafikoa harrapatu (1. argumentuan adierazitako segundu kopurua)
# Defektuz tcpflow-k detektatzen duen interfazea ordez beste bat erabili nahi bada adierazi 2. argumentuan interfazearen izena
# Ondoren process_levin.sh script-ari deitu levin protokoloaren goiburukoak komentatuta ikusteko em<i> fitxategietan
# Azken honek command_conv.sh script-a deituko du eta komando zenbakiak hamartarrez ikusi ahalko dira comm_em<i> fitxategietan
# IP helbideak lortzeko handshake response mezuak aztertuko dira ip_conv.sh script-ean eta ipak fitxategian idatziko dira
# Azkenik IP helbide horien mundu mailako kokapena lortzeko geoloc.sh script-ari deitzen zaio, iplocs fitxategian 6 modu desberdinetan IP bakoitzaren kokapen-xehetasunak idatziko dituena
# Prozesua hasteko modu pribilegiatua beharrezkoa da
# Post-prozesua: $ grep -E "Command number:" comm_em* | cut -d ':' -f 3 | sort | uniq  # Bidali diren komando desberdinak idatzi


# Argumentuak jaso (1 edo 2)
arg=0
if [ $# -gt 1 ];
then
    int=$2 # Zein interfazean entzun behar duen jakinarazi (defektuz tcpflow-k wi-fi interfazea)
    denb=$1
    arg=1
elif [ $# -eq 0 ];
then
    echo "Trafikoa harrapatzen egon beharreko segundu kopurua adierazi behar da. \nBeharrezkoa izan daiteke interfaze izena adieraztea bigarren argumentu bezala"
    exit 1
else
    denb=$1
fi

ter=0

# Entzuten jarri 18080 portuan (automatikoki fitxategiak sortu <srce@IP:Port>-<dest@IP:Port> formatuarekin)
if [ $arg -eq 1 ]; # interfazea adierazita
then
#    sudo tcpflow -a -i $int port 18080 &
    sudo tcpflow -a -i $int port 18080 &
    sleep 2

    ps "$!" > logps # aztertu aktibo dagoen tcpflow: atzeko planoan exekutatu den azken prozesuaren id pasaz
    ter=$?  # 0 itzuliko du tcpflow aktibo baldin badago; bestela exit 1 (interfaze okerra)

    if [ $ter -gt 0 ];
    then
        echo "Interfaze egokia aukeratu:"
        ip address
        exit 1
    fi
else # defektuz
    sudo tcpflow -a port 18080 &
fi

# Argumentu bezala pasatako segundu kopurua itxaron
sleep $denb

# Denbora hori pasatakoan akatu tcpflow prozesua (SIGKILL)
pid=$(ps -aux | grep "tcpflow" | grep -v "grep" | awk '{print $2}')
echo "Terminating tcpflow process" # $pid"
sudo kill -9 $pid

# tcpflow komandoak sortutako fitxategien izenak eskuratu
find -mtime -1 | grep -E '(^|[^0-9])[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]*-[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}' > conn_files
#ls | grep -E '(^|[^0-9])[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]*-[0-9]{3}.[0-9]{3}.[0-9]{3}.[0-9]{3}' > conn_files

iter=1
echo "Processing monero packets... Output: em<i> & comm_em<i> "
while read -r line
do
     # em$iter eta comm_em$iter fitxategietan idatzi komunikazioen informazioa hamasaitarrez eta ASCII kodeketarekin, protokoloaren header-ak komentatuta eta komando zenbakiak
     sh process_levin.sh "$line" $iter
     iter=$((iter+1))
done < conn_files

# Bidaltzen diren IP helbideak harrapatu eta konektatuta dauden idatzi
echo "Extracting IPs from data... Output: ipak "
sh ip_conv.sh > ipak
cat ipak | sort | uniq > ipeak # Kontserbatu behin soilik (ordenatu soilik horretarako... lehenik erantzunik gabekoak '..' gero 'OK' eta gainera ordena: 255 < 5)

# IP helbideen kokapena geoip tresnarekin lortu
echo "Getting geolocation of IPs... Output: iplocs "
sh geoloc.sh ipeak > iplocs

exit 0

