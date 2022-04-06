/* *****************************************************************************
 FILE_NAME:    	uart.c
 DESCRIPTION:   UART driver.
 DESIGNER:      Denis Beraldo
 CREATION_DATE: 20/02/2018
 VERSION:       1.0
********************************************************************************
Version 1.0:	20/02/2018 - Denis Beraldo
				- Initial version
***************************************************************************** */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <time.h>

#include "uart.h"

//#define PRINT_UART

static struct termios ttyS1;


void UART_Write(int Fd_Req, uint8_t *BuffWrite, uint16_t BuffSize)
{
	(void)write(Fd_Req, BuffWrite, BuffSize);
}

UartAnswer_t UART_Read(int Fd_Req, uint8_t *BuffRead, uint16_t *ReadBytes)
{
	uint8_t					rx_buffer[1024];
	int						rx_length	= 0;
	uint16_t				bytesCopied	= 0;

	static UartMachine_t	UartMachine	= UART_IDLE;
	UartAnswer_t			ret			= UART_EMPTY;

	int 					bytes		= 0;
	static int				lastbytes	= 0xFFFFFFFF;

	struct timespec			currTimespec;
	static uint64_t			endTimeout;
	uint64_t				currTime;

	if (Fd_Req != -1)
	{
		if (ioctl(Fd_Req, FIONREAD, &bytes) == -1)
		{
			#ifdef PRINT_UART
			printf("\nIoctl");
			fflush(stdout);
			#endif
		}
		else
		{
			switch (UartMachine)
			{
				case UART_IDLE:
				{
					if (bytes > 0)
					{
						#ifdef PRINT_UART
						printf("\nIdle 1 \t%d", bytes);
						fflush(stdout);
						#endif

						lastbytes = bytes;
						(void)clock_gettime(CLOCK_MONOTONIC_RAW, &currTimespec);

						endTimeout  = 0;
						endTimeout  = CONVERT_SEC_TO_NANOSEC(currTimespec.tv_sec);
						endTimeout += CONVERT_MILLI_TO_NANOSEC(END_FRAME_TIME);
						endTimeout += (uint64_t)currTimespec.tv_nsec;

						UartMachine = UART_RECEIVING;
					}
					else
					{
						#ifdef PRINT_UART
						printf("\n0 bytes");
						fflush(stdout);
						#endif
					}
				}
				break;

				case UART_RECEIVING:
				{
					(void)clock_gettime(CLOCK_MONOTONIC_RAW, &currTimespec);
					currTime = CONVERT_SEC_TO_NANOSEC(currTimespec.tv_sec) + (uint64_t)currTimespec.tv_nsec;

					if (bytes != lastbytes)
					{
						lastbytes = bytes;

						endTimeout  = 0;
						endTimeout  = CONVERT_SEC_TO_NANOSEC(currTimespec.tv_sec);
						endTimeout += CONVERT_MILLI_TO_NANOSEC(END_FRAME_TIME);
						endTimeout += (uint64_t)currTimespec.tv_nsec;

						#ifdef PRINT_UART
						printf("\nRec 1 - \t%d\t%llu\t%llu", bytes, currTime, endTimeout);
						fflush(stdout);
						#endif
					}
					else
					{
						if (currTime >= endTimeout)
						{
							#ifdef PRINT_UART
							printf("\nRec 2 tim - \t%d\t%llu\t%llu", bytes, currTime, endTimeout);
							fflush(stdout);
							#endif

							UartMachine = UART_END;
						}
					}
				}
				break;

				case UART_END:
				{
					while (1)
					{
						rx_length = read(Fd_Req, (void*)rx_buffer, 255);

						if (rx_length > 0)
						{
							(void)memcpy(&BuffRead[bytesCopied], &rx_buffer[0], rx_length);
							bytesCopied += rx_length;

							#ifdef PRINT_UART
							printf("\nCopy end - %d - %d", rx_length, bytesCopied);
							fflush(stdout);
							#endif
						}
						else
						{
							break;
						}
					}

					#ifdef PRINT_UART
					printf("\nReturn idle");
					fflush(stdout);
					#endif

					*ReadBytes = bytesCopied;
					bytesCopied = 0;
					bytes = 0;

					ret = UART_OK;
					UartMachine = UART_IDLE;
				}
				break;
			}
		}
	}
	else
	{
		printf("\nUART file descriptor invalid.");
		fflush(stdout);
		ret = UART_FAIL;
	}

	return ret;
}

int UART_Initialize(char *Port)
{
	int		Fd_temp;
	int		ret = 0;
	char	devBuffer[50];

	(void)sprintf(devBuffer, "sudo chmod 777 %s", Port);
	(void)system(devBuffer);

	/* open the port */
	Fd_temp = open(Port, O_RDWR | O_NOCTTY | O_NDELAY);

	fcntl(Fd_temp, F_SETFL, 0x04000);

	if (Fd_temp != -1)
	{
		memset (&ttyS1, 0, sizeof ttyS1);
		tcgetattr (Fd_temp, &ttyS1);

		cfsetospeed (&ttyS1, B115200);
		cfsetispeed (&ttyS1, B115200);

		ttyS1.c_cflag = (ttyS1.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
		ttyS1.c_iflag &= ~IGNBRK;         // disable break processing
		ttyS1.c_lflag = 0;                // no signaling chars, no echo,
										  // no canonical processing
		ttyS1.c_oflag = 0;                // no remapping, no delays
		ttyS1.c_cc[VMIN]  = 0;            // read doesn't block
		ttyS1.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

		ttyS1.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

		ttyS1.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable reading

		ttyS1.c_cflag &= ~(PARENB | PARODD);      // shut off parity
		ttyS1.c_cflag |= 0;
		ttyS1.c_cflag &= ~CSTOPB;
		ttyS1.c_cflag &= ~CRTSCTS;

		tcsetattr (Fd_temp, TCSANOW, &ttyS1);

		ret = Fd_temp;
	}
	else
	{
		ret = -1;
	}

	#ifdef PRINT_UART
	printf("UART Fd: %d", Fd_temp);
	fflush(stdout);
	#endif

	return ret;
}
void UART_SetParam(int Fd_Req, uint8_t Vmin, uint8_t Vtime)
{
	tcgetattr (Fd_Req, &ttyS1);

	ttyS1.c_cc[VMIN]  = Vmin;
	ttyS1.c_cc[VTIME] = Vtime;

	tcsetattr (Fd_Req, TCSANOW, &ttyS1);
}
