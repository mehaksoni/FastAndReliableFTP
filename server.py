import socket
import time
from struct import *
import struct
done_flag=False
neg_ack=[]
import os
import ctypes 
import sys
def retransmission():
 global neg_ack
 return_array=[]
 for index in range(0,len(neg_ack)):
	if neg_ack[index]==0:
		return_array.append(index)
 return return_array

def recv_packets(filename,destination):
 #create an INET, raw socket
 s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x0800))
 first_packet=0
 last_packet=time.time()
 try:	
 	f=open(filename,"r+b")
 except IOError:
	f=open(filename,"wb")
 global done_flag
 global neg_ack
 while True:
    now=time.time()	
    if (now-last_packet>4 and first_packet==1):
		f.close()
		if len(retransmission())==0:
			open('seq_num.txt', 'w').close()
			done_flag=True	
		else:	
			seq_num_file=open('seq_num.txt','w')
			return_array=retransmission()
			for item in return_array:
				if item==return_array[-1]:
					seq_num_file.write(str(item))
					seq_num_file.close()
					break
				seq_num_file.write(str(item)+",")
		print "SERVER:Breaking after recieveing packets from transmission ",now-last_packet
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
		dollar_present=False
		for i in write_a:
			if i == "$":
				dollar_present=True
				break
			seq_number=seq_number+i
		if dollar_present==False:
			continue
		seq_number=int(seq_number.strip('\0'))
		if neg_ack[seq_number] == 0:
                        f.seek(seq_number*1400)
			neg_ack[seq_number]=1
			write_a=write_a[len(str(seq_number))+2:]
			f.write(write_a)

 s.close()


def send_sequence_numbers(n,portNo,source,destination,interface):
	pth = str(os.getcwd())+"/library1.so"
	for i in range(0,int(n)):
		ctypes.CDLL(pth).seqsender(int(n),int(portNo),source,destination,interface)



def main():

        source=sys.argv[1]
        destination=sys.argv[2]
        interface=sys.argv[3]
	filename=sys.argv[4]	

	print source,destination,interface

	global neg_ack
	open(filename,"w").close()
	global done_flag,neg_ack
	for i in range(0,766959):
		neg_ack.append(0)
	print "SERVER:Receiving First Trasmission"
	recv_packets(filename,destination)

	print "SERVER:Sleeping for Sync"
	time.sleep(3)
	print "SERVER:Sending Sequence Numbers",len(retransmission())
	send_sequence_numbers(30,5002,source,destination,interface)

	print "SERVER:Sent Sequence Numbers"
	print "SERVER:Receiving Second Trasmission"
	recv_packets(filename,destination)
	print "SERVER:Checking the length of the neg_ack now:",len(retransmission())
		
	print "SERVER:Sleeping for Sync"
	time.sleep(3)
	print "SERVER:Sending Sequence Numbers",len(retransmission())
	send_sequence_numbers(30,5002,source,destination,interface)
	print "SERVER:Sent Sequence Numbers"
	print "SERVER:Receiving Third Trasmission"
	recv_packets(filename,destination)
	print "SERVER:Checking the length of the neg_ack now:",len(retransmission())


	print "SERVER:Sleeping for Sync"
	
	time.sleep(3)
	print "SERVER:Sending Sequence Numbers",len(retransmission())
	send_sequence_numbers(30,5002,source,destination,interface)
	print "SERVER:Sent Sequence Numbers"
	print "SERVER:Receiving Fourth Trasmission"
	recv_packets(filename,destination)
	print "SERVER:Checking the length of the neg_ack now:",len(retransmission())
	print "SERVER:Exiting"


main()
