//
// #include <stdlib.h>
// #include <errno.h>
// #include <string.h>
// #include <netdb.h>
//
// #include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
// 
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <getopt.h>
// #include <termios.h>

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "Uart.h"
#include "Net.h"
#include "Pthread.h"




int TCP_Client2Uart(char *ipAddress);
int TCP_Server2Uart(void);
int UDP2Uart(void);


int main(int argc, char *argv[])
{
	TCP_Client2Uart("192.168.10.11");
	return 0;
}

/**
 * @breif TCP客户端转串口
 * @param ipAddress IP地址，格式："192.168.1.1"
 * @return 成功0或失败-1
 */
int TCP_Client2Uart(char *ipAddress)
{
	int uartfd, sockfd;		//串口和网卡设备的文件描述符
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;
	
	sockfd = TCP_NetConnect(SERVER_PORT, ipAddress);		//连接网口
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口

	fdArray[0] = sockfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Net2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2NetPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

TCP_CLIENT_CLOSE:
	pthread_join(net2UartPid, &status);
	pthread_join(uart2NetPid, &status);
	close(sockfd);
	close(uartfd);

	return 0;
}


/**
 * @breif TCP服务端转串口
 * @return 成功0或失败-1
 */
int TCP_Server2Uart(void)
{
	int uartfd, sockfd, clientfd;
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;

	sockfd = TCP_NetListen(SERVER_PORT);
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口
	clientfd = TCP_NetAccept(sockfd);
	
	fdArray[0] = clientfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Net2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_SERVER_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2NetPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto TCP_SERVER_CLOSE;
	}

TCP_SERVER_CLOSE:
	pthread_join(net2UartPid, &status);
	pthread_join(uart2NetPid, &status);
	close(clientfd);
	close(sockfd);
	close(uartfd);

	return 0;
}


/**
 * @breif UDP转串口
 * @return 成功0或失败-1
 */
int UDP2Uart(void)
{
	int uartfd, sockfd;
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t udp2UartPid, uart2UdpPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;

	sockfd = UDP_NetConnect(SERVER_PORT);		//连接网口
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口

	fdArray[0] = sockfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&udp2UartPid, NULL, (void*)UDP2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread UDP2Uart create error!\n");
		goto UDP_CLOSE;
	}

	ret = pthread_create(&uart2UdpPid, NULL, (void*)Uart2UDPPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2UDP create error!\n");
		goto UDP_CLOSE;
	}

UDP_CLOSE:
	pthread_join(udp2UartPid, &status);
	pthread_join(uart2UdpPid, &status);
	close(sockfd);
	close(uartfd);

	return 0;
}































/**
 *               ii.                                         ;9ABH,
 *              SA391,                                    .r9GG35&G
 *              &#ii13Gh;                               i3X31i;:,rB1
 *              iMs,:,i5895,                         .5G91:,:;:s1:8A
 *               33::::,,;5G5,                     ,58Si,,:::,sHX;iH1
 *                Sr.,:;rs13BBX35hh11511h5Shhh5S3GAXS:.,,::,,1AG3i,GG
 *                .G51S511sr;;iiiishS8G89Shsrrsh59S;.,,,,,..5A85Si,h8
 *               :SB9s:,............................,,,.,,,SASh53h,1G.
 *            .r18S;..,,,,,,,,,,,,,,,,,,,,,,,,,,,,,....,,.1H315199,rX,
 *          ;S89s,..,,,,,,,,,,,,,,,,,,,,,,,....,,.......,,,;r1ShS8,;Xi
 *        i55s:.........,,,,,,,,,,,,,,,,.,,,......,.....,,....r9&5.:X1
 *       59;.....,.     .,,,,,,,,,,,...        .............,..:1;.:&s
 *      s8,..;53S5S3s.   .,,,,,,,.,..      i15S5h1:.........,,,..,,:99
 *      93.:39s:rSGB@A;  ..,,,,.....    .SG3hhh9G&BGi..,,,,,,,,,,,,.,83
 *      G5.G8  9#@@@@@X. .,,,,,,.....  iA9,.S&B###@@Mr...,,,,,,,,..,.;Xh
 *      Gs.X8 S@@@@@@@B:..,,,,,,,,,,. rA1 ,A@@@@@@@@@H:........,,,,,,.iX:
 *     ;9. ,8A#@@@@@@#5,.,,,,,,,,,... 9A. 8@@@@@@@@@@M;    ....,,,,,,,,S8
 *     X3    iS8XAHH8s.,,,,,,,,,,...,..58hH@@@@@@@@@Hs       ...,,,,,,,:Gs
 *    r8,        ,,,...,,,,,,,,,,.....  ,h8XABMMHX3r.          .,,,,,,,.rX:
 *   :9, .    .:,..,:;;;::,.,,,,,..          .,,.               ..,,,,,,.59
 *  .Si      ,:.i8HBMMMMMB&5,....                    .            .,,,,,.sMr
 *  SS       :: h@@@@@@@@@@#; .                     ...  .         ..,,,,iM5
 *  91  .    ;:.,1&@@@@@@MXs.                            .          .,,:,:&S
 *  hS ....  .:;,,,i3MMS1;..,..... .  .     ...                     ..,:,.99
 *  ,8; ..... .,:,..,8Ms:;,,,...                                     .,::.83
 *   s&: ....  .sS553B@@HX3s;,.    .,;13h.                            .:::&1
 *    SXr  .  ...;s3G99XA&X88Shss11155hi.                             ,;:h&,
 *     iH8:  . ..   ,;iiii;,::,,,,,.                                 .;irHA
 *      ,8X5;   .     .......                                       ,;iihS8Gi
 *         1831,                                                 .,;irrrrrs&@
 *           ;5A8r.                                            .:;iiiiirrss1H
 *             :X@H3s.......                                .,:;iii;iiiiirsrh
 *              r#h:;,...,,.. .,,:;;;;;:::,...              .:;;;;;;iiiirrss1
 *             ,M8 ..,....,.....,,::::::,,...         .     .,;;;iiiiiirss11h
 *             8B;.,,,,,,,.,.....          .           ..   .:;;;;iirrsss111h
 *            i@5,:::,,,,,,,,.... .                   . .:::;;;;;irrrss111111
 *            9Bi,:,,,,......                        ..r91;;;;;iirrsss1ss1111
 *
 *								狗头保佑，永无BUG！
 */





