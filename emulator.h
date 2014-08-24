int8_t ProcessECM(uint16_t CAID, const uint8_t *ecm, uint8_t *dw);
void read_emu_keyfile(char *path);
#ifndef __APPLE__
void read_emu_keymemory(void);
#endif
