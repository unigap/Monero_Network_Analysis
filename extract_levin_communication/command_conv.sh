#!/bin/sh
# Script honi process_levin.sh script-etik deitzen zaio, mezuen komandoen zenbakia jakin ahal izateko
# Argumentu bezala levin goiburukoak komentatuta dituen fitxategia pasa behar zaio (em<id> modukoa)
# Script honen irteera levin mezuen hasiera (komentatuta) eta komandoen bihurketa (hamartarra) izango da
# Exekutatzeko: $ sh command_conv.sh <filename>



if [ $# -gt 0 ];
then
    arg=$1
else
    echo "Fitxategia behar da"
    exit 1
fi

cmd1=$(cat $arg | grep -o Command:.* | cut -c 13-14  )
cmd2=$(cat $arg | grep -o Command:.* | cut -c 10-11  )

printf " %s \r\n" $cmd1 > file1
printf "%s \r\n" $cmd2 > file2

iter=1
cmds=0
> file # file hutsa sortu
while read -r lerroa;
do
    cmd11=$(cat file1 | head -"$iter" | tail +"$iter" | cut -c 2-3 )
    printf "0x%s%s" "$cmd11" "$lerroa" >> file
    iter=$((iter+1))

done < file2

cmds=$(cat file)
#echo "$cmds"
sed -i '$ s/.$//' file


cat $arg | grep -E "Command: .*" -A 10 -B 3

printf "Command number: %d \n" $cmds
# Azkenekoa komando ezezaguna (0) ignoratu
