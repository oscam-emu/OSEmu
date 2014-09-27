int8_t ProcessECM(uint16_t CAID, const uint8_t *ecm, uint8_t *dw);
int8_t ProcessEMM(uint16_t caid, uint32_t provider, const uint8_t *emm, uint32_t *keysAdded);
void read_emu_keyfile(char *path);
#ifndef __APPLE__
void read_emu_keymemory(void);
#endif
uint32_t GetOSemuVersion(void);
