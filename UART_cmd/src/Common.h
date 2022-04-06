#ifndef COMMON_H_
#define COMMON_H_

#define SWAP_U16(_val)		(uint16_t)(((_val & 0x00FF) << 8) | ((_val & 0xFF00) >> 8))

uint8_t Checksum(uint8_t *PtrData, uint8_t SizeData);
int GetCurrent_ms(void);

#endif /* COMMON_H_ */
