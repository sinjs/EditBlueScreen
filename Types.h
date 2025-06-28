#pragma once
#include <ntdef.h>

typedef struct _BSOD_INFORMATION
{
	UNICODE_STRING Face;
	UNICODE_STRING Message;
	UNICODE_STRING FooterMessage;

	// Resolved
	UINT64 Sadface = NULL;
	UINT64 EtwpLastBranchEntry = NULL;
	UINT64 AddrColorVar = NULL;
	UINT8 Offset = NULL;
	UINT8 ColorOffset = NULL;
	PUNICODE_STRING BsodMessageOne;
	PUNICODE_STRING BsodMessageTwo;
} BSOD_INFORMATION, *PBSOD_INFORMATION;

extern PBSOD_INFORMATION g_BsodInformation;
