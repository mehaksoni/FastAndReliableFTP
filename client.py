from struct import *
import struct
import socket   #for sockets
import threading
import sys  #for exit
import ctypes
import time
import os
import random

def getSize(fileobject):
	fileobject.seek(0,2) # move the cursor to the end of the file
	size = fileobject.tell()
	return size

def getPacketNumber(filename):
	f=open(filename,"rb")
	fsize = getSize(f)
	if fsize%1400 is 0:
		nP=fsize/1400
	else:
		nP=(fsize/1400)+1
	return nP
	
def recv_sequence_number_packets(destination):
 #create an INET, raw socket
 neg_ack=set()
 s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x0800))
 first_packet=0
 last_packet=time.time()
 while True:
    now=time.time()	
    if (now-last_packet>4 and first_packet==1):
		print "Breaking ",now-last_packet
		break
    packet = s.recvfrom(65565)
    packet = packet[0]
    ip_header = packet[14:34]
    iph = unpack('!BBHHHBBH4s4s' , ip_header)
    version_ihl = iph[0]
    version = version_ihl >> 4
    ihl = version_ihl & 0xF
    iph_length = ihl * 4
    ttl = iph[5]
    protocol = iph[6]
    s_addr = socket.inet_ntoa(iph[8]);
    d_addr = socket.inet_ntoa(iph[9]);
    seq_number="" 
    if (s_addr==destination):
		first_packet=1
	        last_packet=time.time()
		write_a=packet[42:]
		if ',' in write_a:	
			write_a=write_a.split(',')
			for item in write_a:
				item=int(item.strip('\0'))
				neg_ack.add(item)
 s.close()
 return list(neg_ack)

def sendInitialPackets(nP,portNo,source,destination,interface,filename):
	pth = str(os.getcwd())+"/library.so"
	ctypes.CDLL(pth).packetsraw(int(nP),int(portNo),source,destination,interface,filename)



def sendFastPackets(nP,negAck,i,source,destination,interface,filename):
	pth = str(os.getcwd())+"/library.so"
	arr = (ctypes.c_int * len(negAck))(*negAck)
	ctypes.CDLL(pth).sendFastPackets(nP,5001,arr,i,source,destination,interface,filename)

	

def main():

	#source 3
	#destination 4
	#interface 5
	#filename 6
	
	source=sys.argv[1]
	destination=sys.argv[2]
	interface=sys.argv[3]
	filename=sys.argv[4]


	nP=getPacketNumber(filename)
	print "CLIENT : Sending for the first time"
	sendInitialPackets(int(nP),5001,source,destination,interface,filename)
	print "CIENT:Ready to REcieve Packets"
	neg_ack=recv_sequence_number_packets(destination)
	print "CLIENT: Recieved sequence number Packets "
	print "CLIENT:Retransmitting "+str(len(neg_ack))+" Packets"
	time.sleep(3)
	neg_ack_array=[]
	for i in range(0,nP):
		neg_ack_array.append(0)
	for item in neg_ack:
		neg_ack_array[int(item)]=1
	
	print "CLIENT:First Retransmission"
	sendFastPackets(nP,neg_ack_array,2,source,destination,interface,filename)



	print "CIENT:Ready to REcieve Packets"
	neg_ack=recv_sequence_number_packets(destination)
	print "CLIENT: Recieved sequence number Packets "
	print "CLIENT:Retransmitting "+str(len(neg_ack))+" Packets"
	print "CLIENT:Sleeping for sync"
	time.sleep(3)
	neg_ack_array=[]
	for i in range(0,nP):
		neg_ack_array.append(0)
	for item in neg_ack:
		neg_ack_array[int(item)]=1
	
	print "CLIENT:Second Retransmission"
	sendFastPackets(nP,neg_ack_array,2,source,destination,interface,filename)



	print "CIENT:Ready to REcieve Packets"
	neg_ack=recv_sequence_number_packets(destination)
	print "CLIENT: Recieved sequence number Packets "
	print "CLIENT:Retransmitting "+str(len(neg_ack))+" Packets"
	print "CLIENT:Sleeping for sync"
	time.sleep(3)
	neg_ack_array=[]
	for i in range(0,nP):
		neg_ack_array.append(0)
	for item in neg_ack:
		neg_ack_array[int(item)]=1
	
	print "CLIENT:Third Retransmission"
	sendFastPackets(nP,neg_ack_array,10,source,destination,interface,filename)
	return		


main()
