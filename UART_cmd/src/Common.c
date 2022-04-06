#include <string.h>
#include <stdbool.h>
#include <stdint-gcc.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include "Common.h"

uint8_t Checksum(uint8_t *PtrData, uint8_t SizeData)
{
	uint8_t i;
	uint8_t Sum = 0;

	for (i = 0; i < SizeData; i++)
	{
		Sum += *PtrData;
		PtrData++;
	}

	return (0xFF - Sum);
}

int GetCurrent_ms(void)
{
	static int		s_init;
	static bool		f_init = false;
	long            ms; // Milliseconds
	time_t          s;  // Seconds
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);

	if (f_init == false)
	{
		f_init = true;
		s = 0;
		s_init = spec.tv_sec;
	}
	else
	{
		s = spec.tv_sec - s_init;
	}

	ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

	if (ms > 999)
	{
		s++;
		ms = 0;
	}

	//printf("\nCurrent time: %d.%03ld", (int)s, ms);

	return (int)((s * 1000) + (int)ms);
}
