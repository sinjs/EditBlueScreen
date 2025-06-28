#include "BlueScreen.h"
#include "Resolve.h"
#include "Types.h"

NTSTATUS BsodInitialize()
{
	UNICODE_STRING SystemRoutineName = RTL_CONSTANT_STRING(L"KeBugCheckEx");

	NTSTATUS Status;

	UINT64 KeBugCheck2 = NULL;
	UINT64 KeDisplayBlueScreen = NULL;
	UINT64 BgpFwDisplayBugCheckScreen = NULL;

	PVOID KeBugCheckExAddress = MmGetSystemRoutineAddress(&SystemRoutineName);
	if (KeBugCheckExAddress == NULL) {
		DbgPrint("Unable to resolve KeBugCheckEx\n");
		return STATUS_NOT_FOUND;
	}

	Status = ResolveKeBugCheck2((UINT64)KeBugCheckExAddress, &KeBugCheck2);
	if (!NT_SUCCESS(Status) || KeBugCheck2 == NULL) {
		DbgPrint("Unable to resolve KeBugCheck2\n");
		return STATUS_NOT_FOUND;
	}

	Status = ResolveKiDisplayBlueScreen((UINT64)KeBugCheck2, &KeDisplayBlueScreen);
	if (!NT_SUCCESS(Status) || KeDisplayBlueScreen == NULL) {
		DbgPrint("Unable to resolve KiDisplayBlueScreen\n");
		return STATUS_NOT_FOUND;
	}

	Status = ResolveBgpFwDisplayBugCheckScreen((UINT64)KeDisplayBlueScreen, &BgpFwDisplayBugCheckScreen);
	if (!NT_SUCCESS(Status) || BgpFwDisplayBugCheckScreen == NULL) {
		DbgPrint("Unable to resolve BgpFwDisplayBugCheckScreen\n");
		return STATUS_NOT_FOUND;
	}

	ResolveGlobalPhrases(BgpFwDisplayBugCheckScreen);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Resolved offsets:\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KeBugCheckEx: %08x\n", KeBugCheckExAddress);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KeBugCheck2: %08x\n", KeBugCheck2);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KeDisplayBlueScreen: %08x\n", KeDisplayBlueScreen);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BgpFwDisplayBugCheckScreen: %08x\n", BgpFwDisplayBugCheckScreen);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ColorOffset: %08x\n", g_BsodInformation->ColorOffset);


	return STATUS_SUCCESS;
}

NTSTATUS BsodOverwriteFace()
{
	PHYSICAL_ADDRESS PhysicalAddress = MmGetPhysicalAddress((PVOID)g_BsodInformation->Sadface);
	PUNICODE_STRING MappedAddress = (PUNICODE_STRING)MmMapIoSpace(PhysicalAddress, sizeof(UNICODE_STRING), MmNonCached);

	if (MappedAddress == NULL) {
		DbgPrint("BsodOverwriteFace: Sadface: MmMapIoSpace failed: %08x\n", STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	MappedAddress->Length = g_BsodInformation->Face.Length;
	MappedAddress->MaximumLength = g_BsodInformation->Face.MaximumLength;

	PHYSICAL_ADDRESS PhysicalAddressBuffer = MmGetPhysicalAddress(MappedAddress->Buffer);
	PVOID MappedBuffer = MmMapIoSpace(PhysicalAddressBuffer, MappedAddress->MaximumLength, MmNonCached);

	if (MappedBuffer == NULL) {
		MmUnmapIoSpace(MappedAddress, sizeof(UNICODE_STRING));
		DbgPrint("BsodOverwriteFace: Buffer: MmMapIoSpace failed: %08x\n", STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlCopyMemory(MappedBuffer, g_BsodInformation->Face.Buffer, g_BsodInformation->Face.Length);

	MmUnmapIoSpace(MappedBuffer, MappedAddress->MaximumLength);
	MmUnmapIoSpace(MappedAddress, sizeof(UNICODE_STRING));

	return STATUS_SUCCESS;
}

NTSTATUS BsodOverwriteMessage(PUNICODE_STRING error, PUNICODE_STRING replace)
{
	PHYSICAL_ADDRESS PhysicalAddress = MmGetPhysicalAddress((PVOID)error);
	PUNICODE_STRING MappedAddress = (PUNICODE_STRING)MmMapIoSpace(PhysicalAddress, sizeof(UNICODE_STRING), MmNonCached);

	if (MappedAddress == NULL) {
		DbgPrint("BsodOverwriteMessage: MmMapIoSpace failed: %08x\n", STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	MappedAddress->Length = replace->Length;
	MappedAddress->MaximumLength = replace->MaximumLength;

	PHYSICAL_ADDRESS PhysicalAddressBuffer = MmGetPhysicalAddress(MappedAddress->Buffer);
	PVOID MappedBuffer = MmMapIoSpace(PhysicalAddressBuffer, MappedAddress->MaximumLength, MmNonCached);

	if (MappedBuffer == NULL) {
		MmUnmapIoSpace(MappedAddress, sizeof(UNICODE_STRING));
		DbgPrint("BsodOverwriteMessage: Buffer: MmMapIoSpace failed: %08x\n", STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlCopyMemory(MappedBuffer, replace->Buffer, replace->Length);

	MmUnmapIoSpace(MappedBuffer, MappedAddress->MaximumLength);
	MmUnmapIoSpace(MappedAddress, sizeof(UNICODE_STRING));

	return STATUS_SUCCESS;
}

NTSTATUS BsodOverwriteColor(UINT32 color)
{
	RtlCopyMemory(
		&g_BsodInformation->AddrColorVar,
		(PVOID)g_BsodInformation->EtwpLastBranchEntry,
		sizeof(g_BsodInformation->EtwpLastBranchEntry)
	);

	g_BsodInformation->AddrColorVar += g_BsodInformation->Offset;

	RtlCopyMemory(
		&g_BsodInformation->AddrColorVar,
		(PVOID)g_BsodInformation->AddrColorVar,
		sizeof(g_BsodInformation->AddrColorVar)
	);

	g_BsodInformation->AddrColorVar += g_BsodInformation->ColorOffset;

	*(PUINT32)g_BsodInformation->AddrColorVar = color;

	return STATUS_SUCCESS;
}
