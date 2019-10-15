#include "editor.h"

void	set_status_msg(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_editor.status_msg, sizeof(g_editor.status_msg), fmt, ap);
	va_end(ap);
	g_editor.status_msg_time = time(NULL);
}
