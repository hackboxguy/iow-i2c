/*
* I2C sample for IO-Warrior28
* This sample use the first found IO-Warrior device on IowKitOpenDevice().
* The I2C device (address) which used in this sample may be changed for your device.
*
* IO-Warrior28 will only works with SDK version 1.7 or higher. Please check your linux kernel
* for the actual driver which includes IO-Warrior28. Driver was added March 2020 into the kernel.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>	/* for NAME_MAX */
#include <sys/ioctl.h>
#include <string.h>
#include <strings.h>	/* for strcasecmp() */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>

#include "iowkit.h"
#define I2C_ACCESS_TYPE_SMBUS 1
#define I2C_SMBUS_READ	1
#define I2C_SMBUS_WRITE	0
#define I2C_SMBUS_QUICK		    0
#define I2C_SMBUS_BYTE		    1
#define I2C_SMBUS_BYTE_DATA	    2
#define I2C_SMBUS_WORD_DATA	    3
#define I2C_SMBUS_PROC_CALL	    4
#define I2C_SMBUS_BLOCK_DATA	    5
#define I2C_SMBUS_I2C_BLOCK_DATA    6
#define I2C_SMBUS_BLOCK_PROC_CALL   7		/* SMBus 2.0 */
#define I2C_SMBUS_BLOCK_DATA_PEC    8		/* SMBus 2.0 */
#define I2C_SMBUS_PROC_CALL_PEC     9		/* SMBus 2.0 */
#define I2C_SMBUS_BLOCK_PROC_CALL_PEC  10	/* SMBus 2.0 */
#define I2C_SMBUS_WORD_DATA_PEC	   11		/* SMBus 2.0 */
#define I2C_SMBUS_BLOCK_MAX	32	/* As specified in SMBus standard*/
#define I2C_SMBUS_I2C_BLOCK_MAX	32	/* Not specified but we use same structure */
#define I2C_READ(FD,BUFF,SIZE) 	read(FD,BUFF,SIZE)
#define I2C_WRITE(FD,BUFF,SIZE) write(FD,BUFF,SIZE)
#define I2C_DEVICE_NODE "/dev/i2c-11" //on pi-3, when usb-tiny i2c-dongle is connected, /dev/i2c-11 shows up
union i2c_smbus_data
{
	uint8_t byte;
	uint16_t word;
	uint8_t block[I2C_SMBUS_BLOCK_MAX + 3]; /* block[0] is used for length */
                          /* one more for read length in block process call */
	                                            /* and one more for PEC */
};
int SetSlaveAddr(int fd,uint8_t addr);
int i2c_smbus_write_byte(int file,uint8_t value);
int i2c_smbus_read_byte(int file);
int SetSlaveAddr(int fd,uint8_t addr);
int is_it_device_node(char* devnode);
/*****************************************************************************/
int read_word(int fd,uint32_t addr, uint16_t *data)
{
	int ret=SetSlaveAddr(fd,(uint8_t)addr);
	if(ret != 0)
		return ret;
	uint8_t buff[16];
	if (I2C_READ(fd,buff,2) != 2)
		return -1;//device node read error
	*data=(uint16_t)(buff[0]<<8)|(uint16_t)buff[1];
	return 0;
}
int write_word(int fd,uint32_t addr, uint16_t data)
{
	int ret=SetSlaveAddr(fd,(uint8_t)addr);
	if(ret != 0)
		return ret;
	uint8_t buff[16];
	buff[0]=(uint8_t)(data>>8);
	buff[1]=data&0x00FF;
	if (I2C_WRITE(fd,buff,2) != 2)
		return -1;//device node write error
	return 0;
}
/*****************************************************************************/
int open_i2c_device(int *fd,char* dev_node)
{
	if(is_it_device_node(dev_node)==0)//check if it is a char-device-node
	{
		*fd = open(dev_node, O_RDWR);
		switch(errno)
		{
			case ENOENT :return -1;//DevOpened=RPC_SRV_RESULT_FILE_NOT_FOUND;break;
			case ENOTDIR:return -1;//DevOpened=RPC_SRV_RESULT_FILE_NOT_FOUND;break;
			case EACCES :return -1;//DevOpened=RPC_SRV_RESULT_DEVNODE_ACCERR;break;
			default:return 0;//DevOpened=RPC_SRV_RESULT_SUCCESS;ConnType=ADLIB_DEV_CONN_TYPE_DEVNODE;break;
		}
	}
	return -1;
}
/*****************************************************************************/
int read_byte(int fd,uint32_t addr, uint8_t *data)
{
	int ret=SetSlaveAddr(fd,(uint8_t)addr);
	if(ret != 0)
		return ret;

	int smret=i2c_smbus_read_byte(fd);
	if(smret<0)
		return -1;//device node read error
	else
		*data=(uint8_t)smret;
	return 0;
}
int write_byte(int fd,uint32_t addr, uint8_t data)
{
	int ret=SetSlaveAddr(fd,(uint8_t)addr);
	if(ret != 0)
		return ret;
	int smret=i2c_smbus_write_byte(fd,data);
	if(smret!=0)
		return -1;
	return 0;
}
/*****************************************************************************/
int32_t i2c_smbus_access(int file, char read_write, uint8_t command,int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;
	args.read_write = read_write;
	args.command = command;
	args.size = size;
	args.data = data;
	return ioctl(file,I2C_SMBUS,&args);
}
int i2c_smbus_read_byte(int file)
{
#ifdef I2C_ACCESS_TYPE_SMBUS
	union i2c_smbus_data data;
	if (i2c_smbus_access(file,I2C_SMBUS_READ,0,I2C_SMBUS_BYTE,&data))
		return -1;
	else
		return 0x0FF & data.byte;
	return 0;
#else
	uint8_t buff[16];
	return read(file,buff,1);
#endif
}
int i2c_smbus_write_byte(int file,uint8_t value)
{
#ifdef I2C_ACCESS_TYPE_SMBUS
	size_t sz=i2c_smbus_access(file,I2C_SMBUS_WRITE,value,
	                        I2C_SMBUS_BYTE,NULL);
	return sz;
#else
	uint8_t buff[16];buff[0]=value;
	size_t sz=write(file,buff,1);
	return sz;
#endif
}
/*****************************************************************************/
int SetSlaveAddr(int fd,uint8_t addr)
{
	if (ioctl(fd, I2C_SLAVE, addr) < 0)
		return -1;
	return 0;
}
/*****************************************************************************/
//checks if a given string is a character device node
int is_it_device_node(char* devnode)
{
	struct stat buf;
	stat(devnode, &buf);
	if(S_ISCHR(buf.st_mode))
		return 0;
	return -1;
}
/*****************************************************************************/
//Print out the report bytes
void ShowReport(IOWKIT28_SPECIAL_REPORT data, ULONG count, const char* text)
{
    if(count > IOWKIT28_SPECIAL_REPORT_SIZE)
        count = IOWKIT28_SPECIAL_REPORT_SIZE;
    printf("%s: ", text);

    for(ULONG i=0; i<count; i++)
        printf("%02X ", data.Bytes[i]);
    printf("\n");
}
/*****************************************************************************/
int main(int argc, char *argv[])
{
    IOWKIT_HANDLE handle;
    IOWKIT28_SPECIAL_REPORT report;
    int inspect=0;
	if(argv[1]!=NULL)
		if(strcmp(argv[1],"inspect")==0)
			inspect=1;
	int fd;
	if(open_i2c_device(&fd,I2C_DEVICE_NODE)==0)
	{
		uint16_t ltbl[20];
		ltbl[0]=0x0000;ltbl[1]=0x1111;ltbl[2]=0x2222;ltbl[3]=0x3333;ltbl[4]=0x4444;ltbl[5]=0x5555;ltbl[6]=0x6666;ltbl[7]=0x7777;
		ltbl[8]=0x8888;ltbl[9]=0x9999;ltbl[10]=0xAAAA;ltbl[11]=0xBBBB;ltbl[12]=0xCCCC;ltbl[13]=0xDDDD;
		ltbl[14]=0xEEEE;ltbl[15]=0xFFFF;
		for(int i=1;i<16;i++)
		{
			if(inspect==1)
			{
				write_byte(fd,0x20,0x00);//pcf8574 address is 0x20 and i2c-test-board addr is 0x50
				usleep(500000);
				write_byte(fd,0x20,0xff);
				usleep(500000);
			}
			else
			{
				write_word(fd,0x50,ltbl[i]);//i2c-test-board addr is 0x50
				usleep(500000);
			}
		}
		close(fd);
		return 0;
	}

    // Open device and use first founc device furhter
    handle = IowKitOpenDevice();
    if (handle == NULL)
    {
        printf("Failed to open device\n");
        return -1;
    }

    //Allow only an IO-Warrior56 to run programm
    if(IowKitGetProductId(handle) != IOWKIT_PID_IOW28)
    {
        printf("No IO-Warrior28 found\n");
        IowKitCloseDevice(handle);
        return 1;
    }

    //Set timeouts for write and read. default is INFINITE
    IowKitSetWriteTimeout(handle, 1000);
    IowKitSetTimeout(handle, 1000);

    //Enable I2C mode
    memset(&report, 0, IOWKIT28_SPECIAL_REPORT_SIZE);

    report.ReportID = 0x01;     //Special mode: I2C
    report.Bytes[0] = 0x01;     //Enable I2C mode
    report.Bytes[1] = 0x00;     //Flags: speed 100 kbit/s
    report.Bytes[2] = 0x00;     //Timeout

    IowKitWrite(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE);

    //Search for I2C devices by reading 1 byte and print out (no further use of i2c address)
    for (int i = 1; i < 127; i++)
    {
        memset(&report, 0, IOWKIT28_SPECIAL_REPORT_SIZE);
        report.ReportID = 0x03;             //I2C read
        report.Bytes[0] = 0x01;             //count
        report.Bytes[1] = (i << 1) | 0x01;  //Device address (8 bit + read bit)
        report.Bytes[2] = 0;                //Mcount

        if (IowKitWrite(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) != IOWKIT28_SPECIAL_REPORT_SIZE)
        {
            printf("ERROR on IowKitWrite(). CANCEL\n");
            break;
        }
        if (IowKitRead(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) != IOWKIT28_SPECIAL_REPORT_SIZE)
        {
            printf("ERROR on IowKitRead(). CANCEL\n");
            break;
        }
        //if (report.Bytes[0] == 0x01) printf("I2C device found (8Bit): 0x%02X\n", (i << 1));; //Read byte 0, if byte count = 1 -> device present
    }

	unsigned char verifyByte[3];
	verifyByte[1]=0x12;
	verifyByte[2]=0x36;
	unsigned short testCounter=0;
	unsigned char lut[20];
	if(inspect)
	{
		lut[0]=0x00;lut[1]=0xFF;lut[2]=0x00;lut[3]=0xFF;lut[4]=0x00;lut[5]=0xFF;
		lut[6]=0x00;lut[7]=0xFF;lut[8]=0x00;lut[9]=0xFF;lut[10]=0x00;lut[11]=0xFF;
		lut[12]=0x00;lut[13]=0xFF;lut[14]=0x00;lut[15]=0xFF;//blinking pattern for pcf8574
	}
	else
	{
		lut[0]=0x00;lut[1]=0x11;lut[2]=0x22;lut[3]=0x33;lut[4]=0x44;lut[5]=0x55;
		lut[6]=0x66;lut[7]=0x77;lut[8]=0x88;lut[9]=0x99;lut[10]=0xAA;lut[11]=0xBB;
		lut[12]=0xCC;lut[13]=0xDD;lut[14]=0xEE;lut[15]=0xFF;//counter pattern for i2c-test-board
	}

    for(testCounter=1;testCounter<16;testCounter++)
    {
        verifyByte[2]=lut[testCounter];
        verifyByte[1]=lut[testCounter];

        //Write two bytes to I2C device 0xA0 or 0x04
        memset(&report, 0x00, IOWKIT28_SPECIAL_REPORT_SIZE);
        report.ReportID = 0x02;     //I2C write
        if(inspect)
	{
	        report.Bytes[0] = 0xC2;     //Flags + byte count
		report.Bytes[1] = 0x40;     //I2C slave address of pcf8574 board
	}
	else
	{
	        report.Bytes[0] = 0xC3;     //Flags + byte count
		report.Bytes[1] = 0xa0;     //I2C slave address of i2c test board
	}
        report.Bytes[2] = verifyByte[1];     //I2C slave register / first data byte
        report.Bytes[3] = verifyByte[2];   //more data.....

        if (IowKitWrite(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) != IOWKIT28_SPECIAL_REPORT_SIZE)
        {
            printf("ERROR on IowKitWrite()\n");
            return -1;
        }

        if (IowKitRead(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) != IOWKIT28_SPECIAL_REPORT_SIZE)
        {
            printf("ERROR on IowKitRead()\n");
            return -1;
        }

        //Readback bytes to verify
        int count = 3;if(inspect) count=2;//for pcf8574 board byte count is 2
        memset(&report, 0x00, IOWKIT28_SPECIAL_REPORT_SIZE);
        report.ReportID = 0x03;         //I2C read
        report.Bytes[0] = count;        //Count
        if(inspect)
		report.Bytes[1] = 0x40 | 0x01;  //I2C slave address + read bit (8 bit) of pcf8574 address
	else
		report.Bytes[1] = 0xa0 | 0x01;  //I2C slave address + read bit (8 bit) of i2c test board
        report.Bytes[2] = 0;            //MCount

        if (IowKitWrite(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) == IOWKIT28_SPECIAL_REPORT_SIZE)
        {
            if (IowKitRead(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) == IOWKIT28_SPECIAL_REPORT_SIZE)
            {
                //verify bytes written.
                if( report.Bytes[1] != verifyByte[1])
                {
                    printf("verification failed on byte1\n");
                    //return -1;
                }
                if( report.Bytes[2] != verifyByte[2])
                {    
                    printf("verification failed on byte2\n");
                    //return -1;
                }
            }
            else
                printf("ERROR on IowKitRead()\n");
        }
        else
            printf("ERROR on IowKitWrite()\n");
 
        //wait 0.5sec before writing next count to i2c-slave
        usleep(500000);	
    }

    //Disable I2C mode (free IO-Pins)
    memset(&report, 0, IOWKIT28_SPECIAL_REPORT_SIZE);
    report.ReportID = 0x01;     //Special mode: I2C
    report.Bytes[0] = 0x00;     //Disable I2C mode
    report.Bytes[1] = 0x00;     //Flags: pullups ON, speed 93.75kHz
    IowKitWrite(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE);

    // Close device
    IowKitCloseDevice(handle);
    return 0;
}
