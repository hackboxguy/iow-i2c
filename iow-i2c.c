/*
* I2C sample for IO-Warrior28
* This sample use the first found IO-Warrior device on IowKitOpenDevice().
* The I2C device (address) which used in this sample may be changed for your device.
*
* IO-Warrior28 will only works with SDK version 1.7 or higher. Please check your linux kernel
* for the actual driver which includes IO-Warrior28. Driver was added March 2020 into the kernel.
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "iowkit.h"


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


int main(int argc, char *argv[])
{
    IOWKIT_HANDLE handle;
    IOWKIT28_SPECIAL_REPORT report;

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
	lut[0]=0x00;lut[1]=0x11;lut[2]=0x22;lut[3]=0x33;lut[4]=0x44;lut[5]=0x55;
	lut[6]=0x66;lut[7]=0x77;lut[8]=0x88;lut[9]=0x99;lut[10]=0xAA;lut[11]=0xBB;
	lut[12]=0xCC;lut[13]=0xDD;lut[14]=0xEE;lut[15]=0xFF;

    for(testCounter=1;testCounter<16;testCounter++)
    {
        verifyByte[2]=lut[testCounter];
        verifyByte[1]=lut[testCounter];

        //Write two bytes to I2C device[0xA0]
        memset(&report, 0x00, IOWKIT28_SPECIAL_REPORT_SIZE);
        report.ReportID = 0x02;     //I2C write
        report.Bytes[0] = 0xC3;     //Flags + byte count
        report.Bytes[1] = 0xa0;     //I2C slave address 8Bit (0x88 for sample)
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
        int count = 3;
        memset(&report, 0x00, IOWKIT28_SPECIAL_REPORT_SIZE);
        report.ReportID = 0x03;         //I2C read
        report.Bytes[0] = count;        //Count
        report.Bytes[1] = 0xa0 | 0x01;  //I2C slave address + read bit (8 bit) (0x88 for sample)
        report.Bytes[2] = 0;            //MCount

        if (IowKitWrite(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) == IOWKIT28_SPECIAL_REPORT_SIZE)
        {
            if (IowKitRead(handle, IOW_PIPE_I2C_MODE, (char *)&report, IOWKIT28_SPECIAL_REPORT_SIZE) == IOWKIT28_SPECIAL_REPORT_SIZE)
            {
                //verify bytes written.
                if( report.Bytes[1] != verifyByte[1])
                {
                    printf("verification failed on byte1\n");
                    return -1;
                }
                if( report.Bytes[2] != verifyByte[2])
                {    
                    printf("verification failed on byte2\n");
                    return -1;
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
