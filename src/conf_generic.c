#include "config.h"	// Various macros
#include "typedefs.h"	// typedefs

void conf_initialise(bool sync)
{
}

char *conf_read_str(const char *area, const char *subarea,
		    const char *name, char *def)
{
}

void conf_write_str(const char *area, const char *subarea,
		    const char *name, char *val)
{
}

int conf_read_int(const char *area, const char *subarea, const char *name,
		  int def)
{
}

void conf_write_int(const char *area, const char *subarea,
		    const char *name, int val)
{
}

void conf_sync()
{
}

void conf_dispose()
{
}
