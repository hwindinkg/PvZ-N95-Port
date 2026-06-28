/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * Ported from PvZ-Portable Sexy.TodLib/TodDebug.cpp for Symbian S60 3rd FP1.
 * Debug utilities using Symbian RDebug::Print and User::Panic.
 */

#include <time.h>
#include <stdarg.h>
#include <string.h>

#include <e32def.h>
#include <e32debug.h>
#include <e32std.h>

#include "TodDebug.h"
#include "TodCommon.h"
#include "../engine/Common.h"

// ---------------------------------------------------------------------------
// TodErrorMessageBox  --  display critical error, then panic
// ---------------------------------------------------------------------------
void TodErrorMessageBox(const char* theMessage, const char* theTitle)
{
	RDebug::Print(_L("ERROR: %s - %s"), theTitle, theMessage);
	User::Panic(_L("TodError"), 1);
}

// ---------------------------------------------------------------------------
// TodTraceMemory  --  no-op on Symbian
// ---------------------------------------------------------------------------
void TodTraceMemory()
{
}

// ---------------------------------------------------------------------------
// TodMalloc / TodFree  --  plain C heap wrappers
// ---------------------------------------------------------------------------
void* TodMalloc(int theSize)
{
	TOD_ASSERT(theSize > 0);
	return User::Alloc(theSize);
}

void TodFree(void* theBlock)
{
	if (theBlock != NULL)
	{
		User::Free(theBlock);
	}
}

// ---------------------------------------------------------------------------
// TodAssertFailed  --  format assertion message, log via RDebug, then panic
// ---------------------------------------------------------------------------
void TodAssertFailed(const char* theCondition, const char* theFile, int theLine, const char* theMsg, ...)
{
	char aFormattedMsg[1024];
	va_list argList;
	va_start(argList, theMsg);
	int aCount = TodVsnprintf(aFormattedMsg, sizeof(aFormattedMsg), theMsg, argList);
	va_end(argList);

	if (aCount != 0)
	{
		if (aFormattedMsg[aCount - 1] != '\n')
		{
			if (aCount + 1 < 1024)
			{
				aFormattedMsg[aCount] = '\n';
				aFormattedMsg[aCount + 1] = '\0';
			}
			else
			{
				aFormattedMsg[aCount - 1] = '\n';
			}
		}
	}

	char aBuffer[1024];
	if (*theCondition != '\0')
	{
		TodSnprintf(aBuffer, sizeof(aBuffer), "\n%s(%d)\nassertion failed: '%s'\n%s\n", theFile, theLine, theCondition, aFormattedMsg);
	}
	else
	{
		TodSnprintf(aBuffer, sizeof(aBuffer), "\n%s(%d)\nassertion failed: %s\n", theFile, theLine, aFormattedMsg);
	}

	RDebug::Print(_L("ASSERT: %s"), aBuffer);
	User::Panic(_L("TodAssert"), theLine);
}

// ---------------------------------------------------------------------------
// TodLog / TodLogString
// ---------------------------------------------------------------------------
void TodLog(const char* theFormat, ...)
{
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	TodLogString(aButter);
}

void TodLogString(const char* theMsg)
{
#ifdef _DEBUG
	RDebug::Print(_L("TodLog: %s"), theMsg);
#else
	(void)theMsg;
#endif
}

// ---------------------------------------------------------------------------
// TodTrace
// ---------------------------------------------------------------------------
void TodTrace(const char* theFormat, ...)
{
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	RDebug::Print(_L("%s"), aButter);
}

// ---------------------------------------------------------------------------
// TodHesitationTrace  --  no-op
// ---------------------------------------------------------------------------
void TodHesitationTrace(...)
{
}

// ---------------------------------------------------------------------------
// TodTraceAndLog
// ---------------------------------------------------------------------------
void TodTraceAndLog(const char* theFormat, ...)
{
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	RDebug::Print(_L("%s"), aButter);
	TodLogString(aButter);
}

// ---------------------------------------------------------------------------
// TodTraceWithoutSpamming  --  at most once per second
// ---------------------------------------------------------------------------
void TodTraceWithoutSpamming(const char* theFormat, ...)
{
	static unsigned long gLastTraceTime = 0;
	unsigned long aTime = static_cast<unsigned long>(time(NULL));
	if (aTime < gLastTraceTime)
	{
		return;
	}

	gLastTraceTime = aTime;
	char aButter[1024];
	va_list argList;
	va_start(argList, theFormat);
	int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
	va_end(argList);

	if (aButter[aCount - 1] != '\n')
	{
		if (aCount + 1 < 1024)
		{
			aButter[aCount] = '\n';
			aButter[aCount + 1] = '\0';
		}
		else
		{
			aButter[aCount - 1] = '\n';
		}
	}

	RDebug::Print(_L("%s"), aButter);
}

// ---------------------------------------------------------------------------
// TodAssertInitForApp  --  minimal init for Symbian
// ---------------------------------------------------------------------------
void TodAssertInitForApp()
{
	TodLog("Started %d\n", static_cast<int>(time(NULL)));
}
