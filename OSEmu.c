#include "globals.h"
#include "helpfunctions.h"
#include "emulator.h"

static const unsigned char cl_user[64];
static const unsigned char cl_passwd[64];
static struct aes_keys cl_aes_keys;
static const uchar cl_ucrc[4];

static int cl_sockfd;
static struct sockaddr_in cl_socket;

#define REQ_SIZE	584		// 512 + 20 + 0x34

static int32_t do_daemon(int32_t nochdir, int32_t noclose)
{
	int32_t fd;

	switch (fork())
	{
		case -1: return (-1);
		case 0:  break;
		default: _exit(0);
	}

	if (setsid() == (-1))
	return (-1);

	if (!nochdir)
	(void)chdir("/");

	if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1)
	{
		(void)dup2(fd, STDIN_FILENO);
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
	if (fd > 2)
		(void)close(fd);
	}
	return (0);
}

static int32_t camd35_send(uchar *buf, int32_t buflen)
{
	int32_t l;
	unsigned char rbuf[REQ_SIZE+15+4], *sbuf = rbuf + 4;

	//Fix ECM len > 255
	if (buflen <= 0)
		buflen = ((buf[0] == 0)? (((buf[21]&0x0f)<< 8) | buf[22])+3 : buf[1]);
	l = 20 + (((buf[0] == 3) || (buf[0] == 4)) ? 0x34 : 0) + buflen;
	memcpy(rbuf, cl_ucrc, 4);
	memcpy(sbuf, buf, l);
	memset(sbuf + l, 0xff, 15);	// set unused space to 0xff for newer camd3's
	i2b_buf(4, crc32(0L, sbuf+20, buflen), sbuf + 4);
	l = boundary(4, l);
	cs_log_debug("send %d bytes to client", l);
	aes_encrypt_idx(&cl_aes_keys, sbuf, l);

	int32_t status;
	socklen_t len = sizeof(cl_socket);
	status = sendto(cl_sockfd, rbuf, l+4, 0, (struct sockaddr *)&cl_socket, len);
	return status;
}

static int32_t camd35_auth_client(uchar *ucrc)
{
  int32_t rc=1;
  uint32_t crc;
  unsigned char md5tmp[MD5_DIGEST_LENGTH];
  crc=(((ucrc[0]<<24) | (ucrc[1]<<16) | (ucrc[2]<<8) | ucrc[3]) & 0xffffffffL);
  if (crc==crc32(0L, MD5(cl_user, strlen(cl_user), md5tmp), MD5_DIGEST_LENGTH)){
		memcpy(&cl_ucrc, ucrc, 4);
  	return 0;
  }
  return(1);
}

static int32_t camd35_recv(uchar *buf, int32_t rs)
{
	int32_t rc = 0, s, n=0, buflen=0, len=0;
	for (s=0; !rc; s++) {
		switch(s) {
			case 0:
				if (rs < 36) {
					rc = -1;
					goto out;
				}
				break;
			case 1:
				switch (camd35_auth_client(buf)) {
					case  0:        break;	// ok
					case  1: rc=-2; break;	// unknown user
				}
				memmove(buf, buf+4, rs-=4);
				break;
			case 2:
				aes_decrypt(&cl_aes_keys, buf, rs);
				if (rs!=boundary(4, rs))
					cs_log_debug("WARNING: packet size has wrong decryption boundary");

				n=(buf[0]==3) ? 0x34 : 0;

				//Fix for ECM request size > 255 (use ecm length field)
				if(buf[0]==0)
					buflen = (((buf[21]&0x0f)<< 8) | buf[22])+3;
				else
					buflen = buf[1];

				n = boundary(4, n+20+buflen);

				cs_log_debug("received %d bytes from client", rs);

				if (n<rs)
					cs_log_debug("ignoring %d bytes of garbage", rs-n);
				else
					if (n>rs) rc=-3;
				break;
			case 3:
				if (crc32(0L, buf+20, buflen)!=b2i(4, buf+4)) rc=-4;
				if (!rc) rc=n;
				break;
		}
	}

out:
	switch(rc) {
		case -2:	cs_log("unknown user");			break;
		case -3:	cs_log("incomplete request!");			break;
		case -4:	cs_log("checksum error (wrong password?)");	break;
	}

	return(rc);
}

static void camd35_process_ecm(uchar *buf, int buflen){
	ECM_REQUEST er;
	if (!buf || buflen < 23)
		return;
	uint16_t ecmlen = (((buf[21] & 0x0f)<< 8) | buf[22])+3;
	if (ecmlen + 20 > buflen)
		return;
	er.ecmlen = ecmlen;
	er.srvid = b2i(2, buf+ 8);
	er.caid = b2i(2, buf+10);
	er.prid = b2i(4, buf+12);
	er.rc = buf[3];
	
	ProcessECM(er.caid,buf + 20,er.cw);

	if (er.rc == E_STOPPED) {
		buf[0] = 0x08;
		buf[1] = 2;
		buf[20] = 0;
		/*
		* the second Databyte should be forseen for a sleeptime in minutes
		* whoever knows the camd3 protocol related to CMD08 - please help!
		* on tests this don't work with native camd3
		*/
		buf[21] = 0;
	}
	if ((er.rc == E_NOTFOUND) || (er.rc == E_INVALID)) {
		buf[0] = 0x08;
		buf[1] = 2;
		memset(buf + 20, 0, buf[1]);
		buf[22] = er.rc; //put rc in byte 22 - hopefully don't break legacy camd3
	} else {
		if (buf[0]==3)
			memmove(buf + 20 + 16, buf + 20 + buf[1], 0x34);
		buf[0]++;
		buf[1] = 16;
		memcpy(buf+20, er.cw, buf[1]);
	}
	camd35_send(buf, 0);
}

void show_usage(char *cmdline){
	fprintf(stderr, "Usage: %s -a <user>:<password> -p <port> [-b -v]\n", cmdline);
	fprintf(stderr, "-b enables to start as a daemon (background)\n");;
	fprintf(stderr, "-v enables a more verbose output (debug output)\n");
}

int main(int argc, char**argv)
{
	int bg = 0, n, opt, port = 0, accountok = 0;
	struct sockaddr_in servaddr;
	socklen_t len;
	unsigned char mbuf[1000];
	unsigned char md5tmp[MD5_DIGEST_LENGTH];
	
	while ((opt = getopt(argc, argv, "bva:p:")) != -1) {
		switch (opt) {
			case 'b':
				bg = 1;
				break;
			case 'a': {
				char *ptr = strtok(optarg, ":");
				cs_strncpy((char *)&cl_user, ptr, sizeof(cl_user));
				ptr = strtok(NULL, ":");
				if(ptr) {
					cs_strncpy((char *)&cl_passwd, ptr, sizeof(cl_passwd));
					accountok = 1;
				}
				break;
			}
			case 'p':
				port = atoi(optarg);
				break;
			case 'v':
				debuglog = 1;
				break;
			default:
				show_usage(argv[0]);
				exit(0);
		}
	}
	if(port == 0 || accountok == 0){
		show_usage(argv[0]);
		exit(0);
	}

 
	if (bg && do_daemon(1, 0))
	{
		fprintf(stderr, "Couldn't start as a daemon\n");	
		exit(0);
	}
	
	get_random_bytes_init();
	
	cl_sockfd = socket(AF_INET,SOCK_DGRAM,0);
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	bind(cl_sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	
	aes_set_key(&cl_aes_keys, (char *) MD5(cl_passwd, strlen((char *)cl_passwd), md5tmp));
	
	for (;;){
		len = sizeof(cl_socket);
		n = recvfrom(cl_sockfd,mbuf,sizeof(mbuf),0,(struct sockaddr *)&cl_socket,&len);
		
		if (camd35_recv(mbuf, n) >= 0 ){
		if(mbuf[0] == 0 || mbuf[0] == 3) {
			camd35_process_ecm(mbuf, n);
		} else {
			cs_log("unknown/not implemented camd35 command! (%d) n=%d", mbuf[0], n);
			}
		}
	}
}
