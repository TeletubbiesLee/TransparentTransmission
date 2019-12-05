#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <getopt.h>
#include <termios.h>


//#define SERVPORT 3333
#define MAXDATASIZE 100 /*max client */
#define BACKLOG 10 /* max client */
#define FALSE 1
#define TRUE 0

/* The name of this program */
const char * program_name;

int speed_arr[] = {
	B921600, B460800, B230400, B115200, B57600, B38400, B19200,
	B9600, B4800, B2400, B1200, B300,
};

int name_arr[] = {
	921600, 460800, 230400, 115200, 57600, 38400,  19200,
	9600,  4800,  2400,  1200,  300,
};


int OpenDev(char *Dev);
int set_Parity(int fd,int databits,int stopbits,int parity);
void set_speed(int fd, int speed);
void print_usage();


#if 0
/**
 *	TCP客户端
 */
int main(int argc, char *argv[])
{
	int uartfd, sockfd, recvbytes;
	char bufReceive[MAXDATASIZE];
	char bufSend[MAXDATASIZE];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	int servport = 3333;
	char device[] = "/dev/ttymxc1";
	int nread;			/* Read the counts of data */
	int flags;

	if (argc < 2)
	{
		fprintf(stderr, "Please enter the server's hostname!\n");
		exit(1);
	}
	if((host = gethostbyname(argv[1])) == NULL)
	{
		herror("gethostbyname error！");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket create error！");
		exit(1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(servport);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("connect error！");
		exit(1);
	}

	uartfd = OpenDev(device);
	if (uartfd > 0)
	{
		set_speed(uartfd, 115200);
	} else
	{
		fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
		exit(1);
	}

	if (set_Parity(uartfd, 8, 1, 'N') == FALSE)
	{
		fprintf(stderr, "Set Parity Error\n");
		close(uartfd);
		exit(1);
	}

	flags = fcntl(sockfd, F_GETFL, 0);				//获取原始sockfd属性
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);		//添加非阻塞

	while(1)
	{
		recvbytes = recv(sockfd, bufReceive, MAXDATASIZE, 0);
		if (recvbytes > 0)
		{
			bufReceive[recvbytes] = '\0';
			if (write(uartfd, bufReceive, strlen(bufReceive)) == -1)
			{
				printf("write error！\r\n");
				exit(1);
			}
			printf("socket receivr, usart send: %s\r\n", bufReceive);
		}

		nread = read(uartfd, bufSend, sizeof(bufSend));
		if (nread > 0)
		{
			bufSend[nread] = '\0';
			if (send(sockfd, bufSend, strlen(bufSend), 0) == -1)
			{
				printf("send error！\r\n");
				exit(1);
			}
			printf("usart receivr, socket send: %s\r\n", bufSend);
		}
	}
	close(sockfd);
	close(uartfd);
	return 0;
}

#else

/**
 *	TCP服务器
 */
int main (int argc, char *argv[])
{
	int uartfd, sockfd, client_fd, recvbytes;
	struct sockaddr_in my_addr; /* loacl */
	struct sockaddr_in remote_addr;
	char bufReceive[MAXDATASIZE];
	char bufSend[MAXDATASIZE];
	socklen_t sin_size;
	int servport = 3333;
	char device[] = "/dev/ttymxc1";
	int nread;			/* Read the counts of data */
	int flags;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket fail！"); exit(1);
	}

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(servport);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind error！");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen error！");
		exit(1);
	}

	uartfd = OpenDev(device);
	if (uartfd > 0)
	{
		set_speed(uartfd, 115200);
	} else
	{
		fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
		exit(1);
	}

	if (set_Parity(uartfd, 8, 1, 'N') == FALSE)
	{
		fprintf(stderr, "Set Parity Error\n");
		close(uartfd);
		exit(1);
	}

	sin_size = sizeof(struct sockaddr_in);
	client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size);
	if (client_fd == -1)
	{
		perror("accept error");
	}
	else
	{
		printf("REC FROM： %s\n", inet_ntoa(remote_addr.sin_addr));
		flags = fcntl(client_fd, F_GETFL, 0);				//获取原始client_fd属性
		fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);		//添加非阻塞
	}

	while(1)
	{
		recvbytes = recv(client_fd, bufReceive, MAXDATASIZE, 0);
		if (recvbytes > 0)
		{
			bufReceive[recvbytes] = '\0';
			if (write(uartfd, bufReceive, strlen(bufReceive)) == -1)
			{
				printf("write error！\r\n");
				exit(1);
			}
			printf("client_fd receivr, usart send: %s\r\n", bufReceive);
		}

		nread = read(uartfd, bufSend, sizeof(bufSend));
		if (nread > 0)
		{
			bufSend[nread] = '\0';
			if (send(client_fd, bufSend, strlen(bufSend), 0) == -1)
			{
				printf("send error！\r\n");
				exit(1);
			}
			printf("usart receivr, client_fd send: %s\r\n", bufSend);
		}
	}
	close(client_fd);
	close(sockfd);
	close(uartfd);
	return 0;
}

#endif


/**
 *	@breif 打开串口
 */
int OpenDev(char *Dev)
{
	int fd = open(Dev, O_RDWR | O_NONBLOCK);         //| O_NOCTTY | O_NDELAY
 	if (-1 == fd) { /*设置数据位数*/
   		perror("Can't Open Serial Port");
   		return -1;
	} else
		return fd;
}

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;
	tcgetattr(fd, &Opt);

	for (i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i])	{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if  (status != 0)
				perror("tcsetattr fd1");
				return;
		}
		tcflush(fd,TCIOFLUSH);
  	 }

	if (i == 12){
		printf("\tSorry, please set the correct baud rate!\n\n");
		print_usage(stderr, 1);
	}
}
/*
	*@brief   设置串口数据位，停止位和效验位
	*@param  fd     类型  int  打开的串口文件句柄*
	*@param  databits 类型  int 数据位   取值 为 7 或者8*
	*@param  stopbits 类型  int 停止位   取值为 1 或者2*
	*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if  ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE ;
	switch (databits) /*设置数据位数*/ {
	case 7:
		options.c_cflag |= CS7;
	break;
	case 8:
		options.c_cflag |= CS8;
	break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
	}

	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
	break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
		options.c_iflag |= INPCK;             /* Disnable parity checking */
	break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
		options.c_iflag |= INPCK;       /* Disnable parity checking */
	break;
	case 'S':
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
	break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
 	/* 设置停止位*/
  	switch (stopbits) {
   	case 1:
    	options.c_cflag &= ~CSTOPB;
  	break;
 	case 2:
  		options.c_cflag |= CSTOPB;
  	break;
 	default:
  		fprintf(stderr,"Unsupported stop bits\n");
  		return (FALSE);
 	}
  	/* Set input parity option */
  	if (parity != 'n')
    	options.c_iflag |= INPCK;
  	options.c_cc[VTIME] = 150; // 15 seconds
    	options.c_cc[VMIN] = 0;

	options.c_lflag &= ~(ECHO | ICANON);

  	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
  	if (tcsetattr(fd,TCSANOW,&options) != 0) {
    	perror("SetupSerial 3");
  		return (FALSE);
 	}
	return (TRUE);
}

void print_usage (FILE *stream, int exit_code)
{
    fprintf(stream, "Usage: %s option [ dev... ] \n", program_name);
    fprintf(stream,
            "\t-h  --help     Display this usage information.\n"
            "\t-d  --device   The device ttyS[0-3] or ttySCMA[0-1]\n"
	    "\t-b  --baudrate Set the baud rate you can select\n"
	    "\t               [230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300]\n"
            "\t-s  --string   Write the device data\n");
    exit(exit_code);
}


