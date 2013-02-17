//#include<string>
#include<time.h>
#include<sys/time.h>

#define TIME_STR_MAX_SIZE 100
static char timeStr[TIME_STR_MAX_SIZE];
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
