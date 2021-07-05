from __future__ import unicode_literals
import time
import sys
import os
import numpy as np
import pandas as pd
#from matplotlib import font_manager as fm, rcParams
import matplotlib #
import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap
from itertools import chain
import socket
import struct
import ipaddress
from typing import NamedTuple


#!#! Execute: python locate.py [0-1] # if we want a lower resolution of the map, pass argument between 0 and 1 (float)
# We combine the execution of the main program with this program (pipe)
# ./main 212.83.175.67 18080 10 | python locate.py
# ./main 212.83.175.67 18080 10 | python locate.py 0.3


# Data-structure definition
class Coords(NamedTuple):
    lon: float
    lat: float

class MyUbic(NamedTuple):
    coord: Coords
    cit: str
    ips: list
    tkop: list


# Draw map (init) method
def draw_map(m, scale=0.1):
    # draw a shaded-relief image
#    m.shadedrelief(scale=scale)

    # lats and longs are returned as a dictionary
    lats = m.drawparallels(np.linspace(-90, 90, 13), labels=[1,0,0,0])
    lons = m.drawmeridians(np.linspace(-180, 180, 13), labels=[0,0,0,1])

    # keys contain the plt.Line2D instances
    lat_lines = chain(*(tup[1][0] for tup in lats.items()))
    lon_lines = chain(*(tup[1][0] for tup in lons.items()))
    all_lines = chain(lat_lines, lon_lines)

    # cycle through these lines and set the desired style
    for line in all_lines:
#        line.set(linestyle='-', alpha=0.2, color='w')
        line.set(linestyle=':', alpha=0.2, color='w')


# Method to remove point from map and structure
def remove_point(x, y):
    for rp in points:
        if(rp.get_data()[0] == x and rp.get_data()[1] == y):
            try:
                rp.set_data([0],[0])
                rp.remove()
            except:
                print("E5\tError removing point.")
                pass
            break

    iter = 0
    for scc in sc:
        rkoord = scc.get_offsets()[0]
        if(rkoord[0] == x and rkoord[1] == y):
            scc.remove()
            sc.pop(iter)
            break
        iter = iter + 1


# Method to draw a node on map
def draw_point(lonf, latf):
    if(lonf > 180.0 or lonf < -180.0 or latf > 90.0 or latf < -90.0):
        print("E4\tLongitude between -180.0 and 180.0 only.\n \tLatitude between -90.0 and 90.0 only.")
    else:
        #lox, lay = m(lonf, latf)
        if(lonf == 0 and latf == 0):
            m.plot(lonf, latf, markerfacecolor='blue', markeredgecolor='black', marker='o', markersize=1.7, alpha=0.5)

        else:
            points.extend( # gorde puntuak (Line2D objektuak) ondoren ezabatzeko
                m.plot(lonf, latf, markerfacecolor='red', markeredgecolor='black', marker='o', markersize=2.7, alpha=0.5)
            )

        fig.canvas.draw()
        fig.canvas.flush_events()

# Find that coordinate if is on map
# else return -1
def find(koord):
    ind = 0
    for i in items:
        if koord == i.coord:
            return ind
        ind = ind + 1
    return -1


# not used (string always)
def ip2int(addr):
    return struct.unpack("!I", socket.inet_aton(addr))[0]

def int2ip(addr):
    return socket.inet_ntoa(struct.pack("!I", addr))
#

# Map labels for each location (hover) showing nodes ip and transacts
def update_annot(ind,a):
    pos = sc[a].get_offsets()[ind["ind"][0]]
    annot.xy = pos
    pp = Coords(pos[0],pos[1])
    ppos = find(pp)
    text = items[ppos].cit
    text2 = ""
    if len(items[ppos].ips) > 20:
        trs = items[ppos].tkop[:20]
        text2 = "\n"+str(len(items[ppos].ips)-20)+" more nodes..."
    else:
        trs = items[ppos].tkop

    ids = '\n'.join(trs)
    text = text +"\n"+ids+text2
    annot.set_text(text)
    annot.get_bbox_patch().set_facecolor('c')
    annot.get_bbox_patch().set_alpha(0.3)

# Mouse hover event listener
def hover(event):
    a = 0
    aurk = 0
    for ssc in sc:
        if aurk == 1:
            break
        vis = annot.get_visible()
        if event.inaxes == ax:
            cont, ind = ssc.contains(event)
            if cont:
                aurk = 1
                update_annot(ind,a)
                annot.set_visible(True)
                fig.canvas.draw_idle()
            else:
                if vis:
#                    aurk = 1
                    annot.set_visible(False)
                    fig.canvas.draw_idle()
        a = a+1





### main exec ###

plt.ion()
fig = plt.figure(figsize=(13, 7), edgecolor='w')
a = 0
ax = fig.add_subplot(111)
ax.set_title("Monero nodes around the world")
plt.rc('axes', unicode_minus=False)

if len(sys.argv) < 2 or type(inNumber) != float :

    # create default map
    m = Basemap(projection='cyl', resolution=None,
                llcrnrlat=-90, urcrnrlat=90,
                llcrnrlon=-180, urcrnrlon=180, ax=ax)
    draw_map(m)

    #m.bluemarble(scale=0.1)
    #m.bluemarble(scale=0.5)
    m.bluemarble()

else:
    # create map with scale arg
    m = Basemap(projection='cyl', resolution=None,
                llcrnrlat=-90, urcrnrlat=90,
                llcrnrlon=-180, urcrnrlon=180, ax=ax)
    draw_map(m)

    m.bluemarble(scale=float(sys.argv[1]))

#m = Basemap(projection='mill', resolution=None,
#            lat_0=0, lon_0=0)
#m.drawstates(color='b')
#m.drawcountries()

#m.drawmeridians(np.arange(m.lonmin,m.lonmax+15,30),size=7,labels=[0,0,0,1],color='gray',dashes=(1,4))
#parallels = np.arange(m.latmin,m.latmax+15,30)
#m.drawparallels(parallels,size=7,labels=[1,0,0,0],color='gray',dashes=(1,4))
#m.drawcountries()

#m = Basemap(projection='merc', resolution=None,
#            llcrnrlat=-90, urcrnrlat=90,
#            llcrnrlon=-180, urcrnrlon=180, ax=ax)

#m.drawcountries()
#m.drawmeridians(np.arange(m.lonmin,m.lonmax+30,60),labels=[0,0,0,1])
#m.bluemarble()


sc = []
points = [] # store nodes on map
items = []  # info
iter = 0    # count nodes on map
remo = 0    # count removed nodes
trko = 0    # count transactions
print("Reading coordinates...")

annot = ax.annotate("", xy=(0,0), xytext=(14,-25),textcoords="offset points",size=8,
                    bbox=dict(boxstyle="round", fc="w"),
                    arrowprops=dict(arrowstyle="->"))
annot.set_visible(False)
fig.canvas.mpl_connect("motion_notify_event", hover)
err = 0

print("<Method> <Longitude> <Latitude> <IP> <City/Tr>:") # input format
# Koordenatuak sarrera estandarretik irakurri begizta infinitu batean (lerroz lerro irakurriz, 'a <long> <lat>' puntua gehitzeko, 'r <long> <lat>' puntua ezabatzeko eta 'q' exekuzioa bukatzeko)
while True:
    try:
        args = input("")
        erag,valo,vala,ip,cit = args.split()
        if(erag == 'q' ): # quit (terminate execution)
            break
        lonf = float(valo)
        latf = float(vala)
        if(lonf > 180.0 or lonf < -180.0 or latf > 90.0 or latf < -90.0):
            err = 1
            print("E1\tLongitude between -180.0 and 180.0 only.\n \tLatitude between -90.0 and 90.0 only. ")
            continue

        elif(erag == 'a'): # puntua mapan gehitu
            berri = Coords(lonf,latf)
            index = find(berri)
            city = cit.replace('_', ' ')
            if lonf == 9.490900 and latf == 51.299301:
                city="Kassel"
            if lonf == 4.899500 and latf == 52.382401:
                city="Amsterdam"
            if lonf == 2.338700 and latf == 48.858200:
                city="Paris"
            if lonf == 21.225700 and latf == 45.753700:
                city="Timisoara"
            if lonf == 14.586400 and latf == 50.679600:
                city="Ceska Lipa"
            if lonf == 49.169300 and latf == 55.767502:
                city="Kazan"
            if lonf == 14.510600 and latf == 49.019299:
                city="Ceske Budejovice"
            if lonf == 19.141500 and latf == 50.211700:
                city="Myslowice"
            if lonf == 14.412200 and latf == 48.933800:
                city="Ceske Budejovice"
            if lonf == -97.821999 and latf == 37.750999:
                city="(Cheney Reservoir)"

            if index == -1: # sortu puntu berria
                it = MyUbic(berri, city, [ip], [ip+' '+str(0)])
                items.append(it)
                s = plt.scatter(lonf,latf,c='r',alpha=0.02, s=17)
            else:           # txertatu ip bat puntuan
                items[index].ips.append(ip) # koordenatu horretan gehitu ip-a
                items[index].tkop.append(ip+' '+str(0)) # nodo horri hasieratu tkop
                s = plt.scatter(lonf,latf,c='r',alpha=0.03, s=len(items[index].ips)/2+17)
            draw_point(lonf, latf)
            sc.append(s)
            iter = iter + 1 # kontatu

        elif(erag == 'r'): # puntua mapatik ezabatu
            remove_point(lonf, latf)
            ri = find(Coords(lonf,latf))
            rin = 0
            for ipak in items[ri].ips:
                if ipak == ip:
                    items[ri].ips.pop(rin)
                    items[ri].tkop.pop(rin)
                    if len(items[ri].ips) == 0:
                        items.pop(ri)
                    break
                rin = rin + 1
            remo = remo + 1 # kontatu

        elif(erag == 't'): # transakzioak gehitu
            rt = find(Coords(lonf,latf))
            rtn = 0
            kop = 0
            if trko == 0:
                print("Receiving transactions...")
            for ipak in items[rt].ips:
                if ipak == ip:
                    tip,kop = items[rt].tkop[rtn].split(" ")
                    items[rt].tkop[rtn] = tip+' '+str(int(kop) + int(cit)) # jasotako transakzio kopurua aurreko balioari inkrementatu
                    trko = trko + int(cit) # gehitu transakzio kopuru totala
                    print("Transactions received: ", cit,"   (", items[rt].tkop[rtn], ") ")
                    break
                rtn = rtn + 1
            draw_point(0.0, 0.0)
            s = plt.scatter(0.0,0.0,c='b',alpha=0.05, s=27)
            sc.append(s)
            remove_point(0.0, 0.0)

#        elif(erag == 'i'): # ignore (matplotlib-ek erantzuteko..)
#            time.sleep(1/10000.0)  # 100 mikroseg
#            time.sleep(1/1000000.0) # 1  mikroseg
#            time.usleep(10) # 10 mikroseg
#            time.usleep(1)  # 1  mikroseg

        else:
            print("E2 Bad format: \n >>> 'a <long> <lat> <IP> <city>' \t // add point on map \n >>> 't <long> <lat> <IP> <tkop>' \t // add transact to node \n >>> 'r <long> <lat> <IP> 0' \t\t // remove point from map \n >>> 'q' \t\t\t\t // terminate execution")


    except ValueError:
        if(args == 'q'): # terminate: 'q'
            break
#        elif(args == 'i'): # ignore
#            time.sleep(1/10000.0)  # 100 mikroseg
#            time.sleep(1/1000000.0)  # 1 mikroseg

#            draw_point(0.0, 0.0)
#            s = plt.scatter(0.0,0.0,c='b',alpha=0.5, s=17)
#            sc.append(s)
#            remove_point(0.0, 0.0)

        else:
            print("E2 Bad format: \n >>> 'a <long> <lat> <IP> <city>' \t // add point on map \n >>> 't <long> <lat> <IP> <tkop>' \t // add transact to node \n >>> 'r <long> <lat> <IP> 0' \t\t // remove point from map \n >>> 'q' \t\t\t\t // terminate execution")
            err = err +1
        pass
    except KeyboardInterrupt: # (Ctrl + c) edo kill -2 PID... baina ez da beti harrapatzen kode bloke honetan
        break
    except Exception as e:
        print(e)
        err = err + 1
        time.sleep(1)
        pass


#fig = plt.figure(figsize=(8, 6), edgecolor='w')
#m = Basemap(projection='mill', resolution=None,
#            lat_0=0, lon_0=0)
#m.drawstates(color='b')
#m.bluemarble()
##draw_map(m)
#plt.show()
print("\nNunmber of nodes: ", iter)
print("Number of transactions: ", trko)
print("Removed: ", remo)
print("Errors: ", err)
#plt.show()
#time.sleep(30)
# save locally svg image 'map.svg'
plt.savefig("map.svg", format="svg")
print("Image stored with name 'map.svg'")
print("More information stored on 'mapinfo' file")

# write info on file 'mapinfo'
file = open("mapinfo", "w")
for it in items:
    file.write(str(it.coord)+": ")
    file.write(it.cit+" { ")
    for tr in it.tkop:
        file.write(tr+"   ")
#    file.write('   '.join(it.tkop))
    file.write("}")
    file.write("\n")

file.close()
