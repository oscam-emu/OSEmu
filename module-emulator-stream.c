#include "globals.h"

#ifdef WITH_EMU
#include "cscrypt/des.h"
#include "oscam-string.h"
#include "oscam-config.h"
#include "oscam-time.h"
#else
#include "des.h"
#endif

#include "ffdecsa/ffdecsa.h"
#include "module-emulator-osemu.h"
#include "module-emulator-stream.h"

extern int32_t exit_oscam;
int8_t stream_server_thread_init = 0;
int32_t emu_stream_source_port = 8001;
int32_t emu_stream_relay_port = 17999;

#ifdef WITH_EMU
pthread_mutex_t emu_fixed_key_data_mutex;
emu_stream_client_data emu_fixed_key_data;
uint16_t emu_stream_cur_srvid = 0;
LLIST *ll_emu_stream_delayed_keys;
int8_t stream_server_has_ecm = 0;
#endif

static int32_t glistenfd, gconnfd;

static uint32_t CheckTsPackets(uint32_t packetSize, uint8_t *buf, uint32_t bufLength, uint16_t *packetCount)
{
	uint32_t i;
	(*packetCount) = 0;
	
	for(i=packetSize; i<bufLength; i+=packetSize) {
		if(buf[i] == 0x47) {
			(*packetCount)++;
		}	
	}
	
	return (*packetCount);
}

static void SearchTsPackets(uint8_t *buf, uint32_t bufLength, uint16_t *packetCount, uint16_t *packetSize, uint16_t *startOffset)
{
	uint32_t i;
	
	(*packetCount) = 0;
	(*packetSize) = 0;
	(*startOffset) = 0;

	for(i=0; i<bufLength; i++) {	
		if(buf[i] == 0x47) {
			if(CheckTsPackets(188, buf+i, bufLength-i, packetCount)) {
				(*packetSize) = 188;
				(*startOffset) = i;
				return;
			}
			else if(CheckTsPackets(204, buf+i, bufLength-i, packetCount)) {
				(*packetSize) = 204;
				(*startOffset) = i;
				return;
			}
			else if(CheckTsPackets(208, buf+i, bufLength-i, packetCount)) {
				(*packetSize) = 208;
				(*startOffset) = i;
				return;
			}					
		}	
		
	}
	
	(*packetCount) = 0;
}

typedef void (*ts_data_callback)(emu_stream_client_data *cdata);

static void ParseTSData(uint8_t table_id, uint8_t table_mask, uint8_t min_table_length, int8_t* flag, uint8_t* data,
							uint16_t data_length, uint16_t* data_pos, int8_t payloadStart, uint8_t* buf, int32_t len,
							ts_data_callback func, emu_stream_client_data *cdata)
{
	uint16_t section_length;
	int32_t i;
	int8_t found_start = 0;
	uint16_t offset = 0;
	int32_t free_data_length;
	int32_t copySize;
	
	if(len < 1)
		{ return; }
	
	if(*flag == 0 && !payloadStart)
		{ return; }

	if(*flag == 0)
	{ 
		*data_pos = 0;
		 offset = 1 + buf[0];
	}
	else if(payloadStart)
	{ 
		offset = 1; 
	}
	
	if(len-offset < 1)
		{ return; }
	
	free_data_length = data_length - *data_pos;
	copySize = (len-offset) > free_data_length ? free_data_length : (len-offset);
	
	memcpy(data+(*data_pos), buf+offset, copySize);	
	(*data_pos) += copySize;

	found_start = 0;
	for(i=0; i < *data_pos; i++)
	{
		if((data[i] & table_mask) == table_id)
		{
			if(i != 0)
			{
				if((*data_pos)-i > i)
					{ memmove(data, &data[i], (*data_pos)-i); }
				else
					{ memcpy(data, &data[i], (*data_pos)-i); }
			
				*data_pos -= i;
			}
			found_start = 1;
			break;
		}	
	}
	if(!found_start)
		{ *flag = 0; return; }

	*flag = 2;

	if(*data_pos < 3)
		{ return; }

	section_length = SCT_LEN(data);

	if(section_length > data_length || section_length < min_table_length)
		{ *flag = 0; return; }
	
	if((*data_pos) < section_length)
		{ return; }

	func(cdata);
	
	found_start = 0;
	for(i=section_length; i < *data_pos; i++)
	{
		if((data[i] & table_mask) == table_id)
		{
			if((*data_pos)-i > i)
				{ memmove(data, &data[i], (*data_pos)-i); }
			else
				{ memcpy(data, &data[i], (*data_pos)-i); }
			
			*data_pos -= i;
			found_start = 1;
			break;
		}	
	}	
	if(!found_start)
		{ *data_pos = 0; }
	
	*flag = 1;
}

static void ParsePATData(emu_stream_client_data *cdata)
{
	uint8_t* data = cdata->data;
	uint16_t section_length = SCT_LEN(data);
	uint16_t srvid;
	int32_t i;

	for(i=8; i+7<section_length; i+=4)
	{
		srvid = b2i(2, data+i);
		
		if(srvid == 0)
			{ continue; }
		
		if(cdata->srvid == srvid)
		{
			cdata->pmt_pid = b2i(2, data+i+2) & 0x1FFF;
			cs_log_dbg(D_READER, "[Emu] stream found pmt pid: %X", cdata->pmt_pid);
			break;
		}
	}
}

static void ParsePMTData(emu_stream_client_data *cdata)
{
	uint8_t* data = cdata->data;
	
	uint16_t section_length = SCT_LEN(data);
	int32_t i;
	uint16_t program_info_length = 0, es_info_length = 0;
	uint8_t descriptor_tag = 0, descriptor_length = 0;
	uint8_t stream_type;
	uint16_t stream_pid, caid;
	
	program_info_length = b2i(2, data+10) &0xFFF;
	
	if(12+program_info_length >= section_length)
		{ return; }
	
	for(i=12; i+1 < 12+program_info_length; i+=descriptor_length)
	{
		descriptor_tag = data[i];
		descriptor_length = data[i+1];
		
		if(descriptor_length < 1)
			{ break; }
			
		if(i+1+descriptor_length >= 12+program_info_length)
			{ break; }
		
		if(descriptor_tag == 0x09 && descriptor_length >= 4)
		{
			caid = b2i(2, data+i+2);
			
			if(caid>>8 == 0x0E)
			{
		    	cdata->ecm_pid = b2i(2, data+i+4) &0x1FFF;
		    	cs_log_dbg(D_READER, "[Emu] stream found ecm_pid: %X", cdata->ecm_pid);
		    	break;
		    }
		}
	}
		
	for(i=12+program_info_length; i+4<section_length; i+=5+es_info_length)
	{
		stream_type = data[i];
		stream_pid = b2i(2, data+i+1) &0x1FFF;
		es_info_length = b2i(2, data+i+3) &0xFFF;
		
		if(stream_type == 0x01 || stream_type == 0x02 || stream_type == 0x10 || stream_type == 0x1B 
			|| stream_type == 0x24 || stream_type == 0x42 || stream_type == 0xD1 || stream_type == 0xEA) 
		{ 
			cdata->video_pid = stream_pid;
			cs_log_dbg(D_READER, "[Emu] stream found video pid: %X", stream_pid);
		}
		
		if(stream_type == 0x03 || stream_type == 0x04 || stream_type == 0x06 || stream_type == 0x0F 
			|| stream_type == 0x11 || (stream_type >= 0x80 && stream_type <= 0x87))
		{
			if(cdata->audio_pid_count >= 4)
				{ continue; }
			
			cdata->audio_pids[cdata->audio_pid_count] = stream_pid;
			cdata->audio_pid_count++;
			cs_log_dbg(D_READER, "[Emu] stream found audio pid: %X", stream_pid);
		}
	}
}

static void ParseECMData(emu_stream_client_data *cdata)
{
	uint8_t* data = cdata->data;
	uint16_t section_length = SCT_LEN(data);
	uint8_t dcw[16];
	
	if(section_length < 0xb)
		{ return; }

	if(data[0xb] > cdata->ecm_nb || (cdata->ecm_nb == 255 && data[0xb] == 0)
		|| ((cdata->ecm_nb - data[0xb]) > 5))
	{
		cdata->ecm_nb = data[0xb];
		PowervuECM(data, dcw, cdata, 0);
	}
}

static void ParseTSPackets(emu_stream_client_data *data, uint8_t *stream_buf, uint16_t packetCount, uint16_t packetSize)
{
	uint32_t i, j;
	uint32_t tsHeader;
	uint16_t pid, offset;
	uint8_t scramblingControl, payloadStart;
	int8_t oddKeyUsed;
	uint32_t *deskey;
	uint8_t *pdata;
	uint8_t *packetCluster[3];
	void *csakey;
	emu_stream_client_data *keydata;
	
	for(i=0; i<packetCount; i++)
	{		
		tsHeader = b2i(4, stream_buf+(i*packetSize));
		pid = (tsHeader & 0x1fff00) >> 8;
		scramblingControl = tsHeader & 0xc0;
		payloadStart = (tsHeader & 0x400000) >> 22;

		if(tsHeader & 0x20)
			{ offset = 4 + stream_buf[(i*packetSize)+4] + 1; }
		else
			{ offset = 4; }
		
		if(packetSize-offset < 1)
			{ continue; }
		
		if(data->have_pat_data != 1)
		{					
			if(pid == 0)
			{ 
				ParseTSData(0x00, 0xFF, 16, &data->have_pat_data, data->data, sizeof(data->data), &data->data_pos, payloadStart, 
								stream_buf+(i*packetSize)+offset, packetSize-offset, ParsePATData, data);
			}
		
			continue;
		}

		if(!data->pmt_pid)
		{
			data->have_pat_data = 0;
			continue;
		}

		if(data->have_pmt_data != 1)
		{
			if(pid == data->pmt_pid)
			{
				ParseTSData(0x02, 0xFF, 21, &data->have_pmt_data, data->data, sizeof(data->data), &data->data_pos, payloadStart, 
								stream_buf+(i*packetSize)+offset, packetSize-offset, ParsePMTData, data);
			}
		
			continue;
		}

		if(data->ecm_pid && pid == data->ecm_pid)
		{
#ifdef WITH_EMU
			stream_server_has_ecm = 1;
#endif
			
			ParseTSData(0x80, 0xFE, 10, &data->have_ecm_data, data->data, sizeof(data->data), &data->data_pos, payloadStart, 
							stream_buf+(i*packetSize)+offset, packetSize-offset, ParseECMData, data);
			continue;
		}
		
		if(scramblingControl == 0)
			{ continue; }
		
		if(!(stream_buf[(i*packetSize)+3] & 0x10))
		{
			stream_buf[(i*packetSize)+3] &= 0x3F;
			continue;
		}

		oddKeyUsed = scramblingControl == 0xC0 ? 1 : 0;

#ifdef WITH_EMU	
		if(!stream_server_has_ecm)
		{
			keydata = &emu_fixed_key_data;
			SAFE_MUTEX_LOCK(&emu_fixed_key_data_mutex); 
		}
		else
		{
#endif
			keydata = data;
#ifdef WITH_EMU
		}
#endif
		
		if(keydata->pvu_csa_used)
		{
			csakey = NULL;
			
			if(pid == data->video_pid)
				{ csakey = keydata->pvu_csa_ks[PVU_CW_VID]; }
			else
			{
				for(j=0; j<data->audio_pid_count; j++)
					if(pid == data->audio_pids[j])
						{ csakey = keydata->pvu_csa_ks[PVU_CW_A1+j]; }
			}
			
			if(csakey != NULL)
			{					
				packetCluster[0] = stream_buf+(i*packetSize);
				packetCluster[1] = stream_buf+((i+1)*packetSize);
				packetCluster[2] = NULL;

				decrypt_packets(csakey, packetCluster);
			}			
		}
		else
		{
			deskey = NULL;
			
			if(pid == data->video_pid)
				{ deskey = keydata->pvu_des_ks[PVU_CW_VID][oddKeyUsed]; }
			else
			{
				for(j=0; j<data->audio_pid_count; j++)
					if(pid == data->audio_pids[j])
						{ deskey = keydata->pvu_des_ks[PVU_CW_A1+j][oddKeyUsed]; }
			}
			
			if(deskey != NULL)
			{					
				for(j=offset; j+7<188; j+=8)
				{
					pdata = stream_buf+(i*packetSize)+j;
					des(pdata, deskey, 0);
				}
				
				stream_buf[(i*packetSize)+3] &= 0x3F;
			}
		}

#ifdef WITH_EMU	
		if(!stream_server_has_ecm)
		{
			SAFE_MUTEX_UNLOCK(&emu_fixed_key_data_mutex); 
		}
#endif
	}
}

static int32_t connect_to_stream(char *http_buf, int32_t http_buf_len, char *stream_path)
{
	struct sockaddr_in cservaddr;
		
	int32_t streamfd = socket(AF_INET, SOCK_STREAM, 0);
	if(streamfd == -1)
		{ return -1; }
	
	bzero(&cservaddr, sizeof(cservaddr));
	cservaddr.sin_family = AF_INET;
	cservaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	cservaddr.sin_port = htons(emu_stream_source_port);
	
	if(connect(streamfd, (struct sockaddr *)&cservaddr, sizeof(cservaddr)) == -1)
		{ return -1; }
			
	snprintf(http_buf, http_buf_len, "GET %s HTTP/1.1\nHost: localhost:%u\n"
				"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:38.0) Gecko/20100101 Firefox/38.0\n"
				"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\n"
				"Accept-Language: en-US\n"
				"Connection: keep-alive\n\n", stream_path, emu_stream_source_port);

	if(send(streamfd, http_buf, strlen(http_buf), 0) == -1)
		{ return -1; }
		
	return streamfd;	
}

static void handle_stream_client(int32_t connfd)
{
#define EMU_DVB_MAX_TS_PACKETS 32
#define EMU_DVB_BUFFER_SIZE 1+188*EMU_DVB_MAX_TS_PACKETS
#define EMU_DVB_BUFFER_WAIT 1+188*(EMU_DVB_MAX_TS_PACKETS-3)

	char *http_buf, stream_path[255], stream_path_copy[255];
	int32_t streamfd;
	int32_t clientStatus, streamStatus;
	uint8_t *stream_buf;
	uint16_t packetCount = 0, packetSize = 0, startOffset = 0;
	uint32_t remainingDataPos, remainingDataLength;
	int32_t bytesRead = 0;
	emu_stream_client_data *data;
	int8_t streamErrorCount = 0;
	int32_t i, srvidtmp;
	char *saveptr, *token;
	
	if(!cs_malloc(&http_buf, 1024))
	{
		close(connfd);
		return;
	}
	
	if(!cs_malloc(&stream_buf, EMU_DVB_BUFFER_SIZE))
	{
		close(connfd);
		NULLFREE(http_buf);
		return;
	}
	
	if(!cs_malloc(&data, sizeof(emu_stream_client_data)))
	{
		close(connfd);
		NULLFREE(http_buf);
		NULLFREE(stream_buf);
		return;
	}
	
	clientStatus = recv(connfd, http_buf, 1024, 0);
	if(clientStatus < 1)
	{
		close(connfd);
		NULLFREE(http_buf);
		NULLFREE(stream_buf);
		NULLFREE(data);
		return;		
	}
	
	http_buf[1023] = '\0';
	if(sscanf(http_buf, "GET %254s ", stream_path) < 1)
	{
		close(connfd);
		NULLFREE(http_buf);
		NULLFREE(stream_buf);
		NULLFREE(data);
		return;
	}
	
	cs_strncpy(stream_path_copy, stream_path, sizeof(stream_path));
	
	token = strtok_r(stream_path_copy, ":", &saveptr);

	for(i=0; token != NULL && i<3; i++)
	{
		token = strtok_r(NULL, ":", &saveptr);
		if(token == NULL)
			{ break; }
	}
	if(token != NULL)
	{
		if(sscanf(token, "%x", &srvidtmp) < 1)
		{
			token = NULL;	
		}
		else
		{
			data->srvid = srvidtmp & 0xFFFF;
		}
	}

	if(token == NULL)
	{
		close(connfd);
		NULLFREE(http_buf);
		NULLFREE(stream_buf);
		NULLFREE(data);
		return;
	}

#ifdef WITH_EMU
	emu_stream_cur_srvid = data->srvid;
	stream_server_has_ecm = 0;
#endif

	snprintf(http_buf, 1024, "HTTP/1.0 200 OK\nConnection: Close\nContent-Type: video/mpeg\nServer: stream_enigma2\n\n");
	clientStatus = send(connfd, http_buf, strlen(http_buf), 0);

	while(!exit_oscam && clientStatus != -1 && streamErrorCount < 3)
	{		
		streamfd = connect_to_stream(http_buf, 1024, stream_path);
		if(streamfd == -1)
		{
			cs_log("[Emu] warning: cannot connect to stream source");
			streamErrorCount++;
			cs_sleepms(100);
			continue;	
		}

		streamErrorCount = 0;
		streamStatus = 0;

		while(!exit_oscam && clientStatus != -1 && streamStatus != -1)
		{
			streamStatus = recv(streamfd, stream_buf+bytesRead, EMU_DVB_BUFFER_SIZE-bytesRead, 0);
			if(streamStatus == -1)
				{ break; }
		
			bytesRead += streamStatus;
			
			if(bytesRead >= EMU_DVB_BUFFER_WAIT)
			{	
				SearchTsPackets(stream_buf, bytesRead, &packetCount, &packetSize, &startOffset);
				
				if(packetCount <= 0)
				{
					bytesRead = 0;
				}
				else
				{
					ParseTSPackets(data, stream_buf+startOffset, packetCount, packetSize);
					
					clientStatus = send(connfd, stream_buf+startOffset, packetCount*packetSize, 0);
						 
					remainingDataPos = startOffset+(packetCount*packetSize);
					remainingDataLength = bytesRead-remainingDataPos;
					
					if(remainingDataPos < remainingDataLength)
						{ memmove(stream_buf, stream_buf+remainingDataPos, remainingDataLength); }
					else
						{ memcpy(stream_buf, stream_buf+remainingDataPos, remainingDataLength); }
					
					bytesRead = remainingDataLength;
				}
			}
		}
		
		close(streamfd);
	}
	
	close(connfd);
	NULLFREE(http_buf);
	NULLFREE(stream_buf);
	for(i=0; i<8; i++)
	{
		if(data->pvu_csa_ks[i])
			{ free_key_struct(data->pvu_csa_ks[i]); }	
	}
	NULLFREE(data);
}

void *stream_server(void *UNUSED(a))
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t clilen;
	int32_t reuse = 1;
	
	glistenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(glistenfd == -1)
	{
		cs_log("[Emu] error: cannot create stream server socket");
		return NULL;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(emu_stream_relay_port);
	setsockopt(glistenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	
	if(bind(glistenfd,(struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		cs_log("[Emu] error: cannot bind to stream server socket");
		close(glistenfd);
		return NULL;
	}
	
	if(listen(glistenfd, 3) == -1)
	{
		cs_log("[Emu] error: cannot listen to stream server socket");
		close(glistenfd);
		return NULL;
	}

	while(!exit_oscam)
  	{
		clilen = sizeof(cliaddr);
		gconnfd = accept(glistenfd,(struct sockaddr *)&cliaddr, &clilen);

		if(gconnfd == -1)
		{
			cs_log("[Emu] error: accept() failed");
			break;
		}
		
		handle_stream_client(gconnfd);
	} 
	
	close(glistenfd);
	
	return NULL;
}

#ifdef WITH_EMU
void *stream_key_delayer(void *UNUSED(a))
{
	int32_t j;
	emu_stream_client_data* cdata = &emu_fixed_key_data;
	emu_stream_cw_item *item;
	
	while(!exit_oscam)
	{
		item = ll_remove_first(ll_emu_stream_delayed_keys);
		
		if(item)
		{
			cs_sleepms(cfg.emu_stream_ecm_delay);
	    	
			SAFE_MUTEX_LOCK(&emu_fixed_key_data_mutex);
	    	
			for(j=0; j<8; j++)
			{
				if(item->csa_used)
				{	
					if(cdata->pvu_csa_ks[j] == NULL)
						{  cdata->pvu_csa_ks[j] = get_key_struct(); }
						
					if(item->is_even)
						{ set_even_control_word(cdata->pvu_csa_ks[j], item->cw[j]); }
					else
						{ set_odd_control_word(cdata->pvu_csa_ks[j], item->cw[j]); }
					
					cdata->pvu_csa_used = 1;
				}
				else
				{					
					if(item->is_even)
						{ des_set_key(item->cw[j], cdata->pvu_des_ks[j][0]); }
					else
						{ des_set_key(item->cw[j], cdata->pvu_des_ks[j][1]); }
						
					cdata->pvu_csa_used = 0;
				}
			}
							
			SAFE_MUTEX_UNLOCK(&emu_fixed_key_data_mutex);
			
			free(item);
		}
		else
		{
			cs_sleepms(50);	
		}
	}
	
	return NULL;
}
#endif

void stop_stream_server(void)
{
	shutdown(gconnfd, 2);
	shutdown(glistenfd, 2);
	close(gconnfd);
	close(glistenfd);	
}
