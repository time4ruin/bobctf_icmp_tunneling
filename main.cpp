#define ETHERTYPE_IP 0x0800

#include <pcap.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdint.h>

struct eth_header{
  uint8_t dst_mac[6];
  uint8_t src_mac[6];
  uint16_t prt_type;
};

struct ip_header{
  uint8_t ver4_hlen4; //version 4bit + header length 4bit
  uint8_t DSF; //Differentiated Serviced Field
  uint16_t totallen;
  uint16_t id;
  uint16_t fragoffset;
  uint8_t ttl; //Time to live
  uint8_t protocol;
  uint16_t hchecksum;
  uint32_t src_ip;
  uint32_t dst_ip;
}; //can be up to 40 optional bytes

struct tcp_header{
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t seqnum;
  uint32_t acknum;
  uint8_t hlen4_rsv4; //header length 4bit + reserved 4bit
  uint8_t flags;
  uint16_t wsize; //window size
  uint16_t checksum;
  uint16_t urgent_pointer;
}; //can be up to 20 optional bytes

void print_mac(uint8_t *p){
	for (int j = 0; j < 6; j++){
		printf("%02X", p[j]);
		if (j!=5) printf(":");
	}
	printf("\n");
}

void print_ip(uint32_t p){
	for (int i = 0; i < 4; i++){
		printf("%d", p & 0xff);
		p = p >> 8;
		if (i != 3) printf(".");
	}
	printf("\n");
}

int saved_ttl = 0;
int word = 0;
int h1=0, h2=0;
short saved_id=0;
void dump(const uint8_t* p, int len){

	struct eth_header *eth;
	struct ip_header *ip;
	struct tcp_header *tcp;

	/*
	for(int i = 0; i < len; i++){
		printf("%02x ", *(p+i));
		if((i & 0x0f) == 0x0f)
			printf("\n");
		}
	printf("\n");
	*/
	
	/* Ethernet Header */
	eth = (struct eth_header *)p;

//	printf("Source MAC: ");
//	print_mac(eth->src_mac);
//	printf("Destination MAC: ");
//	print_mac(eth->dst_mac);
	if (ntohs(eth->prt_type) == ETHERTYPE_IP){ // if IPv4 (0x0800 in little endian)
		p += sizeof(struct eth_header);
		/* IP Header */
		ip = (struct ip_header *)p;
//		printf("Source IP: ");
//		print_ip(ip->src_ip);
//		printf("Destination IP: ");
//		print_ip(ip->dst_ip);
		if (ip->protocol == 0x01){ // if ip & icmp
			/* IP Header */
			//ip = (struct ip_header *)p;
			if (((ip->src_ip >> 24) == 130) || ((ip->dst_ip >> 24) == 130)){
//				printf("no\n");
				//printf("%u %u %u %u\n",saved_ttl, ip->ttl, h1, h2);
				if (ip->ttl == 253 || ip->ttl == 255){
//					printf("\n");
				}
				short *tmp = (short *)p;
				//printf("%x\n",tmp[12]);
				if ((ip->ttl >= 100) && (ip->ttl <= 115) && (saved_id != tmp[12])){
					h1 = h2;
					h2 = ip->ttl - 100;
					//printf("%u %u %u %u\n",saved_ttl, ip->ttl, h1, h2);
					word++;
					//printf("%X %u %u %u %u\n",saved_id, ip->ttl, h1, h2, word);
					if ((word % 2) == 0){
						printf("%c",h1 * 16 + h2);
					}
					saved_id = tmp[12];
				}
				
			}
			else {
				//printf("Source IP: ");
				//print_ip(ip->src_ip);
				//printf("Destination IP: ");
				//print_ip(ip->dst_ip);
				p += ((ip->ver4_hlen4 & 0x0f) * 4);

				/* TCP Header */
				//tcp = (struct tcp_header *)p;
				//printf("Source Port: %d\n", ntohs(tcp->src_port));
				//printf("Destination Port: %d\n", ntohs(tcp->dst_port));
			
				/* Data */
				int Totallen = ntohs(ip->totallen);
				int IPlen = (ip->ver4_hlen4 & 0x0f) * 4;
				//int TCPlen = ((tcp->hlen4_rsv4 & 0xf0) >> 4) * 4;
				//p += TCPlen;

				int Datalen = Totallen - IPlen;// - TCPlen;
				/*for (int j = 0; j < Datalen; j++){
					printf("%c", *p);
					if ((j & 0x0f) == 0x0f){
						//printf("\n");
					}
					p++;
				}*/
				
//				printf("\n");
				//if (writelen > 10) printf("...\n");

			}
		}
	}
//	printf("\n");
}

void usage() {
  printf("syntax: pcap_test\n");
  printf("sample: pcap_test\n");
}

int main(int argc, char* argv[]) {
  if (argc != 1) {
    usage();
    return -1;
  }

  char* dev = argv[1];
  char errbuf[PCAP_ERRBUF_SIZE];
  //pcap_t* handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  pcap_t* handle = pcap_open_offline("./complete.pcapng", errbuf);
  if (handle == NULL) {
    fprintf(stderr, "couldn't open device %s: %s\n", dev, errbuf);
    return -1;
  }

  while (true) {
    struct pcap_pkthdr* header;
    const uint8_t* packet;
    int res = pcap_next_ex(handle, &header, &packet);
    if (res == 0) continue;
    if (res == -1 || res == -2) break;
    //printf("%u bytes captured\n", header->caplen);
    dump(packet, header->caplen);
    //break;
  }

  pcap_close(handle);
  return 0;
}
