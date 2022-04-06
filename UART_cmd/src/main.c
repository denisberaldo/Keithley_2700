#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "Types.h"
#include "main.h"
#include "data.h"

uint8_t recBuffer[1024];

int main(int argc, char **argv)
{
	int F_SETFL_PARAM;
	FILE	*fdLog;
	int		fd;
	char	devBuffer[50];
	int		status;
	int		n;
	int		pos = 0;
	char	last;
	int		timeout = 0;
	int		SentCount = 0;
	static struct termios tty;

	bool		is_finished = false;
	uint8_t 	*ptr_data;
	uint32_t	pos_data = 0;

	int maxfd;

	/* set the timeout as 1 sec for each read */
	struct timeval tm;

	fd_set readSet;

	tm.tv_sec	= 60;
	tm.tv_usec	= 0;

	ptr_data = malloc(5 * 1024 * 1024);

	(void)n;
	(void)sprintf(devBuffer, "sudo chmod a+rw %s", argv[1]);
	(void)system(devBuffer);

//	int rw = access(argv[1], R_OK | W_OK);
//
//	if (rw != 0)
//	{
//		printf("\nPermission error - %s\n", argv[1]);
//		fflush(stdout);
//		exit(0);
//	}


	fdLog = fopen("./output.log", "w+");

	if (fdLog == NULL)
	{
		printf("\nLog file descriptor invalid.\n");
		exit(0);
	}

	/* open the port */
	fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);

	F_SETFL_PARAM = 0x04000;
	fcntl(fd, F_SETFL, 0x04000);

	if (fd != -1)
	{
		memset (&tty, 0, sizeof tty);
		tcgetattr (fd, &tty);

		cfsetospeed (&tty, B19200);
		cfsetispeed (&tty, B19200);

		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
		tty.c_iflag &= ~IGNBRK;         // disable break processing
		tty.c_lflag = 0;                // no signaling chars, no echo,
										  // no canonical processing
		tty.c_oflag = 0;                // no remapping, no delays
		tty.c_cc[VMIN]  = 0;            // read doesn't block
		tty.c_cc[VTIME] = 50;            // 0.5 seconds read timeout

		tty.c_iflag &= ~(IXON | IXOFF | IXANY | INLCR | IGNCR | ICRNL); // shut off xon/xoff ctrl

		tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable reading

		tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
		tty.c_cflag |= 0;
		tty.c_cflag &= ~CSTOPB;
		tty.c_cflag &= ~CRTSCTS;

		tcsetattr (fd, TCSANOW, &tty);

		printf("\nUART opened: %s", argv[1]);
		fflush(stdout);

		maxfd = fd + 1;
	}


	printf("\n");

	#define		MEAS_TYPE	"VOLT"


	WriteCMD(fd, "*RST");

	WriteCMD(fd, ":INITiate:CONTinuous OFF");

	WriteCMD(fd, "ABOR");

	WriteCMD(fd, "FUNC '" MEAS_TYPE ":DC'");
	WriteCMD(fd, MEAS_TYPE ":DC:RANG 10");
	WriteCMD(fd, MEAS_TYPE ":DC:DIG 4");
	WriteCMD(fd, MEAS_TYPE ":DC:NPLC 0.01");


	WriteCMD(fd, "DISP:ENAB OFF");
	WriteCMD(fd, "SYST:AZER:STAT OFF");
	WriteCMD(fd, "SENS:" MEAS_TYPE ":DC:AVER:STAT OFF");


	WriteCMD(fd, "TRIG:SOUR IMM");

	//WriteCMD(fd, "TRIG:DEL 0.1");

	WriteCMD(fd, "SAMP:COUN 400");
	WriteCMD(fd, "DISP:TEXT:STAT OFF");
	WriteCMD(fd, "DISP:TEXT:DATA \"abc\"");

	WriteCMD(fd, ":INITiate");

	//WriteCMD(fd, "*TRG");

	printf("\n\n\nSleeping 10s!!!\n\n\n");
	fflush(stdout);

	sleep(10);


	WriteCMD(fd, "TRAC:DATA?");
	//WriteCMD(fd, "READ?");

	printf("\n\n\nREADY!!!\n\n\n");
	fflush(stdout);

	while(1)
	{
        FD_ZERO(&readSet);
        FD_SET(fd, &readSet);

		//usleep(1000);

        if (select(maxfd, &readSet, NULL, NULL, &tm) > 0)
        {
            if (FD_ISSET(fd, &readSet))
            {
				n = read(fd, (void*)recBuffer, 255);

				tm.tv_sec = 5;

				if (n > 0)
				{
					for (int j = 0; j < n; ++j)
					{
						if (recBuffer[j] == '#')
						{
							printf("\n");
							fflush(stdout);
						}
						//if (recBuffer[j] == ',') { recBuffer[j] = ';'; 		}
						//if (recBuffer[j] == '.') { recBuffer[j] = ',';		}

						//if (recBuffer[j] == 'A') { recBuffer[j] = ';';		}
						//if (recBuffer[j] == 0x13) { recBuffer[j] = ',';		}
						//if (recBuffer[j] == 0x11) { recBuffer[j] = '\n';	}
					}

					(void)memcpy(&ptr_data[pos_data], &recBuffer[0], n);
					pos_data += n;

					#if 0
					printf("\nrec %d - %s", n, recBuffer);
					for (int j = 0; j < n; ++j) { printf(" \t %.2x", recBuffer[j]); }
					fflush(stdout);
					#else
					printf("%s", recBuffer);
					fflush(stdout);
					#endif

					memset(recBuffer, 0x00, sizeof(recBuffer));

					if (is_finished == true)
					{
						break;
					}
				}
				else
				{
					printf("\nrec %d", n);
					fflush(stdout);
				}
			}
		}
		else
		{
			printf("\nSelect\n");
			fflush(stdout);
			break;
		}
	}

	printf("\n***************************************************************");
	ParseData(fdLog, ptr_data, pos_data);
	printf("\n***************************************************************\n");

	fflush(fdLog);
	fclose(fdLog);

	printf("\nFinished\n\n");
	fflush(stdout);

	return 0;
}

void ParseData(FILE *fd, uint8_t *ptr_data, uint32_t size)
{
	uint32_t	startpos = 0;
	char 		bufflocal[64];
	int			a = 0;

	uint8_t		valendpos;
	char		buffvalue[3][32];
	uint8_t		arraypos = 0;

	for (uint32_t j = 0; j < size; ++j)
	{
		if ((ptr_data[j] == '#') && (ptr_data[j + 1] == ','))
		{
			(void)memset(buffvalue, 0x00, sizeof(buffvalue));
			(void)memset(&bufflocal[0], 0x00, sizeof(bufflocal));
			(void)memcpy(&bufflocal[0], &ptr_data[startpos], (j + 2 - startpos));

			startpos = j + 2;

			for (int n = strlen(bufflocal) - 2; n >= 0; --n)
			{
				if (arraypos < 2)
				{
					if ((bufflocal[n] >= 'A') && (bufflocal[n] <= 'Z'))
					{
						valendpos = n;
					}

					if (bufflocal[n] == ',')
					{
						(void)memcpy(&buffvalue[arraypos++][0], &bufflocal[n + 1], (valendpos - 1 - n));
						a++;
					}
				}
				else
				{
					if (bufflocal[n + 1] == ',')
					{
						valendpos = n;
					}
				}

				if (n == 0)
				{
					if ((bufflocal[n] == '+') && (bufflocal[n + 1] == '9') && (bufflocal[n + 2] == '.') && (bufflocal[n + 3] == '9'))
					{
						sprintf(&buffvalue[arraypos++][0], "+127E-03");
					}
					else
					{
						(void)memcpy(&buffvalue[arraypos++][0], &bufflocal[n], (valendpos - 2 - n));
					}
					arraypos = 0;

					printf("\n%s;%s;%s;", &buffvalue[0][0], &buffvalue[1][0], &buffvalue[2][0]);
					fflush(stdout);

					fprintf(fd, "%s;%s;%s;\n", &buffvalue[0][0], &buffvalue[1][0], &buffvalue[2][0]);
				}
			}
		}
	}
}

void WriteCMD(int fd, char *cmd)
{
	usleep(100000);
	write(fd, cmd, strlen(cmd));
	write(fd, "\n", 1);
}

