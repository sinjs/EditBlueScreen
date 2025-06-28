#include <ntddk.h>

#include "Types.h"
#include "BlueScreen.h"
#include "Config.h"

PBSOD_INFORMATION g_BsodInformation = NULL;

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	g_BsodInformation = (PBSOD_INFORMATION)
		ExAllocatePool(NonPagedPoolNx, sizeof(BSOD_INFORMATION));

	if (g_BsodInformation == NULL)
	{
		DbgPrint("ExAllocatePool failed: %08x\n", STATUS_INSUFFICIENT_RESOURCES);
		return STATUS_FAILED_DRIVER_ENTRY;
	}
	else
	{
		RtlSecureZeroMemory(g_BsodInformation, sizeof(BSOD_INFORMATION));
	}

	g_BsodInformation->Face = RTL_CONSTANT_STRING(BSOD_FACE);
	g_BsodInformation->Message = RTL_CONSTANT_STRING(BSOD_FIRST_MESSAGE);
	g_BsodInformation->FooterMessage = RTL_CONSTANT_STRING(BSOD_SECOND_MESSAGE);

	if (!NT_SUCCESS(BsodInitialize())) {
		DbgPrint("BsodInitialize failed\n");
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	if (!NT_SUCCESS(BsodOverwriteFace())) {
		DbgPrint("BsodOverwriteFace failed\n");
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	if (!NT_SUCCESS(BsodOverwriteMessage(
		g_BsodInformation->BsodMessageOne,
		&g_BsodInformation->Message
	))) {
		DbgPrint("BsodOverwriteMessage (BsodMessageOne) failed\n");
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	if (!NT_SUCCESS(BsodOverwriteMessage(
		g_BsodInformation->BsodMessageTwo,
		&g_BsodInformation->FooterMessage
	))) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BsodOverwriteMessage (BsodMessageTwo) failed\n");
		return STATUS_FAILED_DRIVER_ENTRY;
	}

	if (!NT_SUCCESS(BsodOverwriteColor(BSOD_COLOR))) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "BsodOverwriteColor failed\n");
	    return STATUS_FAILED_DRIVER_ENTRY;
	}

	ExFreePool(g_BsodInformation);

	return STATUS_FAILED_DRIVER_ENTRY;
}