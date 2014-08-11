char ProcessECM(uint16_t CAID, const unsigned char *ecm, unsigned char *dw);
void read_emu_keyfile(char *path);
#ifndef __APPLE__
void read_emu_keymemory(void);
#endif
