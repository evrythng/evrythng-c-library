#include <stdarg.h>
#include "Messages.h"
#include "Thread.h"
#include "marvell_api.h"

extern time_t wmtime_time_get_posix();

void time(time_t *c_time)
{
	*c_time = wmtime_time_get_posix();
}

static int Log_level=2;

void Log_setLevel(int level)
{
  Log_level = level;
}

#if defined(EVRYTHNG_DEBUG)
mutex_type log_mutex;

static char* Log_level_string[] =
{
  "",
  "MAXIMUM",
  "MEDIUM",
  "MINIMUM",
  "PROTOCOL",
  "ERROR",
  "SEVERE",
  "FATAL",
};

void Log(int log_level, int msgno, char* format, ...)
{
	if (log_level >= Log_level)
	{
		char* temp = NULL;
		static char msg_buf[256];
		va_list args;

		/* we're using a static character buffer, so we need to make sure only one thread uses it at a time */
		Thread_lock_mutex(log_mutex); 
		if (format == NULL && (temp = Messages_get(msgno, log_level)) != NULL)
			format = temp;

		va_start(args, format);
		vsnprintf(msg_buf, sizeof(msg_buf), format, args);

		wmprintf("%s: %s\n\r", Log_level_string[log_level], msg_buf);
		va_end(args);
		Thread_unlock_mutex(log_mutex); 
	}
}
#endif
