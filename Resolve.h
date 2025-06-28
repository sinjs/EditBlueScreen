#pragma once

#include <ntdef.h>

NTSTATUS ResolveKeBugCheck2(UINT64 address, PUINT64 result);
NTSTATUS ResolveKiDisplayBlueScreen(UINT64 address, PUINT64 result);
NTSTATUS ResolveBgpFwDisplayBugCheckScreen(UINT64 address, PUINT64 result);
NTSTATUS ResolveGlobalPhrases(UINT64 address);