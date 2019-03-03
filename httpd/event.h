#ifndef __EVENT_H__
#define __EVENT_H__

#ifndef WIN32
#define MAX_PATH 260
#include <sys/select.h> 
#include <sys/time.h> 
#endif



#define BOUNDARY_MAX_LEN    128

typedef enum
{
    EV_UNKNOWN              = 0x00,
    EV_READ                 = 0x01,
    EV_WRITE                = 0x02,
    EV_EXCEPT               = 0x04,
    EV_PERSIST              = 0x08
} event_type_t;

typedef enum  
{
    EV_IDLE                  = 0,
    EV_BUSY,
    EV_FINISH
} event_status_t;

typedef struct
{
    char                    file[MAX_PATH];
    char                    boundary[BOUNDARY_MAX_LEN];
	char					fileName[MAX_PATH];
    uint32_t                total;
    uint32_t                offset;
    uint32_t                size;
	uint16_t				nTotal; //if file is split to multipart 
    FILE                   *fp;         // fixed fopen EACCES error. just for write file
    char                    data[1];
} event_data_t;

typedef struct __event_session_data {
	char**  pKey;
	char**  pValue;
	int	   nCount;
	
}event_session_data;

typedef struct event_t event_t;
struct event_t
{
    uint32_t                fd;         // socket
    uint32_t                ip;         // ip 
    uint8_t                 type;       // event_type_t
    uint8_t                 status;     // event_status_t
	void                   *param;      // param
    event_data_t           *data;       // event_data_t*
	event_session_data	   *sessiondata;
    void (*callback) (event_t*);
};

ret_code_t event_init();
ret_code_t event_uninit();
ret_code_t event_add(event_t *ev);
ret_code_t event_del(event_t *ev);
ret_code_t event_dispatch();

int session_data_init(event_session_data* pData,int nCnt, char** pKey, char** pValue);
void release_session_data(event_session_data* pData);
char* session_data_getKey(event_session_data* pData, char* pKey);


#endif