#include "uns.h"
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <stdint.h>
#include <stdlib.h>
#include <bits/endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define L_ENDIAN
#else
#define B_ENDIAN
#endif

struct et_header{
    uint8_t  et_dhost[6];
    uint8_t  et_shost[6];
    uint16_t et_type;
}__attribute__ ((__packed__));


struct udp_fake_header
{
    uint32_t saddr;
    uint32_t daddr;
    uint8_t  nothing;
    uint8_t  protocol;
    uint16_t udp_len;
}__attribute__ ((__packed__));

struct udp_fake_header g_udp_fake_header;

struct dnshdr{
    uint16_t tid;
    uint16_t tflag;
    uint16_t qnum;
    uint16_t anum;
    uint16_t auth_rss;
    uint16_t add_rss;
}__attribute__ ((__packed__));

#define MAXBUF 1024

int send_udp(uint32_t ip, int port)
{
    struct sockaddr_in addr;
    int fd;
    if( (fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("create udp socket error %s\n", strerror(errno));
        return 0;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(ip);
    return 0;
}

void parse_dns(char * buf)
{
    struct dnshdr * dns = (struct dnshdr *) buf;
    char * p = buf+sizeof(struct dnshdr);
    char ch;
    while((ch = *p++)!= '\0')
    {
        if(ch >=0 && ch<=63){
            if( ch == 0)
                break;
            char tmp[16] ={0};
            strncpy(tmp, p, ch);
            printf("%s.", tmp);
            p += ch;
        }
    }
    printf("\n");
}

unsigned short check_sum(unsigned short *packet,int packlen)
{
	register unsigned long sum = 0;
	while (packlen > 1)
	{
		sum += *(packet++);
		packlen -= 2;
	}
	if (packlen > 0)
		sum += *(unsigned char *) packet;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return (unsigned short) (~sum);
}

void parse_udp(char * buf)
{
    uint16_t sum;
    struct udphdr* udp = (struct udphdr *)buf;
    g_udp_fake_header.udp_len = udp->len;
    struct udp_fake_header *pudp = (struct udp_fake_header * ) malloc(sizeof(struct udp_fake_header)+ htons(udp->uh_ulen));
    memcpy(pudp, &g_udp_fake_header, sizeof(struct udp_fake_header));
    if( ntohs(udp->dest) == 53)
    {
        printf("a DNS request comming!!!!!!!!!!!!!!\n");
        printf("udp checksum = %04x %04x\n", udp->uh_sum, udp->check);
        udp->uh_sum = 0;
        memcpy((unsigned char *)pudp+sizeof(struct udp_fake_header), (unsigned char *)buf, htons(udp->uh_ulen));
        sum = check_sum( (uint16_t *)pudp, htons(udp->uh_ulen)+sizeof(struct udp_fake_header));
        printf("checksum calc= %04x  %04x\n",sum, check_sum( (uint16_t *)buf, htons(udp->uh_ulen)));
        parse_dns(buf+sizeof(struct udphdr));
    }
}

void parse_ip(char * buf)
{
    struct iphdr * ip= (struct iphdr *) buf;

    struct in_addr in;
    // why no htonl , because saddr is network-byte already?
    in.s_addr = ip->saddr;
    in.s_addr = ip->daddr;
    if( 17 == ip->protocol){
        g_udp_fake_header.saddr = ip->saddr;
        g_udp_fake_header.daddr = ip->daddr;
        g_udp_fake_header.nothing = 0;
        g_udp_fake_header.protocol = 17;
        g_udp_fake_header.udp_len= 0;
        parse_udp(buf+sizeof(struct iphdr));
    }
}

void parse_et(char * buf)
{
    char type[8] ={0};
    struct et_header * eh;
    eh = (struct et_header *) buf;
    switch (ntohs(eh->et_type)) {
        case 0x800:
            strcpy(type, "IP");
            parse_ip(buf + sizeof(struct et_header));
            break;
        case 0x806:
            strcpy(type, "ARP");
            break;
        case 0x8137:
            strcpy(type, "IPX");
            break;
        case 0x809b:
            strcpy(type, "TALK");
            break;
        default:
            strcpy(type, "other");
            break;
    }
}

int
main()
{
    int fd;
    char buf[MAXBUF];
    if( (fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))< 0)
    {
        printf("create SOCK_RAW failed: %s\n", strerror(errno));
        exit(0);
    }

    while(1)
    {
        int nread = recvfrom(fd, buf, MAXBUF, 0 ,NULL, NULL);
        if( nread > 46)
        {
            parse_et(buf);
        }
    }
}
