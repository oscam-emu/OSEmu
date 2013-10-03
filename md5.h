#define MD5_DIGEST_LENGTH 16

typedef struct MD5Context {
	uint32_t buf[4];
	uint32_t bits[2];
	unsigned char in[64];
} MD5_CTX;

unsigned char *MD5(const unsigned char *input, unsigned long len, unsigned char *output_hash);
char * __md5_crypt(const char *text_pass, const char *salt, char *crypted_passwd);
void MD5_Init(MD5_CTX *ctx);
void MD5_Update(MD5_CTX *ctx, const unsigned char *buf, unsigned int len);
void MD5_Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5_CTX *ctx);