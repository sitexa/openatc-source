/*
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "../libintl.h"

#include <map>
#include <string>

#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined(WINCE)
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#include "MessageCatalog.hpp"
#include "Util.hpp"

using namespace std;

using namespace libintllite;
using namespace libintllite::internal;

static char* currentDefaultDomain = NULL;
static map<char*, MessageCatalog*> loadedMessageCatalogPtrsByDomain;

libintl_lite_bool_t loadMessageCatalog(const char* domain, const char* moFilesRoot)
{
	if (!moFilesRoot || !domain)
	{
		return LIBINTL_LITE_BOOL_FALSE;
	}

	FILE* moFile = NULL;
	CloseFileHandleGuard closeFileHandleGuard(moFile);

	for (const string &path : buildMoFilePaths(domain))
	{
		string fullPath = moFilesRoot + path;
		moFile = fopen(fullPath.c_str(), "rb");
		if (loadMessageCatalogFile(domain, moFile) == LIBINTL_LITE_BOOL_TRUE)
		{
			return LIBINTL_LITE_BOOL_TRUE;
		}
	}

	return LIBINTL_LITE_BOOL_FALSE;
}

libintl_lite_bool_t loadMessageCatalogFile(const char* domain, FILE* moFile)
{
	try
	{
		if (sizeof(uint32_t) != 4)
		{
			return LIBINTL_LITE_BOOL_FALSE;
		}

		if (!moFile || !domain)
		{
			return LIBINTL_LITE_BOOL_FALSE;
		}

		uint32_t magicNumber;
		if (!readUIn32FromFile(moFile, false, magicNumber)) return LIBINTL_LITE_BOOL_FALSE;
		if ((magicNumber != 0x950412de) && (magicNumber != 0xde120495)) return LIBINTL_LITE_BOOL_FALSE;

		uint32_t fileFormatRevision;
		if (!readUIn32FromFile(moFile, false, fileFormatRevision)) return LIBINTL_LITE_BOOL_FALSE;
		if (fileFormatRevision != 0) return LIBINTL_LITE_BOOL_FALSE;

		bool needsBeToLeConversion = isBigEndian();

		uint32_t numberOfStrings;
		if (!readUIn32FromFile(moFile, needsBeToLeConversion, numberOfStrings)) return false;
		if (numberOfStrings == 0)
		{
			return LIBINTL_LITE_BOOL_TRUE;
		}

		uint32_t offsetOrigTable;
		if (!readUIn32FromFile(moFile, needsBeToLeConversion, offsetOrigTable)) return LIBINTL_LITE_BOOL_FALSE;

		uint32_t offsetTransTable;
		if (!readUIn32FromFile(moFile, needsBeToLeConversion, offsetTransTable)) return LIBINTL_LITE_BOOL_FALSE;

		string* sortedOrigStringsArray = NULL;
		ArrayGurard<string> sortedOrigStringsArrayGuard(sortedOrigStringsArray);
		sortedOrigStringsArray = new string[numberOfStrings];
		if (!sortedOrigStringsArray)
		{
			return LIBINTL_LITE_BOOL_FALSE;
		}

		if (!loadMoFileStringsToArray(moFile,
				numberOfStrings,
				offsetOrigTable,
				needsBeToLeConversion,
				sortedOrigStringsArray)) return LIBINTL_LITE_BOOL_FALSE;

		string* translatedStringsArray = NULL;
		ArrayGurard<string> translatedStringsArrayGuard(translatedStringsArray);
		translatedStringsArray = new string[numberOfStrings];
		if (!translatedStringsArray)
		{
			return LIBINTL_LITE_BOOL_FALSE;
		}

		if (!loadMoFileStringsToArray(moFile,
				numberOfStrings,
				offsetTransTable,
				needsBeToLeConversion,
				translatedStringsArray)) return LIBINTL_LITE_BOOL_FALSE;

		MessageCatalog* newMessageCatalogPtr = new MessageCatalog(numberOfStrings,
				sortedOrigStringsArray,
				translatedStringsArray);
		if (!newMessageCatalogPtr) return LIBINTL_LITE_BOOL_FALSE;
		sortedOrigStringsArrayGuard.release();
		translatedStringsArrayGuard.release();

		char* domainDup = strdup(domain);
		if (!domainDup) return LIBINTL_LITE_BOOL_FALSE;
		closeLoadedMessageCatalog(domain);
		loadedMessageCatalogPtrsByDomain[domainDup] = newMessageCatalogPtr;

		return LIBINTL_LITE_BOOL_TRUE;
	}
	catch (...)
	{
		return LIBINTL_LITE_BOOL_FALSE;
	}
}

libintl_lite_bool_t bindtextdomain(const char* domain, const char* moFilePath)
{
	return loadMessageCatalog( domain, moFilePath );
}

libintl_lite_bool_t bind_textdomain_codeset(const char* domain, const char* moFilePath)
{
	// not implemented yet
	return LIBINTL_LITE_BOOL_FALSE;
}

void closeLoadedMessageCatalog(const char* domain)
{
	if (domain)
	{
		for (map<char*, MessageCatalog*>::iterator i = loadedMessageCatalogPtrsByDomain.begin();
				i != loadedMessageCatalogPtrsByDomain.end();
				i++)
		{
			if (strcmp(i->first, domain) == 0)
			{
				free(i->first);
				delete i->second;
				loadedMessageCatalogPtrsByDomain.erase(i);
				return;
			}
		}
	}
}

void closeAllLoadedMessageCatalogs()
{
	for (map<char*, MessageCatalog*>::iterator i = loadedMessageCatalogPtrsByDomain.begin();
			i != loadedMessageCatalogPtrsByDomain.end();
			i++)
	{
		free(i->first);
		delete i->second;
	}
	loadedMessageCatalogPtrsByDomain.clear();
	free(currentDefaultDomain);
	currentDefaultDomain = NULL;
}

const char* textdomain(const char* domain)
{
	if (domain)
	{
		char* newDefaultDomain = strdup(domain);
		if (!newDefaultDomain)
		{
			return NULL;
		}
		free(currentDefaultDomain);
		currentDefaultDomain = newDefaultDomain;
		return newDefaultDomain;
	}
	else
	{
		return NULL;
	}
}

const char* gettext(const char* origStr)
{
	return dgettext(NULL, origStr);
}

const char* dgettext(const char* domain, const char* origStr)
{
	if (!origStr)
	{
		return NULL;
	}

	if (!domain)
	{
		if (currentDefaultDomain)
		{
			domain = currentDefaultDomain;
		}
		else
		{
			return origStr;
		}
	}

	const MessageCatalog* msgCat = NULL;
	for (map<char*, MessageCatalog*>::iterator i = loadedMessageCatalogPtrsByDomain.begin();
			!msgCat && (i != loadedMessageCatalogPtrsByDomain.end());
			i++)
	{
		if (strcmp(i->first, domain) == 0)
		{
			msgCat = i->second;
		}
	}

	if (!msgCat)
	{
		return origStr;
	}

	const string* translatedStrPtr = msgCat->getTranslatedStrPtr(origStr);
	if (translatedStrPtr)
	{
		return translatedStrPtr->c_str();
	}
	else
	{
		return origStr;
	}
}

const char* ngettext(const char* origStr, const char* origStrPlural, unsigned long n)
{
	if (n == 1)
	{
		return gettext(origStr);
	}
	else
	{
		return gettext(origStrPlural);
	}
}

const char* dngettext(const char* domain, const char* origStr, const char* origStrPlural, unsigned long n)
{
	if (n == 1)
	{
		return dgettext(domain, origStr);
	}
	else
	{
		return dgettext(domain, origStrPlural);
	}
}
