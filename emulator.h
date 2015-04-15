#ifndef OSEMU_EMULATOR_H_
#define OSEMU_EMULATOR_H_

int8_t ProcessECM(int16_t ecmDataLen, uint16_t caid, uint32_t UNUSED(provider), const uint8_t *ecm,
				  uint8_t *dw, uint16_t srvid, uint16_t ecmpid);
int8_t ProcessEMM(uint16_t caid, uint32_t provider, const uint8_t *emm, uint32_t *keysAdded);


void set_emu_keyfile_path(char *path);
uint8_t read_emu_keyfile(char *path);

#if !defined(__APPLE__) && !defined(__ANDROID__)
void read_emu_keymemory(void);
#endif


uint32_t GetOSemuVersion(void);

int32_t GetIrdeto2Hexserial(uint16_t caid, uint8_t *hexserial);

#endif
