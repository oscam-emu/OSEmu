char ProcessECM(uint16_t CAID, const unsigned char *ecm, unsigned char *dw);
void ReadKeyFile(char *path);
#ifndef __APPLE__
void ReadKeyMemory(void);
#endif