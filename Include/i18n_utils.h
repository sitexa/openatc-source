#ifndef I18N_UTILS__H_
#define I18N_UTILS__H_

#include "libintl.h"
#include <iconv.h>

#ifndef _
#define _(string) gettext(string)
#endif

#endif//I18N_UTILS__H_