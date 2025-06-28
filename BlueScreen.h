#pragma once

#include <ntddk.h>

NTSTATUS BsodInitialize();
NTSTATUS BsodOverwriteFace();
NTSTATUS BsodOverwriteMessage(PUNICODE_STRING error, PUNICODE_STRING replace);
NTSTATUS BsodOverwriteColor(UINT32 color);
