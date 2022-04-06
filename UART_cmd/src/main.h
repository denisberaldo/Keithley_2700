#ifndef MAIN_H_
#define MAIN_H_

void WriteCMD(int fd, char *cmd);
void ParseData(FILE *fd, uint8_t *ptr_data, uint32_t size);

#endif
