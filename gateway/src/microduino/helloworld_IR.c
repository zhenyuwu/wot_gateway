/*
 *按照教程学习串口的编程
 *网址1：http://blog.csdn.net/shanzhizi/article/details/9241393
 *网址2：http://blog.csdn.net/shui1025701856/article/details/7571686
 *网址3：http://wenku.baidu.com/link?url=TsZc-rSDq_8ayNdWXYzkJRHLEowtx7OQMiFj_BH4M3VfvDrw4hU785aIciwS1xAmViJmz6pN0ety1_Zv8-AM2OAflOB3-UiMu9RvcSQKrAG
 */
//头文件
#include <stdio.h> 	//标准输入输出定义
#include <stdlib.h>	//标准函数库定义
#include <unistd.h>	//Unix标准函数定义
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>//时钟同步函数库
#include <sys/socket.h>
#include <fcntl.h> 	//文件控制定义
#include <termios.h>	//POSIX中断控制定义
#include <errno.h>	//错误号定义
#include <string.h>
#include <netdb.h>
#include <string.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#define serial_device "/dev/ttyS1"

char buff[512] = "";//定义数据字符串
char readbuff[100] = "";//串口读取

//打开串口
int open_port(void)
{
	int fd;		//串口的标识符
	//O_NOCTTY用来告诉Linux这个程序不会成为“控制终端”
	//O_NDELAY用来告诉Linux这个程序不关心DCD信号
	fd=open(serial_device,O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd == -1)
	{
		//不能打开串口
		perror("open_port: Unable to open /dev/ttyS0 -");
		return(fd);
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
		printf("open ttys1 .....\n");
		return(fd);
	}
}

//设置波特率
void set_speed_and_parity(int fd, int speed)
{
	int i=0;		//设置循环标志——注意不要在for内设置，否则会出错
	struct termios Opt;	//定义termios结构
	if(tcgetattr(fd,&Opt)!=0)
	{
		perror("tcgetattr fd");
		return;
	}
	tcflush(fd, TCIOFLUSH);
	cfsetispeed(&Opt, speed);
	cfsetospeed(&Opt, speed);
	/*tcsetattr函数标志：
	TCSANOW：立即执行而不等待数据发送或者接受完成。
	TCSADRAIN：等待所有数据传递完成后执行。
	TCSAFLUSH：Flush input and output buffers and make the change
	*/
	if(tcsetattr(fd, TCSANOW, &Opt) != 0)
	{	
		perror("tcsetattr fd");
		return;
	}
	tcflush(fd, TCIOFLUSH);
	//设置奇偶校验——默认8个数据位、没有校验位
        
	Opt.c_cflag &= ~PARENB;
   	Opt.c_cflag &= ~CSTOPB;
	Opt.c_cflag &= ~CSIZE;
	Opt.c_cflag |= CS8;
	
	//其他的一些配置
	//原始输入，输入字符只是被原封不动的接收
	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//软件流控制无效，因为硬件没有硬件流控制，所以就不需要管了
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	//原始输出方式可以通过在c_oflag中重置OPOST选项来选择：
	Opt.c_oflag |= ~OPOST;
	//VMIN可以指定读取的最小字符数。如果它被设置为0，那么VTIME值则会指定每个字符读取的等待时间。
	Opt.c_cc[VTIME] = 0;
	Opt.c_cc[VMIN] = 0;
	
//  Opt.c_cflag &= ~INPCK;
// 	Opt.c_cflag |= (CLOCAL | CREAD);
//	
// 	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
// 
//  Opt.c_oflag &= ~OPOST;
	Opt.c_oflag &= ~(ONLCR | OCRNL);    
//
	Opt.c_iflag &= ~(ICRNL | INLCR);
//  Opt.c_iflag &= ~(IXON | IXOFF | IXANY);    
//    
//  Opt.c_cc[VTIME] = 0;
//  Opt.c_cc[VMIN] = 0;
    
	tcflush(fd, TCIOFLUSH);

}


/**
  *串口发送数据函数
  *fd：串口描述符
  *data：待发送数据
  *datalen：数据长度
  */
int serial_write(int fd ,char* buff, int datalen)
{
	int len=0;
	//获取实际传输数据的长度
	len=write(fd,buff,datalen);
	printf("send data OK! datalen=%d\n",len);
	return len;	
}

/** 
  *串口接收数据 
  *要求启动后，在pc端发送ascii文件 
  */ 
int serial_read(int fd,char readbuff[],int datalen)
{
	int nread=0;
	printf("Ready for receiving data...");
	nread=read(fd,readbuff,datalen);
	if(nread>0)
	{
		printf("readlength=%d\n",nread);
		//printf("%s\n",readbuff);
	}
	return nread;
}

//串口控制
int serialport()
{
	int fd;	
	//打开串口
	if((fd=open_port())<0)
    	{
        	perror("open_port error");
        	return 0;
    	}
	//设置波特率和校验位
	set_speed_and_parity(fd,115200);
	return (fd);
}

//socket服务器
//等待客户端发送信息
int socket_server(int fd)
{
    int sfp,nfp;
    int recbytes;
	char buff_temp[512]="";
    struct sockaddr_in s_add,c_add;
    int sin_size;
    int portnum=8888;
    printf("Hello,welcome to my server !\r\n");
    sfp = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == sfp)
    {
        printf("socket fail ! \r\n");
        return -1;
    }
    printf("socket ok !\r\n");
    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr=htonl(INADDR_ANY);
    s_add.sin_port=htons(portnum);
	sin_size=sizeof(s_add);
    
    if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        printf("bind fail !\r\n");
        return -1;
    }
    printf("bind ok !\r\n");
    
//    if(-1 == listen(sfp,5))
//    {
//        printf("listen fail !\r\n");
//        return -1;
//    }
//    printf("listen ok\r\n");
//    while(1)
//    {
//        sin_size = sizeof(struct sockaddr_in);
//        
//        nfp = accept(sfp, (struct sockaddr *)(&c_add), &sin_size);
//        if(-1 == nfp)
//        {
//            printf("accept fail !\r\n");
//            return -1;
//        }
//        printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));
//        
//        if(-1 == (recbytes = read(nfp,buff_temp,1024)))
//        {
//            printf("read data fail !\r\n");
//            return -1;
//        }
//        printf("read ok\r\nREC:\r\n");
	while(1)  
	{  
		recvfrom(sfp,buff_temp,sizeof(buff_temp),0,(struct sockaddr *)&s_add,&sin_size);  
//		printf("Response from server:%s\n",buff_temp);  
		if(strncmp(buff_temp,"stop",4) == 0)//接受到的消息为 “stop”  
		{  
			printf("Sender has told me to end the connection\n");
			break;  
		}
		if(strcmp(buff_temp,"")!=0)
		{
			int buff_temp_size=sizeof(buff_temp);
			printf("OK\n");
			buff_temp[buff_temp_size]='\r';
			buff_temp[buff_temp_size+1]='\n';
			buff_temp[buff_temp_size+2]='\0';
			strcpy(buff,"+ZBD=1983,");
			strcat(buff,buff_temp);
			printf("%s\r\n",buff);
			if(serial_write(fd,buff,15)<=0)
				printf("serial_write is failed.");
		}
        close(nfp);
    }
}

//主函数
int main(void)
{
	int fd;
	int nread,i,n =0,datalen=255, len = 0;
	
	fd=serialport();
	printf("fd=%d\n",fd);
	
	//尝试先写内容
	nread=write(fd,"hello\r\n",8);
	printf("nread%d\n",nread);

    if(-1 == socket_server(fd))
    {
        printf("Server is break down.");
    }
}
