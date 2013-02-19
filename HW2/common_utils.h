//#include<string>
#pragma once
#include<time.h>
#include<sys/time.h>

#define TIME_STR_MAX_SIZE 100
static char timeStr[TIME_STR_MAX_SIZE];


//#define cout std::cout<<getTimeStr()<<thread_info_map[pthread_self()]<<" : " 
#define cout std::cout

#define COUT cout

#define PRINT(a,b) \
	if(a>=loglevel) \
{\
	cout<<b<<std::endl<<std::flush;\
	outFile<<b;\
}




#define PRINT_PACKET(pkt,dir) \
	PRINT(LOG_DEBUG, "==========START=======================\n"); \
PRINT(LOG_DEBUG, " ----"<<dir<<" Packet Info----\n"); \
PRINT(LOG_DEBUG, " Conn_Id:  "<<pkt.conn_id<<"\n"); \
PRINT(LOG_DEBUG, " Seq_no:  "<<pkt.seq_no<<"\n"); \
PRINT(LOG_DEBUG, " Payload: "<<pkt.data<<"\n"); \
PRINT(LOG_DEBUG, "==========END=======================\n");

/*typedef enum loglevels{
        LOG_DEBUG=1,
        LOG_INFO =2,
        LOG_CRIT =3
}loglevels;
*/

inline const char*	getTimeStr(void)
{
	time_t t (time(0));
	const char *log_date_format = "%d/%m/%y %H:%M:%S";
	memset((void *)timeStr,'\0',TIME_STR_MAX_SIZE);
	tm *tb=NULL;
	if ( (tb = localtime(&t)) == NULL ) {
	        //PRINT(LOG_INFO, "ERROR: In getTimeStr tb is NULL\n");
		return timeStr;
	}
	strftime(timeStr,TIME_STR_MAX_SIZE,log_date_format,tb);
	struct timeval tv;

	if (gettimeofday(&tv,NULL) < 0) {
		//PRINT(LOG_INFO, "ERROR: In getTimeStr gettimeofday failedL\n");
		return timeStr;
	}

	unsigned int msec = (tv.tv_usec/1000);
	char millisec[4];

	if( msec < 10 )
	{
		sprintf(millisec,":00%d",msec);
	}
	else if( msec < 100 )
	{
		sprintf(millisec,":0%d",msec);
	}
	else
	{
		sprintf(millisec,":%d",msec);
	}

	std::string timeString = strncat(timeStr,millisec,4);

	timeStr[strlen(timeStr)] = '\0';
	return timeStr;
}
