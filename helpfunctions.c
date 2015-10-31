#include "globals.h"
#include "helpfunctions.h"
#include <netdb.h>

extern uint32_t osemu_stacksize;

/* This function encapsulates malloc. It automatically adds an error message
   to the log if it failed and calls cs_exit(quiterror) if quiterror > -1.
   result will be automatically filled with the new memory position or NULL
   on failure. */
bool cs_malloc(void *result, size_t size)
{
	void **tmp = (void**)result;
	*tmp = (void*)malloc(size);
	if(*tmp == NULL)
	{
		fprintf(stderr, "%s: ERROR: Can't allocate %zu bytes!", __func__, size);
	}
	else
	{
		memset(*tmp, 0, size);
	}
	return !!*tmp;
}

void cs_sleepms(uint32_t msec)
{
	//does not interfere with signals like sleep and usleep do
	struct timespec req_ts;
	req_ts.tv_sec = msec / 1000;
	req_ts.tv_nsec = (msec % 1000) * 1000000L;
	int32_t olderrno = errno; // Some OS (especially MacOSX) seem to set errno to ETIMEDOUT when sleeping
	while (1)
	{
		/* Sleep for the time specified in req_ts. If interrupted by a
		signal, place the remaining time left to sleep back into req_ts. */
		int rval = nanosleep (&req_ts, &req_ts);
		if (rval == 0)
			break; // Completed the entire sleep time; all done.
		else if (errno == EINTR)
			continue; // Interrupted by a signal. Try again.
		else 
			break; // Some other error; bail out.
	}
	errno = olderrno;
}

void cs_log_txt(const char* format, ... ) {

	FILE *fp = NULL;
	if(havelogfile) { fp = fopen(logfile, "a"); }

	va_list ap, ap2;
	va_start(ap, format);
	va_copy(ap2, ap);

	if(!bg) { vfprintf(stderr, format, ap); }
	va_end(ap);

	if(fp) { vfprintf(fp, format, ap2); }
	va_end(ap2);

	if(!bg) { fprintf(stderr, "\n"); }
	if(fp)  { fprintf(fp, "\n"); fclose(fp); }
}

char *cs_hexdump(int32_t m, const uint8_t *buf, int32_t n, char *target, int32_t len)
{
	int32_t i = 0;
	target[0] = '\0';
	m = m ? 3 : 2;
	if(m * n >= len)
	{ n = (len / m) - 1; }
	while(i < n)
	{
		snprintf(target + (m * i), len - (m * i), "%02X%s", *buf++, m > 2 ? " " : "");
		i++;
	}
	return target;
}

void cs_log_hex(const uint8_t *buf, int32_t n, const char *fmt, ...)
{
	FILE *fp = NULL;
	char log_txt[512];
	const char *newline;
	int32_t i;

	newline = "\n";
	
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(log_txt, sizeof(log_txt), fmt, ap);
	va_end(ap);

	if(havelogfile) { fp = fopen(logfile, "a"); }

	if(!bg)
	{
		fwrite(log_txt, sizeof(char), strlen(log_txt), stderr);
		fwrite(newline, sizeof(char), strlen(newline), stderr);
	}
	if(fp)
	{
		fwrite(log_txt, sizeof(char), strlen(log_txt), fp);
		fwrite(newline, sizeof(char), strlen(newline), fp);
	}

	if(buf)
	{
		for(i = 0; i < n; i += 16)
		{
			cs_hexdump(1, buf + i, (n - i > 16) ? 16 : n - i, log_txt, sizeof(log_txt));
			if(!bg)
			{
				fwrite(log_txt, sizeof(char), strlen(log_txt), stderr);
				fwrite(newline, sizeof(char), strlen(newline), stderr);
			}
			if(fp) {
				fwrite(log_txt, sizeof(char), strlen(log_txt), fp);
				fwrite(newline, sizeof(char), strlen(newline), fp);
			}
		}
	}

	if(fp) { fclose(fp); }
}


int32_t boundary(int32_t exp, int32_t n)
{
	return (((n-1) >> exp) + 1) << exp;
}

uint32_t b2i(int32_t n, const uint8_t *b)
{
	switch(n) {
	case 2:
		return  (b[0] <<  8) |  b[1];
	case 3:
		return  (b[0] << 16) | (b[1] <<  8) |  b[2];
	case 4:
		return ((b[0] << 24) | (b[1] << 16) | (b[2] <<8 ) | b[3]) & 0xffffffff;
	default:
		cs_log("Error in b2i, n=%i",n);
	}
	return 0;
}

uint8_t *i2b_buf(int32_t n, uint32_t i, uint8_t *b)
{
	switch(n) {
	case 2:
		b[0] = (i>> 8) & 0xff;
		b[1] = (i    ) & 0xff;
		break;
	case 3:
		b[0] = (i>>16) & 0xff;
		b[1] = (i>> 8) & 0xff;
		b[2] = (i    ) & 0xff;
	case 4:
		b[0] = (i>>24) & 0xff;
		b[1] = (i>>16) & 0xff;
		b[2] = (i>> 8) & 0xff;
		b[3] = (i    ) & 0xff;
		break;
	}
	return b;
}

/* Ordinary strncpy does not terminate the string if the source is exactly
   as long or longer as the specified size. This can raise security issues.
   This function is a replacement which makes sure that a \0 is always added.
   num should be the real size of char array (do not subtract -1). */
void cs_strncpy(char *destination, const char *source, size_t num)
{
	if (!source) {
		destination[0] = '\0';
		return;
	}
	uint32_t l, size = strlen(source);
	if (size > num - 1)
	{ l = num - 1; }
	else
	{ l = size; }
	memcpy(destination, source, l);
	destination[l] = '\0';
}

/* CRYPTO */

#define RAND_POOL_SIZE 64

// The last bytes are used to init random seed
static uint8_t rand_pool[RAND_POOL_SIZE + sizeof(uint32_t)];

void get_random_bytes_init(void) {
	srand(time(NULL));
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		fd = open("/dev/random", O_RDONLY);
		if (fd < 0)
		{ return; }
	}
	if (read(fd, rand_pool, RAND_POOL_SIZE + sizeof(uint32_t)) > -1) {
		uint32_t pool_seed = b2i(4, rand_pool + RAND_POOL_SIZE);
		srand(pool_seed);
	}
	close(fd);
}

void get_random_bytes(uint8_t *dst, uint32_t dst_len) {
	static uint32_t rand_pool_pos; // *MUST* be static
	uint32_t i;
	for (i = 0; i < dst_len; i++) {
		rand_pool_pos++; // Races are welcome...
		dst[i] = rand() ^ rand_pool[rand_pool_pos % RAND_POOL_SIZE];
	}
}

/*
 * crc32 -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-1996 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
static const uint32_t crc_table[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
	0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
	0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
	0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
	0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
	0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
	0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
	0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
	0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
	0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
	0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
	0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
	0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
	0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
	0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
	0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
	0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
	0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
	0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
	0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
	0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
	0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
	0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
	0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
	0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
	0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
	0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
	0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
	0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
	0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
	0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
	0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
	0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
	0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
	0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
	0x2d02ef8d
};

#define DO1(buf) crc = crc_table[((int32_t)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf) DO1(buf); DO1(buf);
#define DO4(buf) DO2(buf); DO2(buf);
#define DO8(buf) DO4(buf); DO4(buf);

uint32_t crc32(uint32_t crc, const unsigned char *buf, uint32_t len)
{
	if (!buf)
	{ return 0; }
	crc = crc ^ 0xffffffff;
	while (len >= 8) {
		DO8(buf);
		len -= 8;
	}
	if (len) {
		do {
			DO1(buf);
		}
		while (--len);
	}
	return crc ^ 0xffffffff;
}

static uint32_t fletcher_crc_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

uint32_t fletcher_crc32(uint8_t *data, uint32_t len)
{
	uint32_t i;
	uint32_t crc = 0xffffffff;

	for (i=0; i<len; i++)
	{ crc = (crc << 8) ^ fletcher_crc_table[((crc >> 24) ^ *data++) & 0xff]; }

	return crc;
}

void aes_set_key(struct aes_keys *aes, char *key)
{
	AES_set_decrypt_key((const unsigned char *)key, 128, &aes->aeskey_decrypt);
	AES_set_encrypt_key((const unsigned char *)key, 128, &aes->aeskey_encrypt);
}

void aes_decrypt(struct aes_keys *aes, uint8_t *buf, int32_t n)
{
	int32_t i;
	for (i=0; i+15<n; i+=16) {
		AES_decrypt(buf+i, buf+i, &aes->aeskey_decrypt);
	}
}

void aes_encrypt_idx(struct aes_keys *aes, uint8_t *buf, int32_t n)
{
	int32_t i;
	for (i=0; i+15<n; i+=16) {
		AES_encrypt(buf+i, buf+i, &aes->aeskey_encrypt);
	}
}

uint32_t cs_getIPfromHost(const char *hostname)
{
	uint32_t result = 0;

	struct addrinfo hints, *res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;
	hints.ai_protocol = IPPROTO_TCP;

	int32_t err = getaddrinfo(hostname, NULL, &hints, &res);
	if(err != 0 || !res || !res->ai_addr)
	{
		cs_log("can't resolve %s, error: %s", hostname, err ? gai_strerror(err) : "unknown");
	}
	else
	{
		result = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
	}
	if(res) { freeaddrinfo(res); }

	return result;
}

#ifdef IPV6SUPPORT
void cs_getIPv6fromHost(const char *hostname, struct in6_addr *addr, struct sockaddr_storage *sa, socklen_t *sa_len)
{
	uint32_t ipv4addr = 0;
	struct addrinfo hints, *res = NULL;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	int32_t err = getaddrinfo(hostname, NULL, &hints, &res);
	if(err != 0 || !res || !res->ai_addr)
	{
		cs_log("can't resolve %s, error: %s", hostname, err ? gai_strerror(err) : "unknown");
	}
	else
	{
		ipv4addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
		if(res->ai_family == AF_INET)
			{ cs_in6addr_ipv4map(addr, ipv4addr); }
		else
			{ IP_ASSIGN(*addr, SIN_GET_ADDR(*res->ai_addr)); }
		if(sa)
			{ memcpy(sa, res->ai_addr, res->ai_addrlen); }
		if(sa_len)
			{ *sa_len = res->ai_addrlen; }
	}
	if(res)
		{ freeaddrinfo(res); }
}
#endif

void cs_resolve(const char *hostname, IN_ADDR_T *ip, struct SOCKADDR *sock, socklen_t *sa_len)
{
#ifdef IPV6SUPPORT
	cs_getIPv6fromHost(hostname, ip, sock, sa_len);
#else
	*ip = cs_getIPfromHost(hostname);
	if(sa_len)
		{ *sa_len = sizeof(*sock); }
#endif
}

/* Starts a thread named nameroutine with the start function startroutine. */
int32_t start_thread(const char *nameroutine, thread_func startroutine, void *arg, pthread_t *pthread, int8_t detach, int8_t modify_stacksize)
{
	pthread_t temp;
	pthread_attr_t attr;
	
	cs_log_dbg(D_TRACE, "starting thread %s", nameroutine);

	SAFE_ATTR_INIT(&attr);
	
	if(modify_stacksize)
 		{ SAFE_ATTR_SETSTACKSIZE(&attr, osemu_stacksize); }
 		
	int32_t ret = pthread_create(pthread == NULL ? &temp : pthread, &attr, startroutine, arg);
	if(ret)
		{ cs_log("ERROR: can't create %s thread (errno=%d %s)", nameroutine, ret, strerror(ret)); }
	else
	{
		cs_log_dbg(D_TRACE, "%s thread started", nameroutine);
		
		if(detach)
			{ pthread_detach(pthread == NULL ? temp : *pthread); }
	}

	pthread_attr_destroy(&attr);

	return ret;
}

static inline uint8_t to_uchar(char ch)
{
	return ch;
}


#define BASE64_LENGTH(inlen) ((((inlen) + 2) / 3) * 4)

void base64_encode(const char *in, size_t inlen, char *out, size_t outlen)
{
	static const char b64str[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	while(inlen && outlen)
	{
		*out++ = b64str[(to_uchar(in[0]) >> 2) & 0x3f];
		if(!--outlen) { break; }
		*out++ = b64str[((to_uchar(in[0]) << 4) + (--inlen ? to_uchar(in[1]) >> 4 : 0)) & 0x3f];
		if(!--outlen) { break; }
		*out++ = (inlen ? b64str[((to_uchar(in[1]) << 2) + (--inlen ? to_uchar(in[2]) >> 6 : 0)) & 0x3f] : '=');
		if(!--outlen) { break; }
		*out++ = inlen ? b64str[to_uchar(in[2]) & 0x3f] : '=';
		if(!--outlen) { break; }
		if(inlen) { inlen--; }
		if(inlen) { in += 3; }
		if(outlen) { *out = '\0'; }
	}
}

size_t b64encode(const char *in, size_t inlen, char **out)
{
	size_t outlen = 1 + BASE64_LENGTH(inlen);
	if(inlen > outlen)
	{
		*out = NULL;
		return 0;
	}
	if(!cs_malloc(out, outlen))
		{ return -1; }
	base64_encode(in, inlen, *out, outlen);
	return outlen - 1;
}
