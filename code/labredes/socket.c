/*-------------------------------------------------------------*/
/* Exemplo Socket Raw - envio de mensagens                     */
/*-------------------------------------------------------------*/

#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>//sys
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h> //funcoes para manipulacao de enderecos IP
#include <netinet/in_systm.h> //tipos de dados
#include <net/if.h>  //estrutura ifr
#include <netinet/in.h> //definicao de protocolos
#define BUFFSIZE 1518
unsigned char buff1[BUFFSIZE]; // buffer de recepcao
int sockd;
int on;
struct ifreq ifr;
#define ETHERTYPE_LEN 2
#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define BUFFER_LEN 1518
#define PCKT_LEN 8192
typedef unsigned char MacAddress[MAC_ADDR_LEN];
typedef unsigned char IPAddress[IP_ADDR_LEN];
extern int errno;

//calcula checksum
unsigned short checkSumIP(unsigned short *addr,int len)
{
        register int sum = 0;
        u_short answer = 0;
        register u_short *w = addr;
        register int nleft = len;
        /*
         * Our algorithm is simple, using a 32 bit accumulator (sum), we add
         * sequential 16 bit words to it, and at the end, fold back all the
         * carry bits from the top 16 bits into the lower 16 bits.
         */
        while (nleft > 1)  {
                sum += *w++;
                nleft -= 2;
        }

        /* mop up an odd byte, if necessary */
        if (nleft == 1) {
                *(u_char *)(&answer) = *(u_char *)w ;
                sum += answer;
        }

        /* add back carry outs from top 16 bits to low 16 bits */
        sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
        sum += (sum >> 16);                     /* add carry */
        answer = ~sum;                          /* truncate to 16 bits */
        return(answer);
}

int main()
{
  int sockFd = 0, retValue = 0;
  char *buffer = malloc(100);

  struct sockaddr_ll destAddr;
  unsigned char etherTypeT[sizeof(char)*2] = {0x08,0x00}; 
  int currentSize =0;  

  /* Configura MAC Origem e Destino */
  
  MacAddress localMac = {0xa4,0x1f,0x72,0xf5,0x90,0xad};
  IPAddress localIP = {0xa,0x20,0x8f,0xD1};

  MacAddress destMac = {0xa4, 0x1f, 0x72, 0xf5, 0x90, 0xAA};  
  IPAddress destIP = {0x0a, 0x20, 0x8f, 0xAA};  
  
  
  //usuario digita o endereço ip
  void *ipDestino[4];
  printf("PRIMEIRA PARTE DO ENDEREÇO: ");
  scanf ("%d", &ipDestino[0]);
  printf("SEGUNDA PARTE DO ENDEREÇO: ");
  scanf ("%d", &ipDestino[1]);  
  printf("TERCEIRA PARTE DO ENDEREÇO: ");
  scanf ("%d", &ipDestino[2]);  
  printf("QUARTA PARTE DO ENDEREÇO: ");
  scanf ("%d", &ipDestino[3]);  

  destIP[0] = ipDestino[0];
  destIP[1] = ipDestino[1];
  destIP[2] = ipDestino[2];
  destIP[3] = ipDestino[3];

  //Usuario digita o endereço mac
  void *macDestino[6];
  printf("PRIMEIRA PARTE DO ENDEREÇO MAC: ");
  scanf ("%X", &macDestino[0]);
  printf("SEGUNDA PARTE DO ENDEREÇO MAC: ");
  scanf ("%X", &macDestino[1]);
  printf("TERCEIRA PARTE DO ENDEREÇO MAC: ");
  scanf ("%X", &macDestino[2]);
  printf("QUARTA PARTE DO ENDEREÇO MAC: ");
  scanf ("%X", &macDestino[3]);
  printf("QUINTA PARTE DO ENDEREÇO MAC: ");
  scanf ("%X", &macDestino[4]);
  printf("SEXTA PARTE DO ENDEREÇO MAC: ");
  scanf ("%X", &macDestino[5]);

  destMac[0] = macDestino[0];
  destMac[1] = macDestino[1];
  destMac[2] = macDestino[2];
  destMac[3] = macDestino[3];
  destMac[4] = macDestino[4];
  destMac[5] = macDestino[5];

  /* Criacao do socket. Todos os pacotes devem ser construidos a partir do protocolo Ethernet. */
  /* htons: converte um short (2-byte) integer para standard network byte order. */
  if((sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    printf("Erro na criacao do socket.\n");
    exit(1);
  }

  /* Identicacao de qual maquina (MAC) deve receber a mensagem enviada no socket. */
  destAddr.sll_family = htons(PF_PACKET);
  destAddr.sll_protocol = htons(ETH_P_ALL);
  destAddr.sll_halen = 6;
  destAddr.sll_ifindex = 2;  /* indice da interface pela qual os pacotes serao enviados. Eh necessario conferir este valor. */
  memcpy(&(destAddr.sll_addr), destMac, MAC_ADDR_LEN);

  //tentativa unificada
  //buffer de toda a mensagem
  char *bufferUnificado = malloc(100);
  //cabecalho ethernet
  unsigned char ethCabecalho[sizeof(unsigned char)*14] = {0xa4, 0x1f, 0x72, 0xf5, 0x90, 0x83, 0xa4,0x1f,0x72,0xf5,0x90,0xad,0x08,0x00};
  //seta novo endereço mac
  ethCabecalho[0] = macDestino[0];
  ethCabecalho[1] = macDestino[1];
  ethCabecalho[2] = macDestino[2];
  ethCabecalho[3] = macDestino[3];
  ethCabecalho[4] = macDestino[4];
  ethCabecalho[5] = macDestino[5];
  memcpy(bufferUnificado, &ethCabecalho, 14*sizeof(unsigned char));
  //cabecalho ip
  unsigned char ipCabecalho[sizeof(unsigned char)*20] = {0x45, 0x00, 0x00, 0x54, 0xF5, 0x62,0x40,0x00,0x40,0x01,0x00,0x00,0xa,0x20,0x8f,0xD1,0x0a, 0x20, 0x8f, 0xc2};
  ipCabecalho[16] = ipDestino[0];
  ipCabecalho[17] = ipDestino[1];
  ipCabecalho[18] = ipDestino[2];
  ipCabecalho[19] = ipDestino[3];  
  //calculo do checksumIP
  unsigned short int checksumIP = checkSumIP(&ipCabecalho, 20);
  //int x = (number >> (8*n)) & 0xff http://stackoverflow.com/questions/7787423/c-get-nth-byte-of-integer
  ipCabecalho[10] = (checksumIP >> (8*0)) & 0xFF;
  ipCabecalho[11] = (checksumIP >> (8*1)) & 0xFF;
  memcpy(bufferUnificado+14*sizeof(unsigned char), &ipCabecalho, 20*sizeof(unsigned char));
  //cabecalho icmp
  unsigned char icmpCabecalho[sizeof(unsigned char) * 8] = {0x08,0x00,0x00,0x00,0x0e,0x74,0x00,0x01};
  //calculo do checksumICMP
  unsigned short int checkICMP = checkSumIP(&icmpCabecalho,8);
  icmpCabecalho[2] = (checkICMP >> (8*0)) & 0xFF;
  icmpCabecalho[3] = (checkICMP >> (8*1)) & 0xFF;
  memcpy(bufferUnificado+34*sizeof(unsigned char),&icmpCabecalho,8*sizeof(unsigned char));

  //recebimento do pacote
  int sockd = 0;
  if((sockd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    printf("Erro na criacao do socket.\n");
    exit(1);
  }
	// O procedimento abaixo eh utilizado para "setar" a interface em modo promiscuo
	strcpy(ifr.ifr_name, "eth0");
	if(ioctl(sockd, SIOCGIFINDEX, &ifr) < 0)
		printf("erro no ioctl!");
	ioctl(sockd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(sockd, SIOCSIFFLAGS, &ifr);

  //timer para calcular o rtt	
  time_t begin,end;
   

  int recebidos = 0;
  int perdidos = 0; 
  //envio de 6 pacotes 
  int pacotes = 0;
  while(pacotes < 6) {
  //verifica se recebeu pacote
    int recebeu = 0;
	   pacotes++;
    begin= time(NULL);
    //envia o pacote
    if((retValue = sendto(sockFd, bufferUnificado, 98, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
       printf("ERROR! sendto() \n");
       exit(1);
    }
    //printf("Send success (%d).\n", retValue);

    //recebe pacote
    int tentativa = 0;
    while(difftime(time(NULL), begin) < 10 ){
    	recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);
    	//ve se é o mac desse pc e o mac de quem enviou
    	if (buff1[0] == localMac[0] && buff1[1] == localMac[1] && buff1[2] == localMac[2] && // meu MAC
				buff1[3] == localMac[3] && buff1[4] == localMac[4] && buff1[5] == localMac[5] &&
				buff1[6] == destMac[0] && buff1[7] == destMac[1] && buff1[8] == destMac[2] && // mac de quem eu enviei
	 			buff1[9] == destMac[3] && buff1[10] == destMac[4] && buff1[11] == destMac[5] &&
        buff1[35] == 0x00){
    		//terminar o tll
	 		  end = time(NULL); 
    		printf("IP Origem: %d.%d.%d.%d\n",buff1[26],buff1[27],buff1[28],buff1[29]);
    		printf("TTL da mensagem: %d \n", buff1[22]);
    		printf("RTT da mensagem: %f \n", difftime(end, begin));
      begin = time(NULL);
      recebidos++;
      recebeu = 1;
			break;
		}
		tentativa++;
	}
  if (recebeu == 0){
    perdidos++;
    printf("TIMEOUT\n");
  }
  }
  printf("Pacotes enviados: %d | Pacotes recebidos: %d | Pacotes perdidos: %d \n", pacotes,recebidos,perdidos );
}
