#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <errno.h> //For errno - the error number
#include <netinet/udp.h>   //Provides declarations for udp header
#include <netinet/ip.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

#define MY_DEST_MAC0    0xFF
#define MY_DEST_MAC1    0xFF
#define MY_DEST_MAC2    0xFF
#define MY_DEST_MAC3    0xFF
#define MY_DEST_MAC4    0xFF
#define MY_DEST_MAC5    0xFF
#define BUF_SIZ         1501

void *get_in_addr(struct sockaddr *sa) {
         if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
         }
         return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

extern void packetsraw(int np, int dport, char * src, char * dest, char * iface, char * fName){
	packetRaw(np, dport, src, dest,iface,fName);
}

extern void sendFastPackets(int np, int dport, int ack[],int freq, char * src, char * dest, char * iface, char * fName){
    fastPacket(np, dport, ack,freq, src, dest, iface, fName);
}

//Used References:
// https://austinmarton.wordpress.com/2011/09/14/sending-raw-ethernet-packets-from-a-specific-interface-in-c-on-linux/
// http://www.binarytides.com/raw-udp-sockets-c-linux/ 
void packetRaw(int np, int dport, char * source, char * destination, char * iface, char * fName) {

	FILE *fr;
    fr = fopen (fName, "rb");
	fseek(fr, 0, SEEK_END);
	long fsize = ftell(fr);
	fseek(fr, 0, SEEK_SET);
	
    char line[1400];

    char interface[10];
    int sockfd;
    int i=0;
    char payloadStr[10];
    struct ifreq if_idx;
    struct ifreq if_mac;
    int tx_len = 0;
    char sendbuf[BUF_SIZ];
    struct ether_header *eh = (struct ether_header *) sendbuf;

    struct sockaddr_ll socket_address;
    char ifName[IFNAMSIZ];

    memset(interface,0,10);
    strcat(interface, iface);

    /* Open RAW socket to send on */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("socket");
    }

    // /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, interface, strlen(interface));
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");

    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, interface, strlen(interface));
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

	int t=0;
	int counter = 0;
    for(t=0;t< np; t++) {
        // /* Construct the Ethernet header */
        memset(sendbuf, 0, BUF_SIZ);
	    tx_len=0;

        /* Ethernet header */
        eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
        eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
        eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
        eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
        eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
        eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
        eh->ether_dhost[0] = MY_DEST_MAC0;
        eh->ether_dhost[1] = MY_DEST_MAC1;
        eh->ether_dhost[2] = MY_DEST_MAC2;
        eh->ether_dhost[3] = MY_DEST_MAC3;
        eh->ether_dhost[4] = MY_DEST_MAC4;
        eh->ether_dhost[5] = MY_DEST_MAC5;
        /* Ethertype field */
        eh->ether_type = htons(0x0800);
        tx_len += sizeof(struct ether_header);

        char sipBuf [10];
        char dipBuf[10];
        memset(sipBuf,0,10);
        memset(dipBuf,0,10);

        strcpy (sipBuf,source);
        strcpy(dipBuf,destination);
        struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
        /* IP Header */
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 16; // Low delay
        iph->id = htons(54321);
        iph->ttl = 64; // hops
        iph->protocol = 17; // UDP

        /* Source IP address, can be spoofed */
        iph->saddr = inet_addr(sipBuf);

        /* Destination IP address */
        iph->daddr = inet_addr(dipBuf);
        tx_len += sizeof(struct iphdr);

        struct udphdr *udph = (struct udphdr *) (sendbuf + sizeof(struct iphdr) + sizeof(struct ether_header));
        /* UDP Header */
        udph->source = htons(3423);
        udph->dest = htons(dport);
        udph->check = 0; // skip
        tx_len += sizeof(struct udphdr);


        memset(line,0,1400);
	    char packNum[15];
	    memset(packNum,0,15);
	    sprintf(packNum,"%d",t);
	
	    for(i=0;i<=strlen(packNum);i++){
                sendbuf[tx_len++]=packNum[i];   
        }
        sendbuf[tx_len++]='$';
        char ch;
        for (i = 0; i < 1400; i++) {
        	ch=fgetc(fr);
        	counter++;
        	if(((int)fsize < counter))
        		break;
        	else{
                    sendbuf[tx_len++] = ch;
                }
        }
        /* Length of UDP payload and header */
        udph->len = htons(tx_len - sizeof(struct ether_header) - sizeof(struct iphdr));
        /* Length of IP payload and header */
        iph->tot_len = htons(tx_len - sizeof(struct ether_header));

        /* Index of the network device */
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        /* Address length*/
        socket_address.sll_halen = ETH_ALEN;

        /* Destination MAC */
        socket_address.sll_addr[0] = MY_DEST_MAC0;
        socket_address.sll_addr[1] = MY_DEST_MAC1;
        socket_address.sll_addr[2] = MY_DEST_MAC2;
        socket_address.sll_addr[3] = MY_DEST_MAC3;
        socket_address.sll_addr[4] = MY_DEST_MAC4;
        socket_address.sll_addr[5] = MY_DEST_MAC5;

        /* Send packet */
	    if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                printf("Send failed\n");
		    memset(sendbuf, 0, BUF_SIZ);
        }
    close(sockfd);
    fclose(fr);
    }


void fastPacket(int np, int dport, int ack[] ,int freq, char  * src, char * dest , char * iface, char * fName) {

        FILE *fr;
        fr = fopen (fName, "rb");
        fseek(fr, 0, SEEK_END);
        long fsize = ftell(fr);
        fseek(fr, 0, SEEK_SET);
        char line[1400];

        char interface[10];
        int sockfd;
        int i=0;
        char payloadStr[10];
        struct ifreq if_idx;
        struct ifreq if_mac;
        int tx_len = 0;
        char sendbuf[BUF_SIZ];
        struct ether_header *eh = (struct ether_header *) sendbuf;
        struct sockaddr_ll socket_address;
        char ifName[IFNAMSIZ];

        memset(interface,0,10);
        strcat(interface, iface);
        //printf("%s\n",interface);

        /* Open RAW socket to send on */
        if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
            perror("socket");
        }

        // /* Get the index of the interface to send on */
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, interface, strlen(interface));
        if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
            perror("SIOCGIFINDEX");

        /* Get the MAC address of the interface to send on */
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, interface, strlen(interface));
        if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
            perror("SIOCGIFHWADDR");

        int t=0;
        int counter = 0;
        
        for(t=0;t< np; t++) {
        // /* Construct the Ethernet header */
        memset(sendbuf, 0, BUF_SIZ);
        tx_len=0;

        /* Ethernet header */
        eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
        eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
        eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
        eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
        eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
        eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
        eh->ether_dhost[0] = MY_DEST_MAC0;
        eh->ether_dhost[1] = MY_DEST_MAC1;
        eh->ether_dhost[2] = MY_DEST_MAC2;
        eh->ether_dhost[3] = MY_DEST_MAC3;
        eh->ether_dhost[4] = MY_DEST_MAC4;
        eh->ether_dhost[5] = MY_DEST_MAC5;
        /* Ethertype field */
        eh->ether_type = htons(0x0800);
        tx_len += sizeof(struct ether_header);

        char sipBuf [10];
        char dipBuf[10];
        memset(sipBuf,0,10);
        memset(dipBuf,0,10);

        strcpy (sipBuf,src);
        strcpy(dipBuf,dest);
        struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
        /* IP Header */
        iph->ihl = 5;
        iph->version = 4;
        iph->tos = 16; // Low delay
        iph->id = htons(54321);
        iph->ttl = 64; // hops
        iph->protocol = 17; // UDP

        /* Source IP address, can be spoofed */
        iph->saddr = inet_addr(sipBuf);
        // iph->saddr = inet_addr("192.168.0.112");

        /* Destination IP address */
        iph->daddr = inet_addr(dipBuf);
        tx_len += sizeof(struct iphdr);

        struct udphdr *udph = (struct udphdr *) (sendbuf + sizeof(struct iphdr) + sizeof(struct ether_header));
        /* UDP Header */
        udph->source = htons(3423);
        udph->dest = htons(dport);
        udph->check = 0; // skip
        tx_len += sizeof(struct udphdr);

        memset(line,0,1400);
        char packNum[15];
        memset(packNum,0,15);
        sprintf(packNum,"%d",t);

        for(i=0;i<=strlen(packNum);i++){
            sendbuf[tx_len++]=packNum[i];   
        }
        sendbuf[tx_len++]='$';
        char ch;
        for (i = 0; i < 1400; i++) {
            ch=fgetc(fr);
            counter++;
            if(((int)fsize < counter))
                break;
            else
                sendbuf[tx_len++] = ch;        
        }

        /* Length of UDP payload and header */
        udph->len = htons(tx_len - sizeof(struct ether_header) - sizeof(struct iphdr));
        /* Length of IP payload and header */
        iph->tot_len = htons(tx_len - sizeof(struct ether_header));

        /* Index of the network device */
        socket_address.sll_ifindex = if_idx.ifr_ifindex;
        /* Address length*/
        socket_address.sll_halen = ETH_ALEN;

        /* Destination MAC */
        socket_address.sll_addr[0] = MY_DEST_MAC0;
        socket_address.sll_addr[1] = MY_DEST_MAC1;
        socket_address.sll_addr[2] = MY_DEST_MAC2;
        socket_address.sll_addr[3] = MY_DEST_MAC3;
        socket_address.sll_addr[4] = MY_DEST_MAC4;
        socket_address.sll_addr[5] = MY_DEST_MAC5;

        /* Send packet */
        int no=0;
        if (ack[t]==1){
            for (no=0; no<freq; no++){
                if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
                    printf("Send failed\n");
            }
        }
        memset(sendbuf, 0, BUF_SIZ);
    }
    close(sockfd);
    fclose(fr);
}
