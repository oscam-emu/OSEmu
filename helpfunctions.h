#ifndef OSEMU_HELPFUNCTIONS_H_
#define OSEMU_HELPFUNCTIONS_H_

bool cs_malloc(void *result, size_t size);

#define NULLFREE(X) {if (X) {void *tmpX=X; X=NULL; free(tmpX); }}

void cs_sleepms(uint32_t msec);

void cs_log_txt(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
char *cs_hexdump(int32_t m, const uchar *buf, int32_t n, char *target, int32_t len);
void cs_log_hex(const uint8_t *buf, int32_t n, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#define cs_log(fmt, params...)							cs_log_txt(fmt, ##params)
#define cs_log_dump(buf, n, fmt, params...)				cs_log_hex(buf,  n, fmt, ##params)
#define cs_log_dbg(a, fmt, params...)					do { if (debuglog) cs_log_txt(fmt, ##params); } while(0)
#define cs_log_dump_dbg(buf, n, fmt, params...)			do { if (debuglog) cs_log_hex(buf , n, fmt, ##params); } while(0)

int32_t boundary(int32_t exp, int32_t n);

uint32_t b2i(int32_t n, const uchar *b);
uchar *i2b_buf(int32_t n, uint32_t i, uchar *b);

void cs_strncpy(char *destination, const char *source, size_t num);

void get_random_bytes_init(void);
void get_random_bytes(uint8_t *dst, uint32_t dst_len);

uint32_t crc32(uint32_t crc, const unsigned char *buf, uint32_t len);
uint32_t fletcher_crc32(uint8_t *data, uint32_t len);

void aes_set_key(struct aes_keys *aes, char *key);
void aes_decrypt(struct aes_keys *aes, uchar *buf, int32_t n);
void aes_encrypt_idx(struct aes_keys *aes, uchar *buf, int32_t n);

#endif
