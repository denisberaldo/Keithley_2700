#ifndef UART_INCLUDE
#define UART_INCLUDE

#include <stdint.h>

/* End frame indication.		*/
#define END_FRAME_TIME			50							/* in milliseconds */

#define CONVERT_SEC_TO_NANOSEC(__Sec)		\
							(uint64_t)__Sec * (uint64_t)1000000000

#define CONVERT_MILLI_TO_NANOSEC(__Sec)		\
							(uint64_t)__Sec * (uint64_t)1000000


/* Internal Uart machine. */
typedef enum
{
	UART_IDLE		,
	UART_RECEIVING	,
	UART_END		,
} UartMachine_t;

/* Uart answer. */
typedef enum
{
	UART_UNDEF		,
	UART_PROCESSING	,
	UART_OK			,
	UART_FAIL		,
	UART_EMPTY		,
} UartAnswer_t;

void			UART_Write(int Fd_Req, uint8_t *BuffWrite, uint16_t BuffSize);
UartAnswer_t	UART_Read(int Fd_Req, uint8_t *BuffRead, uint16_t *ReadBytes);
void			UART_SetParam(int Fd_Req, uint8_t Vmin, uint8_t Vtime);
int				UART_Initialize(char *Port);

#endif
