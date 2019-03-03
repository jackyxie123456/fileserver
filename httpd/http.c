#include "httpd.h"
#include <errno.h>
#include "utils.h"
#include "network.h"
#include "mergepipe.h"

#define MAX_URI_LEN 8192

typedef struct  
{
    char                *key;
    char                *value;
} request_fields_t;

typedef struct
{
    char                *method;
    char                *uri;
    char                *version;
    uint8_t             fields_count;
    request_fields_t    *fields;
} request_header_t;

static void accept_callback(event_t *ev);
static void read_callback(event_t *ev);
static void write_callback(event_t *ev);

static int   read_request_header(event_t *ev, char **buf, int *size);
static int   read_request_boundary(event_t *ev);
static int   parse_request_header(char *data, request_header_t *header);
static void  release_request_header(request_header_t *header);
static void  release_event(event_t *ev);
static event_data_t *create_event_data(const char *header, const char *html);
static event_data_t *create_event_data_fp(const char *header, FILE *fp, int read_len, int total_len);
static void  release_event_data(event_t *ev);
/*
	0  SUCC
	!= 0  FAIL 
*/
static int  uri_decode(char* uri ,char* outUri , int nLen);
static uint8_t ishex(uint8_t x);

static int   reset_filename_from_formdata(event_t *ev, char **formdata, int size);
static int   parse_boundary(event_t *ev, char *data, int size, char **ptr);

static const char *reponse_content_type(char *file_name);
static const char *response_header_format();
static const char *response_body_format();
static void response_home_page(event_t *ev, char *path);
static void response_upload_page(event_t *ev, int result);
static void response_send_file_page(event_t *ev, char *file_name);
static void response_http_400_page(event_t *ev);
static void response_http_404_page(event_t *ev);
static void response_http_500_page(event_t *ev);
static void response_http_501_page(event_t *ev);
static void send_response(event_t *ev, char* title, char *status);

static void new_merge_task(const char* fileName, int index , int nTotal);

SOCKET sSockFd = 0;//

void http_stop() {
	if (sSockFd) {
#ifdef WIN32
		shutdown(sSockFd, SD_BOTH);
#else 
		shutdown(sSockFd, SHUT_RDWR);
#endif
		close_socket(sSockFd);
		event_uninit();
		network_unint();
	}
}
int http_startup(int *port)
{
    event_t ev = {0};

	sSockFd = 0;//
    log_info("{%s:%d} Http server start...", __FUNCTION__, __LINE__);
    network_init();
    event_init();
    network_listen(port, &sSockFd);

    ev.fd = sSockFd;
    ev.ip = htonl(INADDR_ANY);
    ev.type = EV_READ | EV_PERSIST;
    ev.callback = accept_callback;
	ev.sessiondata = NULL;
	ev.data = NULL;
    event_add(&ev);

    event_dispatch();

   // closesocket(fd);
	close_socket(sSockFd);
    event_uninit();
    network_unint();
    log_info("{%s:%d} Http server stop ...", __FUNCTION__, __LINE__);
    return SUCC;
}

static void accept_callback(event_t *ev)
{
    SOCKET fd = 0 ;
    struct in_addr addr;
    event_t ev_ = {0};

    if (SUCC == network_accept(ev->fd, &addr, &fd))
    {   
        ev_.fd = fd;
        ev_.ip = addr.s_addr;
        ev_.type = EV_READ | EV_PERSIST;
        ev_.callback = read_callback;
		ev_.sessiondata = NULL;
		ev_.data = NULL;
        event_add(&ev_);

        log_info("{%s:%d} A new client connect. ip = %s, socket=%d", __FUNCTION__, __LINE__, inet_ntoa(addr), fd);
    }
    else
    {
        log_error("{%s:%d} accept fail. GetLastError=%d", __FUNCTION__, __LINE__, WSAGetLastError());
	}
}



static int split(char *str, char *delimiter , char** pValue) {
	int len = strlen(str);
	int i = 0;
	//将strCopy中的每个分隔符赋值为'\0'
	for (i = 0; str[i] != '\0'; i++) {
		int j = 0;
		for ( j = 0; delimiter[j] != '\0'; j++) {
			if (str[i] == delimiter[j]) {
				str[i] = '\0';
				break;
			}
		}
	}
	//char** res = (char**)malloc((len + 2) * sizeof(char*));
	len++; //遍历到strCopy最后的'\0'才结束
	int resI = 0; //每一个分隔符和原字符串的'\0'标志依次作为数组中的字符串的结束标志
	for ( i = 0; i < len; i++) {
		pValue[resI++] = str + i;
		while (str[i] != '\0') {
			i++;
		}
	}
	pValue[resI] = NULL; //字符串数组中结束标志

	return resI;
}


#define MAX_COUNT 1024 //

static int  parse_url(const char* url , char** key ,char** value ) {
	
	char* szKeyValue[MAX_COUNT] = {0};
	int nCount = 0;
	int nKeyCnt = 0;
	nCount = split(url, "&", szKeyValue);

	int i = 0;
	for (i = 0; i < nCount; i++) {
		char* szKeyV[3] = { 0 };
		int nKeyValue = split(szKeyValue[i], "=", szKeyV);
		key[i] = szKeyV[0];
		value[i] = szKeyV[1];
		nKeyCnt++;
	}
	return nKeyCnt;
}

static void read_callback(event_t *ev)
{
    char *buf = NULL;
    int   size = 0;
    request_header_t header;
    int   i = 0;
    int   content_length = 0;
    char *temp = NULL;
    char  file_path[MAX_PATH] = { 0 };
	char  szUrl[MAX_URI_LEN] = { 0 };
	int iResult = 0;//

	if (NULL == ev)
		return ;

	memset(&header, 0x00, sizeof(header));
	log_info("{%s:%d} read_callback  status : %d %d",
		__FUNCTION__, __LINE__, ev->status  , ev->fd);

    if (ev->status == EV_IDLE)
    {
		iResult = read_request_header(ev, &buf, &size);
        if (SUCC != iResult)
        {
            response_http_400_page(ev);
            free(buf);
			log_error("{%s:%d} read_request_header err %d...", 
							__FUNCTION__, __LINE__ , iResult);
            return ;
        }
		if (!buf) {
			log_error("{%s:%d} read_request_header buf is null ...",
				__FUNCTION__, __LINE__);
			return ;
		}
            
        parse_request_header(buf, &header);
        int nUrlRet = uri_decode(header.uri , szUrl , MAX_URI_LEN);
		if (0 != nUrlRet) {
			log_error("{%s:%d} >>> parse url error %d",
				__FUNCTION__, __LINE__ , nUrlRet);
			response_http_501_page(ev);
			release_request_header(&header);
			free(buf);
			return ;
		}
		
        header.uri = utf8_to_ansi(szUrl);

        log_info("{%s:%s:%d} >>> Entry recv ... uri=%s", 
			__FILE__,__FUNCTION__, __LINE__, header.uri);
		if (NULL == header.uri) {
			
			response_http_501_page(ev);
			release_request_header(&header);
			free(buf);
			return;
		}

        if (strcmp(header.method, "GET") && strcmp(header.method, "POST"))
        {
            // 501 Not Implemented
			log_warn("{%s:%s:%d} >>> read_callback method =%s",
				__FILE__,__FUNCTION__, __LINE__, header.method);
            response_http_501_page(ev);
            release_request_header(&header);
            free(buf);
			release_event(ev);
            return;
        }
        if (0 == strcmp(header.method, "POST"))
        {
            /***** This program is not supported now.                             *****/
            /***** Just using Content-Type = multipart/form-data when upload file *****/

            // 1. get Content-Length
            // 2. if don't have Content-Length. using chunk
            // 3. read_request_body()
            // 4. get Content-Type
            // 5. Content-Type is json or others
        }

        if (0 == strncmp(header.uri, "/upload", strlen("/upload")) 
				&& 0 == strcmp(header.method, "POST"))
        {
            // get upload file save path from uri
            memset(file_path, 0, sizeof(file_path));
            memcpy(file_path, root_path(), strlen(root_path()));

			char* pKey[256] = { 0 };
			char* pValue[256] = { 0 };
			char szUrl[4096] = { 0 };
			int nLen = strlen(header.uri + strlen("/upload?"));
			strcpy(szUrl, header.uri + strlen("/upload?"));
			int nKeys = parse_url(szUrl, pKey, pValue);
			if (nKeys > 0) {
				ev->sessiondata = malloc(sizeof(event_session_data));
				if (NULL != ev->sessiondata) {
					memset(ev->sessiondata, 0x00, sizeof(event_session_data));
					session_data_init(ev->sessiondata, nKeys, pKey, pValue);
				}
			}

			char* sPathVal = session_data_getKey(ev->sessiondata, "path");
            if(NULL != sPathVal){
                memcpy(file_path + strlen(file_path), 
					sPathVal,
					strlen(sPathVal));
            }
            if (0 == strcmp(header.method, "POST"))
            {
                // get Content-Length boundary_length
                for (i = 0; i < header.fields_count; i++)
                {
                    if (0 == strcmp(header.fields[i].key, "Content-Length"))
                    {
                        content_length = atoi(header.fields[i].value);
                        break;
                    }
                }
                if (content_length == 0)
                {
                    // filelength == 0 
					log_warn("{%s:%d} >>> read_callback file len is 0",
						__FUNCTION__, __LINE__);
                    response_http_501_page(ev);
                    release_request_header(&header);
                    free(buf);
                    return;
                }
				
				release_event_data(ev);
                // get boundary
                ev->data = (event_data_t*)malloc(sizeof(event_data_t) - sizeof(char) + BUFFER_PAGE);
                if (!ev->data)
                {
                    // 500 Internal Server Error
					log_warn("{%s:%d} >>> read_callback can not malloc",
						__FUNCTION__, __LINE__);
                    response_http_500_page(ev);
                    release_request_header(&header);
                    free(buf);
                    return;
                }
				memset(ev->data, 0, sizeof(event_data_t) - sizeof(char) + BUFFER_PAGE);
                for (i = 0; i < header.fields_count; i++)
                {
                    if (0 == strcmp(header.fields[i].key, "Content-Type"))
                    {
                        temp = strstr(header.fields[i].value, "boundary=");
                        if (temp)
                        {
                            temp += strlen("boundary=");
							int nLen = strlen(temp);
							if (nLen > BOUNDARY_MAX_LEN - 1)
								nLen = BOUNDARY_MAX_LEN - 1;
                            memcpy(ev->data->boundary, temp, nLen);
                        }
                        break;
                    }
                }
                if (ev->data->boundary[0] == 0)
                {
                    // not support
                    // not found boundary
					log_warn("{%s:%d} >>> read_callback boundary not found",
						__FUNCTION__, __LINE__);
                    response_http_501_page(ev);
                    release_request_header(&header);
                    free(buf);
                    return;
                }

                // release memory
                release_request_header(&header);
                free(buf);

                // set event
				// no check file_path length , dangerous
				memset(ev->data->file, 0x00, sizeof(ev->data->file));
                memcpy(ev->data->file, file_path, strlen(file_path));
                ev->data->offset = 0;
                ev->data->total = content_length;

				log_info("{%s:%s:%d} >>> read_request_boundary %s %d",
					__FILE__,
					__FUNCTION__, 
					__LINE__,
					ev->data ? ev->data->file : "", 
					content_length );
                // read & save files
                int retReqBody = read_request_boundary(ev);
				if (retReqBody) {
					log_error("{%s:%d} >>> read_request_boundary ret %d",
						__FUNCTION__, __LINE__,retReqBody);
				}
            }
            else
            {
                // not post  ,not support
                // 501 Not Implemented
				log_info("{%s:%d} >>> read_callback boundary not found",
					__FUNCTION__, __LINE__);
                response_http_501_page(ev);
                free(buf);
                release_request_header(&header);
                return;
            }
        }
        else if ( header.uri && strlen(header.uri) > 0 &&
					header.uri[strlen(header.uri) - 1] == '/')
        {
			log_info("{%s:%d} >>> read_callback request path %s ",
				__FUNCTION__, __LINE__, header.uri + 1);
            response_home_page(ev, header.uri + 1);
            free(buf);
            release_request_header(&header);
            return ;
        }
        else
        {
            // send file
            memset(file_path, 0, sizeof(file_path));
            memcpy(file_path, root_path(), strlen(root_path()));
            
			memcpy(file_path + strlen(file_path), 
				   header.uri + 1,
				   strlen(header.uri + 1));

			log_info("{%s:%d} >>> send file %s ",
				__FUNCTION__, __LINE__, file_path);

            response_send_file_page(ev, file_path);
            free(buf);
            release_request_header(&header);
            return ;
        }
    }
    else
    {
		log_info("{%s:%d} read_callback  status != EV_IDLE : %d ",
			__FUNCTION__, __LINE__, ev->status);
        // read & save files
        read_request_boundary(ev);
    }
}

static void write_callback(event_t *ev)
{
    if (!ev->data)
        return;

	log_info("{%s:%d} write_callback . progress=%d%%, socket=%d", __FUNCTION__, __LINE__, ev->data->total ? ev->data->offset * 100 / ev->data->total : 100, ev->fd);

    if (ev->data->size != 
		send(ev->fd, ev->data->data, ev->data->size, 0))
    {
        log_error("{%s:%d} send fail. socket=%d, WSAGetLastError=%d", __FUNCTION__, __LINE__, ev->fd, WSAGetLastError());
#ifdef WIN32
		shutdown(ev->fd, SD_SEND);
#else 
		shutdown(ev->fd, SHUT_WR);
#endif
		release_event_data(ev);
        return;
    }

    if (ev->data->total == ev->data->offset)
    {
        log_info("{%s:%d} send response completed. socket=%d", __FUNCTION__, __LINE__, ev->fd);
        release_event_data(ev);
    }
    else
    {
        response_send_file_page(ev, ev->data->file);
    }
}

static int read_request_header(event_t *ev, char **buf, int *size)
{
    char c;
    int len = 1;
    int idx = 0;
    int ret = 0;

    while (TRUE)
    {
        ret = network_read(ev->fd, &c, len);
        if (ret == DISC)
        {
            release_event(ev);
            return SUCC;
        }
        else if (ret == SUCC)
        {
            if (*buf == NULL)
            {
                *size = BUFFER_UNIT;
                *buf = (char*)malloc(*size);
                if (!(*buf))
                {
                    log_error("{%s:%d} malloc fail.", __FUNCTION__, __LINE__);
                    return FAIL;
                }
            }
            (*buf)[idx++] = c;
            if (idx >= *size - 1) // last char using for '\0'
            {
                // buffer is not enough
                *size += BUFFER_UNIT;
                *buf = (char*)realloc(*buf, *size);
                if (!(*buf))
                {
                    log_error("{%s:%d} realloc fail.", __FUNCTION__, __LINE__);
                    return FAIL;
                }
            }
            if (idx >= 4 && (*buf)[idx - 1] == LF && (*buf)[idx - 2] == CR
                && (*buf)[idx - 3] == LF && (*buf)[idx - 4] == CR)
            {
                (*buf)[idx] = 0;
                return SUCC;
            }
        }
        else
        {
            log_error("{%s:%d} recv unknown fail.", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }

    log_error("{%s:%d} cannot found header.", __FUNCTION__, __LINE__);
    return FAIL;
}



static int check_crc_code(unsigned int crccode_old , const char* fileName ,int nFileLen) {

	int nLen = 0;
	unsigned int crccode = calcCrcCode(fileName , &nLen);
	

	log_info("{%s:%s:%d} crc code check %s %u , %u  , crcLen %d , fileLen %d ",
		__FILE__,
		__FUNCTION__,
		__LINE__,
		fileName,
		crccode,
		crccode_old, 
		nLen,
		nFileLen);

	if (nFileLen != nLen) {
		log_error("{%s:%s:%d} File Len check error %s,crcLen %d , fileLen %d .",
			__FILE__,
			__FUNCTION__,
			__LINE__,
			fileName,
			nLen,
			nFileLen);
	}

	if (crccode != crccode_old) {
		
		log_error("{%s:%s:%d} crc code check error %u , %u ,crcLen %d , fileLen %d .", 
			__FILE__,
			__FUNCTION__, 
			__LINE__,
			crccode , 
			crccode_old,
			nLen,
			nFileLen);

		return 1;
	}

	return 0;
}


#define META_POST ".meta"

static int save_meta_data(const char* fileName, int nIndex, int nTotal, int nSize, unsigned int nCrccode) {
	char szPath[MAX_PATH] = { 0 };
	FILE* pFile = NULL;

	char* pTotal = strstr((char*)fileName, "_t_");
	if (NULL == pTotal)
	{
		memcpy(szPath, fileName, strlen(fileName));
	}
	else {
		memcpy(szPath, fileName, pTotal - fileName);
	}

	int nLenCat = strlen(META_POST);
	nLenCat = nLenCat > (MAX_PATH - strlen(szPath)) ? (MAX_PATH - strlen(szPath)) : nLenCat;
	strncat(szPath, META_POST, nLenCat);
	pFile = fopen(szPath, "rb+");
	if (NULL == pFile)
		return -1;

	fseek(pFile, 0, SEEK_END);

	char szBuff[1024] = { 0 };

	sprintf(szBuff, "%d,%s,%d,%d,%u\r\n",
		nIndex, fileName, nTotal, nSize, nCrccode);
	fputs(szBuff, pFile);
	fclose(pFile);

	return 0;
}

/*
	0  ok
	-1  write failed 
	-2  not found fileName
	-3  recv data failed
*/

static int read_request_boundary(event_t *ev)
{
#define WRITE_FILE(fp, buf, size, ev) do { \
    if (size) { \
        if (size != fwrite(buf, 1, size, fp)) { \
			log_error("{%s:%d} write file fail. socket=%d", __FUNCTION__, __LINE__, ev->fd); \
            ev->status = EV_IDLE; \
            response_upload_page(ev, 0); \
			release_event(ev); \
            return -1; \
        } \
    } \
} while (0)

#define GET_FILENAME(ev, ptr, size_) do { \
    ret = reset_filename_from_formdata(ev, &ptr, (size_)); \
    if (ret == 0) { \
        log_error("{%s:%d} cannot found filename in formdata. socket=%d", __FUNCTION__, __LINE__, ev->fd); \
        ev->status = EV_IDLE; \
        response_upload_page(ev, 0); \
        return -2; \
    } else if (ret == 1) { \
        ev->data->fp = fopen(ev->data->file, "wb"); \
        if (!ev->data->fp) { \
            log_error("{%s:%d} open file fail. filename=%s, socket=%d, errno=%d", __FUNCTION__, __LINE__, ev->data->file, ev->fd, errno); \
            ev->status = EV_IDLE; \
            response_upload_page(ev, 0); \
			release_event(ev); \
            return -3; \
        } \
        compare_buff_size -= ptr - compare_buff; \
        memcpy(compare_buff, ptr, compare_buff_size); \
        compare_buff[compare_buff_size] = 0; \
        goto _re_find; \
    } else { \
        ev->data->size = compare_buff + compare_buff_size - ptr; \
        memcpy(ev->data->data, ptr, ev->data->size); \
    } \
} while (0)

    char    *buf    = NULL;
    char    *ptr    = NULL;
    int      ret    = 0;
    uint32_t offset = 0;
    uint32_t writen = 0;
    uint32_t compare_buff_size = 0;
    char     buffer[BUFFER_PAGE + 1] = {0};
    char     compare_buff[BUFFER_PAGE * 2 + 1] = {0};

	if (NULL == ev || NULL == ev->data) {
		response_http_500_page(ev);
		release_event(ev);

		log_error("{%s:%s:%d} , read_request_boundary ev or data is NULL %x ", 
			__FILE__,__FUNCTION__, __LINE__, ev); 
		return -2;
	}

	offset = ev->data->total - ev->data->offset;
    offset = offset > BUFFER_PAGE ? BUFFER_PAGE : offset;

	log_info("{%s:%s:%d} read_request_boundary offset %d size:%d",
		__FILE__, __FUNCTION__, __LINE__, offset, ev->data->size);

    ret = network_read(ev->fd, buffer, offset);
    
    if (ret == DISC)
    {
        response_http_500_page(ev);
        release_event(ev);
        return -3;
    }
    else if (ret == SUCC)
    {
        memset(compare_buff, 0, sizeof(compare_buff));
		assert(ev->data->size < BUFFER_PAGE);
        if (ev->data->size)
        {
            memcpy(compare_buff, ev->data->data, ev->data->size);
        }
        memcpy(compare_buff + ev->data->size, buffer, offset);
        compare_buff_size = offset + ev->data->size;
        ev->data->size = 0;
        memset(ev->data->data, 0, BUFFER_PAGE);

        // parse boundary
	_re_find:
		log_info("{%s:%d} before parse_boundary ... , compare_buff_size : %d",
			__FUNCTION__, __LINE__,
			compare_buff_size);

        ret = parse_boundary(ev, compare_buff, compare_buff_size, &ptr);
        switch (ret)
        {
        case 0: // write all bytes to file
            WRITE_FILE(ev->data->fp, compare_buff, compare_buff_size, ev);
			log_info("{%s:%d} parse_boundary ret 0 upload [%s] progress=%d%%. socket=%d", __FUNCTION__, __LINE__, ev->data->file, ev->data->offset * 100 / ev->data->total, ev->fd);
            break;
        case 1: // first boundary
            // get file name from boundary header
			log_info("{%s:%d} parse_boundary ret 1  socket=%d", __FUNCTION__, __LINE__,  ev->fd);
            GET_FILENAME(ev, ptr, compare_buff + compare_buff_size - ptr);
            break;
        case 2: // last boundary
			{

				ASSERT(ev->data->total == ev->data->offset + offset);
				writen = ptr - compare_buff;
				log_info("{%s:%d} parse_boundary ret 2 upload [%s] complete. socket=%d", __FUNCTION__, __LINE__, ev->data->file, ev->fd);
				// writen bytes before boundary
				WRITE_FILE(ev->data->fp, compare_buff, writen, ev);
			
				char fileName[MAX_PATH] = {0};
				int  nTotal = ev->data->nTotal;
				int nIndex = 0;//
				char* sIndex = session_data_getKey(ev->sessiondata, "index");
				if (NULL != sIndex) {
					nIndex = atoi(sIndex);
				}
				strncpy(fileName, ev->data->file , MAX_PATH);
				unsigned int crccode_old = -1;//
				char* pCrc = session_data_getKey(ev->sessiondata, "crccode");
				if (NULL != pCrc) {
					crccode_old = (unsigned int)atoi(pCrc);
				}

				int nFileLen1 = 0;//
				char* pFileLen1 = session_data_getKey(ev->sessiondata, "size");
				if (NULL != pFileLen1) {
					nFileLen1 = atoi(pFileLen1);
				}

				release_event_data(ev);

				new_merge_task(fileName, nIndex , nTotal);
				int nCrcError = check_crc_code(crccode_old, fileName , nFileLen1);
			
				ev->status = EV_IDLE;
				if (nCrcError) {
					response_upload_page(ev, 0);
				}
				else {
					response_upload_page(ev, 1);
				}
				return 0;

			}
        case 3: // middle boundary
			{
				// writen bytes before boundary
				writen = ptr - compare_buff;
				WRITE_FILE(ev->data->fp, compare_buff, writen, ev);

				char fileName[MAX_PATH] = { 0 };
				int  nTotal = ev->data->nTotal;
				int nIndex = 0;//
				char* sIndex = session_data_getKey(ev->sessiondata, "index");
				if (NULL != sIndex) {
					nIndex = atoi(sIndex);
				}
				strncpy(fileName, ev->data->file, MAX_PATH);

				new_merge_task(fileName , nIndex , nTotal);

				if (ev->data) {
					fclose(ev->data->fp);
					ev->data->fp = NULL;
				}
				log_info("{%s:%d} upload [%s] complete. socket=%d", __FUNCTION__, __LINE__, ev->data->file, ev->fd);
				// get file name from boundary header
				GET_FILENAME(ev, ptr, compare_buff + compare_buff_size - ptr);
			}
			break;
        case 4: // backup last boundary
        case 5: // backup middle boundary
            writen = ptr - compare_buff;
            WRITE_FILE(ev->data->fp, compare_buff, writen, ev);
			log_info("{%s:%d} parse_boundary ret 5 writen:%d", 
				__FUNCTION__, __LINE__,writen);
			// backup
			ASSERT(ev->data->size < BUFFER_PAGE);
            ev->data->size = compare_buff + compare_buff_size - ptr;
            memcpy(ev->data->data, ptr, ev->data->size);
            break;
        default:
            break;
        }

        ev->data->offset += offset;
        ev->status = EV_BUSY;

		log_info("{%s:%s:%d} recv ...,fd : %d , offset : %d",
			__FILE__,__FUNCTION__, __LINE__,
			ev->fd ,
			ev->data->offset
		);
    }	
    else
    {
        log_error("{%s:%d} recv unknown fail.", __FUNCTION__, __LINE__);
		release_event_data(ev);
        ev->status = EV_IDLE;
        response_upload_page(ev, 0);
		
		return -4;
    }
}




static int parse_request_header(char *data, request_header_t *header)
{
#define move_next_line(x)   while (*x && *(x + 1) && *x != CR && *(x + 1) != LF) x++;
#define next_header_line(x) while (*x && *(x + 1) && *x != CR && *(x + 1) != LF) x++; *x=0;
#define next_header_word(x) while (*x && *x != ' ' && *x != ':' && *(x + 1) && *x != CR && *(x + 1) != LF) x++; *x=0;

    char *p = data;
    char *q;
    int idx = 0;

    memset(header, 0, sizeof(request_header_t));
    // method
    next_header_word(p);
    header->method = data;
    // uri
    data = ++p;
    next_header_word(p);
    header->uri = data;
    // version
    data = ++p;
    next_header_word(p);
    header->version = data;
    // goto fields data
    next_header_line(p);
    data = ++p + 1;
    p++;

	header->fields_count = 0;
    // fields_count
    q = p;
    while (*p)
    {
        move_next_line(p);
        data = ++p + 1;
        p++;
        header->fields_count++;
        if (*data && *(data + 1) && *data == CR && *(data + 1) == LF)
            break;
    }
    // malloc fields
    header->fields = (request_fields_t*)malloc(sizeof(request_fields_t)*header->fields_count);
    if (!header->fields)
    {
        log_error("{%s:%d} malloc fail.", __FUNCTION__, __LINE__);
        return FAIL;
    }
    // set fields
    data = p = q;
    while (*p)
    {
        next_header_word(p);
        header->fields[idx].key = data;
        data = ++p;
        if (*data == ' ')
        {
            data++;
            p++;
        }
        next_header_line(p);
        header->fields[idx++].value = data;
        data = ++p + 1;
        p++;
        if (*data && *(data + 1) && *data == CR && *(data + 1) == LF)
            break;
    }
    ASSERT(idx == header->fields_count);

	log_info("{%s:%d} parse_request_header ...%d",
				__FUNCTION__, __LINE__, header->fields_count);
    return SUCC;
}

static void release_request_header(request_header_t *header)
{
    if (header->uri)
    {
        free(header->uri);
		header->uri = NULL;
    }
    if (header->fields)
    {
        free(header->fields);
		header->fields = NULL;
    }
}

static void release_event(event_t *ev)
{
   // closesocket(ev->fd);
	close_socket(ev->fd);
    release_event_data(ev);
    event_del(ev);
	ev->fd = NULL;//必须放在后面
	log_error("{%s:%s:%d} release_event ...",__FILE__, __FUNCTION__, __LINE__);
}

static event_data_t *create_event_data(const char *header, const char *html)
{
    event_data_t* ev_data = NULL;
    int header_length = 0;
    int html_length = 0;
    int data_length = 0;

    if (header)
        header_length = strlen(header);
    if (html)
        html_length = strlen(html);

    data_length = sizeof(event_data_t) - sizeof(char) + header_length + html_length;
	log_error("{%s:%d} create_event_data malloc %d", __FUNCTION__, __LINE__,data_length);
	ev_data = (event_data_t*)malloc(data_length);
    if (!ev_data)
    {
        log_error("{%s:%s:%d} malloc failed", __FILE__,__FUNCTION__, __LINE__);
        return ev_data;
    }
    memset(ev_data, 0, data_length);
    ev_data->total = header_length + html_length;
    ev_data->offset = header_length + html_length;
    ev_data->size = header_length + html_length;
    if (header)
        memcpy(ev_data->data, header, header_length);
    if (html)
        memcpy(ev_data->data + header_length, html, html_length);
    return ev_data;
}

static event_data_t *create_event_data_fp(const char *header, FILE *fp, int read_len, int total_len)
{
    event_data_t* ev_data = NULL;
    int header_length = 0;
    int data_length = 0;

    if (header)
        header_length = strlen(header) + 1;

    data_length = sizeof(event_data_t) - sizeof(char) + header_length + read_len;
    ev_data = (event_data_t*)malloc(data_length);
    if (!ev_data)
    {
        log_error("{%s:%d} malloc failed", __FUNCTION__, __LINE__);
        return ev_data;
    }
    memset(ev_data, 0, data_length);
    ev_data->total = total_len;
    ev_data->size = read_len + header_length;
    if (header)
        memcpy(ev_data->data, header, header_length);

    if (fp && read_len != fread(ev_data->data + header_length, 1, read_len, fp))
    {
        log_error("{%s:%d} fread failed", __FUNCTION__, __LINE__);
        free(ev_data);
        ev_data = NULL;
    }
    return ev_data;
}

static void release_event_data(event_t *ev)
{
    if (ev->data)
    {
        if (ev->data->fp)
        {
            fclose(ev->data->fp);
            ev->data->fp = NULL;
        }
        free(ev->data);
        ev->data = NULL;
    }
}

/*
	ret 
	0    SUCC
	!= 0  ERROR
*/

static int uri_decode(char* uri ,char* outUrl , int nLen)
{
	if (NULL == uri)
		return -1;//

    int len = strlen(uri);
    char *out = NULL;
    char *o = out;
    char *s = uri;
    char *end = uri + strlen(uri);
    int c;

    out = (char*)malloc(len + 1);
    if (!out)
    {
        log_error("{%s:%d} malloc fail.", __FUNCTION__, __LINE__);
        return -2;
    }

    for (o = out; s <= end; o++)
    {
        c = *s++;
        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%' 
            && (!ishex(*s++) || !ishex(*s++) || !sscanf(s - 2, "%2x", &c)))
        {
            // bad uri
            free(out);
            return -3;
        }

        if (out)
        {
            *o = c;
        }
    }
	int nOutLen = strlen(out);
	if (nOutLen > nLen - 1)
		nOutLen = nLen - 1;

    memcpy(outUrl, out, nOutLen);
	outUrl[nOutLen] = 0;
    free(out);
	return 0;
}

static uint8_t ishex(uint8_t x)
{
    return	(x >= '0' && x <= '9')	||
        (x >= 'a' && x <= 'f')	||
        (x >= 'A' && x <= 'F');
}

static int reset_filename_from_formdata(event_t *ev, char **formdata, int size)
{
    char *file_name = NULL;
    char *p         = NULL;
    int   i         = 0;
    int   found     = 0;
    char *anis      = NULL;

    // find "\r\n\r\n"
    p = *formdata;
    for (i = 0; i <= size - 4; i++)
    {
        if (0 == memcmp(*formdata + i, CRLF CRLF, strlen(CRLF CRLF)))
        {
            found = 1;
            (*formdata)[i] = 0;
            (*formdata) += i + strlen(CRLF CRLF);
            break;
        }
    }
    if (!found)
        return 2;

    // file upload file name from formdata
    file_name = strstr(p, "filename=\"");
    if (!file_name)
        return 0;
    file_name += strlen("filename=\"");
    *strstr(file_name, "\"") = 0;

    anis = utf8_to_ansi(file_name);
	if (!anis)
		return 0;
    // IE browser: remove client file path
    p = anis;
    while (*p)
    {
        if (*p == '\\' || *p == '/')
        if (*(p + 1))
            anis = p + 1;
        p++;
    }

	char* sPathVal = session_data_getKey(ev->sessiondata, "storeId");
	if (NULL == sPathVal) {
		sPathVal = get_default_store_path();
	}

	char* sCamID = session_data_getKey(ev->sessiondata, "camId");
	// 每个门店有多个摄像头
	int nCamPath = 0;

	int nIndex = 0;//
	int nTotal = 1;//

	char* sTotal = session_data_getKey(ev->sessiondata, "total");
	char* sIndex = session_data_getKey(ev->sessiondata, "index");

	if (NULL != sTotal && NULL != sIndex) {
		nIndex = atoi(sIndex);
		nTotal = atoi(sTotal);
	}

    // reset filepath
    for (i = strlen(ev->data->file) - 1; i >= 0; i--)
    {
		int nStorePath = 0;
        if (ev->data->file[i] == '/')
        {
			
			if (sPathVal) {
				memset(ev->data->file + i + 1, 
					   0x00, sizeof(ev->data->file) - (i + 1));

				nStorePath = strlen(sPathVal);
				if (nStorePath > 16)
					nStorePath = 16;

				memcpy(ev->data->file + i + 1, sPathVal, nStorePath);
				
				if (nStorePath > 0 && sPathVal[nStorePath - 1] != '/') {
					strcat(ev->data->file, "/");
					nStorePath += 1;
				}
			}
			else {
				memset(ev->data->file + i + 1, 
					0x00, sizeof(ev->data->file) - (i + 1));
			}

			if (sCamID) {// 添加 camID
				nCamPath = strlen(sCamID);
				if (nCamPath > 8)
					nCamPath = 8;//
				strncat(ev->data->file, sCamID , nCamPath);
				strcat(ev->data->file, "/");
				nCamPath += 1;
			}

			if (dir_exist(ev->data->file)) {
				mkdirs(ev->data->file);
			}
			int nFileLen = strlen(anis);
			if (nFileLen > (MAX_PATH - 1 - (i + 1 + nStorePath + nCamPath)))
				nFileLen = (MAX_PATH - 1 - (i + 1 + nStorePath + nCamPath));
			
			memcpy(ev->data->file + i + 1 + nStorePath + nCamPath,
				anis, nFileLen);
			if (nTotal > 1) {
				// 增加对 File Part 支持 
				char szPart[100] = {0};
				sprintf(szPart, "_t_%d_i_%d", nTotal ,nIndex);
				int nLen = strlen(szPart);
				if (nLen > (MAX_PATH - 1 - (i + 1 + nStorePath + nCamPath + nFileLen)))
					nLen = (MAX_PATH - 1 - (i + 1 + nStorePath + nCamPath + nFileLen));
				memcpy(ev->data->file + i + 1 + nStorePath + nCamPath + nFileLen,
					szPart, nLen);
			}
			ev->data->nTotal = nTotal;
            break;
        }
    }

    // if file exist delete file
    if (file_exist(ev->data->file))
    {
        remove_file(ev->data->file);
    }
    free(anis);
    return 1;
}

static int parse_boundary(event_t *ev, char *data, int size, char **ptr)
{
    char first_boundary[BOUNDARY_MAX_LEN]  = { 0 };
    char middle_boundary[BOUNDARY_MAX_LEN] = { 0 };
    char last_boundary[BOUNDARY_MAX_LEN]   = { 0 };
    int  first_len, middle_len, last_len;
    int i;

    sprintf(first_boundary, "--%s\r\n", ev->data->boundary);      //------WebKitFormBoundaryOG3Viw9MEZcexbvT\r\n
    sprintf(middle_boundary, "\r\n--%s\r\n", ev->data->boundary);   //\r\n------WebKitFormBoundaryOG3Viw9MEZcexbvT\r\n
    sprintf(last_boundary, "\r\n--%s--\r\n", ev->data->boundary); //\r\n------WebKitFormBoundaryOG3Viw9MEZcexbvT--\r\n
    first_len  = strlen(first_boundary);
    middle_len = strlen(middle_boundary);
    last_len   = strlen(last_boundary);

    ASSERT(size > first_len);
    ASSERT(size > middle_len);
    ASSERT(size >= last_len);
    if (0 == memcmp(data, first_boundary, first_len))
    {
        *ptr = data + first_len;
        return 1; // first boundary
    }

    for (i = 0; i < size; i++)
    {
        if (size - i >= last_len)
        {
            if (0 == memcmp(data + i, middle_boundary, middle_len))
            {
                *ptr = data + i/* + middle_len*/;
                return 3; // middle boundary
            }

            if (0 == memcmp(data + i, last_boundary, last_len))
            {
                *ptr = data + (size - last_len);
                return 2; // last boundary
            }
        }
        else if (size - i >= middle_len && size - i < last_len)
        {
            if (0 == memcmp(data + i, middle_boundary, middle_len))
            {
                *ptr = data + i/* + middle_len*/;
                return 3; // middle boundary
            }
            if (0 == memcmp(data + i, last_boundary, size - i))
            {
                *ptr = data + i;
                return 4; // backup last boundary
            }
        }
        else if (size - i >= 7 && size - i < middle_len)
        {
            if (0 == memcmp(data + i, middle_boundary, size - i))
            {
                *ptr = data + i;
                return 5; // backup middle boundary
            }
            if (0 == memcmp(data + i, last_boundary, size - i))
            {
                *ptr = data + i;
                return 4; // backup last boundary
            }
        }
        else
        {
            if (0 == memcmp(data + i, middle_boundary, size - i))
            {
                *ptr = data + i;
                return 5; // backup middle boundary
            }
            if (0 == memcmp(data + i, last_boundary, size - i))
            {
                *ptr = data + i;
                return 4; // backup last boundary
            }
        }
    }

    return 0;
}



static const char *reponse_content_type(char *file_name)
{
    const request_fields_t content_type[] =     {
        { "html",   "text/html"                 },
        { "css",    "text/css"                  },
        { "txt",    "text/plain"                },
        { "log",    "text/plain"                },
        { "cpp",    "text/plain"                },
        { "c",      "text/plain"                },
        { "h",      "text/plain"                },
        { "js",     "application/x-javascript"  },
        { "png",    "application/x-png"         },
        { "jpg",    "image/jpeg"                },
        { "jpeg",   "image/jpeg"                },
        { "jpe",    "image/jpeg"                },
        { "gif",    "image/gif"                 },
        { "ico",    "image/x-icon"              },
        { "doc",    "application/msword"        },
        { "docx",   "application/msword"        },
        { "ppt",    "application/x-ppt"         },
        { "pptx",   "application/x-ppt"         },
        { "xls",    "application/x-xls"         },
        { "xlsx",   "application/x-xls"         },
        { "mp4",    "video/mpeg4"               },
        { "mp3",    "audio/mp3"                 }
    };

    char *ext = NULL;
    int i;

    if (!file_name)
    {
        return "text/html";
    }
    ext = file_ext(file_name);
    if (ext)
    {
        for (i = 0; i < sizeof(content_type)/sizeof(content_type[0]); 
				i++)
        {
            if (0 == strcmp(content_type[i].key, ext))
            {
                return content_type[i].value;
            }
        }
    }
    return "application/octet-stream";
}

static const char *response_header_format()
{
    const char *http_header_format =
        "HTTP/1.1 %s" CRLF
        "Content-Type: %s" CRLF
        "Content-Length: %d" CRLF
        CRLF;

    return http_header_format;
}

static const char *response_body_format()
{
    const char *http_body_format =
        "<html>" CRLF
        "<head><title>%s</title></head>" CRLF
        "<body bgcolor=\"white\">" CRLF
        "<center><h1>%s</h1></center>" CRLF
        "</body></html>";

    return http_body_format;
}

static void response_home_page(event_t *ev, char *path)
{
    const char *html_format = 
        "<html>" CRLF
        "<head>" CRLF
        "<meta charset=\"utf-8\">" CRLF
        "<title>Index of /%s</title>" CRLF
        "</head>" CRLF
        "<body bgcolor=\"white\">" CRLF
        "<h1>Index of /%s</h1><hr>" CRLF
        "<form action=\"/upload?path=%s&index=0&total=1&name=jacky\" method=\"post\" enctype=\"multipart/form-data\">" CRLF
        "<input type=\"file\" name=\"file\" multiple=\"true\" />" CRLF
        "<input type=\"submit\" value=\"Upload\" /></form><hr><pre>" CRLF
        "%s" CRLF
        "</body></html>";

    char header[BUFFER_UNIT] = { 0 };
    event_data_t* ev_data = NULL;
    int length;
	char file_path[MAX_PATH] = { 0 };
    char *file_list = NULL;
    char *html = NULL;
    event_t ev_ = {0};
    char *utf8 = NULL;

	utf8 = ansi_to_utf8(path);
	if (NULL == utf8) {
		response_http_404_page(ev);
		return ;
	}
		
	memcpy(file_path, root_path(), strlen(root_path()));
	int nLen = MAX_PATH - strlen(root_path()) - 1;
	if ((strlen(utf8) + 1) < nLen ) {
		nLen = strlen(utf8) + 1;
	}
		
	log_info("{%s:%s:%d} response_home_page %s", __FILE__,
		__FUNCTION__, __LINE__, path);

	strncat(file_path, utf8 , nLen);
    file_list = local_file_list(file_path);
	if (!file_list) {
		log_error("{%s:%s:%d} local_file_list %s is null.",
			__FILE__, __FUNCTION__, __LINE__,file_path);

		free(utf8);
		response_http_404_page(ev);
		return ;
	}
        
    length = strlen(html_format) + strlen(file_list) + (strlen(utf8) - strlen("%s"))*3 + 1;
    html = (char*)malloc(length);
    if (!html)
    {
        log_error("{%s:%s:%d} malloc fail.", __FILE__,__FUNCTION__, __LINE__);
		free(utf8);
		free(file_list);
		response_http_404_page(ev);
        return ;
    }
    sprintf(html, html_format, utf8, utf8, utf8, file_list);
    free(utf8);
    free(file_list);
    sprintf(header, response_header_format(), "200 OK", reponse_content_type(NULL), strlen(html));
    ev_data = create_event_data(header, html);
    free(html);

    ev_.fd = ev->fd;
    ev_.ip = ev->ip;

    ev_.type = EV_WRITE;
    ev_.param = ev->param;
    ev_.data = ev_data;
    ev_.callback = write_callback;
	ev_.sessiondata = NULL;
    event_add(&ev_);
}

static void response_send_file_page(event_t *ev, char *file_name)
{
    char header[BUFFER_UNIT] = { 0 };
    FILE* fp = NULL;
    int total = 0;
    int len = 0;
    event_data_t* ev_data = NULL;
    event_t ev_ = {0};

    fp = fopen(file_name, "rb");
    if (!fp)
    {
        log_error("{%s:%d} open [%s] failed, errno=%d", __FUNCTION__, __LINE__, file_name, errno);
        release_event_data(ev);
        response_http_404_page(ev);
        goto end;
    }
    if (ev->data == NULL)
    {
        fseek(fp, 0, SEEK_END);
        total = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        len = total > BUFFER_UNIT ? BUFFER_UNIT : total;
        sprintf(header, response_header_format(), "200 OK", reponse_content_type(file_name), total);
        ev_data = create_event_data_fp(header, fp, len, total);
		if (ev_data)//ev_data can be null 
		{
			memcpy(ev_data->file, file_name, strlen(file_name));
			ev_data->offset += len;
		}
    }
    else
    {
        fseek(fp, ev->data->offset, SEEK_SET);
        len = ev->data->total - ev->data->offset > BUFFER_UNIT ? BUFFER_UNIT : ev->data->total - ev->data->offset;
        ev_data = create_event_data_fp(NULL, fp, len, ev->data->total);
		if (ev_data)
		{
			memcpy(ev_data->file, file_name, strlen(file_name));
			ev_data->offset = ev->data->offset + len;
		}
        release_event_data(ev);
    }

    ev_.fd = ev->fd;
    ev_.ip = ev->ip;
    ev_.type = EV_WRITE;
    ev_.param = ev->param;
    ev_.data = ev_data;
    ev_.callback = write_callback;
	ev_.sessiondata = NULL;
    event_add(&ev_);

end:
    if (fp)
        fclose(fp);
}

static void response_upload_page(event_t *ev, int result)
{
    if (result)
    {
        send_response(ev, "Upload completed", "200 OK");
    }
    else
    {
        send_response(ev, "Upload failed", "200 OK");
    }
}

static void response_http_400_page(event_t *ev)
{
    send_response(ev, "400 Bad Request", NULL);
}

static void response_http_404_page(event_t *ev)
{
    send_response(ev, "404 Not Found", NULL);
}

static void response_http_500_page(event_t *ev)
{
    send_response(ev, "500 Internal Server Error", NULL);
}

static void response_http_501_page(event_t *ev)
{
    send_response(ev, "501 Not Implemented", NULL);
}

static void send_response(event_t *ev, char *title, char *status)
{
    char header[BUFFER_UNIT] = { 0 };
    char body[BUFFER_UNIT]   = { 0 };
    event_data_t* ev_data = NULL;
    event_t ev_ = {0};

    sprintf(body, response_body_format(), title, title);
    sprintf(header, response_header_format(), status ? status : title, reponse_content_type(NULL), strlen(body));
    ev_data = create_event_data(header, body);

    ev_.fd = ev->fd;
    ev_.ip = ev->ip;
    ev_.type = EV_WRITE;
    ev_.param = ev->param;
    ev_.data = ev_data;
    ev_.callback = write_callback;
	ev_.sessiondata = NULL;
    event_add(&ev_);
}


static void new_merge_task(const char* fileName, int nIndex , int nTotal) {
	MergeInfo mergeInfo;
	int nLen = strlen(fileName);

	if (nTotal <= 1)
		return;//

	if (nIndex != nTotal - 1) {
		log_info("{%s:%s:%d} mergeInfo %s ,%d ,index %d != nTotal - 1 ",
			__FILE__, __FUNCTION__, __LINE__,
			fileName, nTotal , nIndex);
		return ;
	}
	

	nLen = nLen > MAX_PATH - 1 ? MAX_PATH - 1 : nLen;
	strncpy(mergeInfo.fileName, fileName, nLen);
	mergeInfo.totalNo = nTotal;

	writepipe(&mergeInfo);

	log_info("{%s:%d} mergeInfo %s ,%d", __FUNCTION__, __LINE__, 
		fileName, nTotal);
}