#include "Resolve.h"
#include "Types.h"
#include <ntddk.h>

NTSTATUS ResolveKeBugCheck2(UINT64 address, PUINT64 result)
{
    UINT8 Signature[] = {
        0x45, 0x33, 0xc9,  // xor r9d, r9d
		0x45, 0x33, 0xc0,  // xor r8d, r8d
		0x33, 0xd2,        // xor edx, edx
        0xe8               // call nt!KeBugCheck2
    };

    UINT8 FunctionEnd[] = {
        0x90,              // nop
        0xc3,              // ret
        0xcc,              // int 3
        0xcc,              // int 3
        0xcc,              // int 3
        0xcc               // int 3
    };

    SIZE_T FunctionEndLength = 0;

    while (FunctionEndLength != sizeof(FunctionEnd)) {
        SIZE_T Length = RtlCompareMemory((PVOID)address, Signature, sizeof(Signature));
		if (Length == sizeof(Signature)) {
            UINT32 Offset = 0;

            RtlCopyMemory(
                &Offset,
                (PVOID)(address + Length),
                sizeof(Offset)
            );

			*result = address + Length + Offset + sizeof(Offset);

            return STATUS_SUCCESS;
		}

		FunctionEndLength = RtlCompareMemory(
			(PVOID)address,
			FunctionEnd,
			sizeof(FunctionEnd)
		);

        address++;
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS ResolveKiDisplayBlueScreen(UINT64 address, PUINT64 result)
{
	UINT8 Signature[] = {
		0x0f, 0x45, 0xc1,  // cmovne eax, ecx
		0x8b, 0xc8,        // mov ecx, eax
		0x83, 0xc9, 0x01,  // or ecx, 1
		0x45, 0x84, 0xf6,  // test r14b, r14b
		0x0f, 0x45, 0xc8,  // cmovne ecx, eax
		0xe8               // call nt!KiDisplayBlueScreen
	};

    UINT8 FunctionEnd[] = {
        0x5f,              // pop rdi
        0x5e,			   // pop rsi
        0x5d,              // pop rbp
        0xc3               // ret
    };

    SIZE_T FunctionEndLength = 0;

    while (FunctionEndLength != sizeof(FunctionEnd)) {
        SIZE_T Length = RtlCompareMemory((PVOID)address, Signature, sizeof(Signature));
        if (Length == sizeof(Signature)) {
            UINT32 Offset = 0;

            RtlCopyMemory(
                &Offset,
                (PVOID)(address + Length),
                sizeof(Offset)
            );

            *result = address + Length + Offset + sizeof(Offset);

            return STATUS_SUCCESS;
        }

        FunctionEndLength = RtlCompareMemory(
            (PVOID)address,
            FunctionEnd,
            sizeof(FunctionEnd)
        );

        address++;
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS ResolveBgpFwDisplayBugCheckScreen(UINT64 address, PUINT64 result)
{
    UINT8 Signature[] = {
        0x4c, 0x8b, 0xc3,  // mov r8, rbx
        0x48, 0x8b, 0xd6,  // mov rdx, rsi
        0x41, 0x8b, 0xcf,  // mov ecx, r15d
        0xe8               // call nt!BgpFwDisplayBugCheckScreen
    };

    UINT8 FunctionEnd[] = {
        0x41, 0x5d,        // pop r13
        0x41, 0x5c,        // pop r12
        0x5d,              // pop rbp
        0xc3               // ret
    };

    SIZE_T FunctionEndLength = 0;

    while (FunctionEndLength != sizeof(FunctionEnd)) {
        SIZE_T Length = RtlCompareMemory((PVOID)address, Signature, sizeof(Signature));
        if (Length == sizeof(Signature)) {
            UINT32 Offset = 0;

            RtlCopyMemory(
                &Offset,
                (PVOID)(address + Length),
                sizeof(Offset)
            );

            *result = address + Length + Offset + sizeof(Offset);

            return STATUS_SUCCESS;
        }

        FunctionEndLength = RtlCompareMemory(
            (PVOID)address,
            FunctionEnd,
            sizeof(FunctionEnd)
        );

        address++;
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS ResolveGlobalPhrases(UINT64 address)
{
    UINT8 EtwpLastBranchSig[] = {
        0xbf, 0xc8, 0x01, 0x00, 0x00  // mov edi, 1c8h
    };

    UINT8 SadFaceSig[] = {
		0x41, 0x8b, 0x54, 0xf7, 0x0c, // mov edx, dword ptr [r15 + rsi * 8 + 0ch]
		0x44, 0x8b, 0xcb,             // mov r9d, ebx
        0x48, 0x8d                    // lea rcx, [nt!HalpPciConfigReadHandlers + 0x18]
    };

    UINT8 ColorOffsetSig[] = {
        0xeb, 0x03,                   // jmp nt!BgpFwDisplayBugCheckScreen+0xcd
		0x8b, 0x48, 0x28              // mov ecx, dword ptr [rax + 28h]
    };

    UINT8 MessagesSig[] = {
        0x4c, 0x8d, 0x15              // lea r10, [nt!EtwpLastBranchLookAsideList]
    };

    UINT8 FunctionEnd[] = {
		0x41, 0x5f,                   // pop r15
		0x41, 0x5e,                   // pop r14
		0x41, 0x5d,                   // pop r13
		0x41, 0x5c                    // pop r12
    };

	BOOLEAN IsNotEndOfFunction = TRUE;

    while (IsNotEndOfFunction) {
        SIZE_T EtwpLastBranchLength = RtlCompareMemory(
            (PVOID)(address + 7),
            EtwpLastBranchSig,
            sizeof(EtwpLastBranchSig)
        );

        SIZE_T ColorOffsetLength = RtlCompareMemory(
            (PVOID)(address),
            ColorOffsetSig,
            sizeof(ColorOffsetSig)
        );

        SIZE_T SadFaceLength = RtlCompareMemory(
            (PVOID)(address),
            SadFaceSig,
            sizeof(SadFaceSig)
        );

        SIZE_T MessagesSigLength = RtlCompareMemory(
            (PVOID)(address),
            MessagesSig,
            sizeof(MessagesSig)
        );

        SIZE_T FunctionEndLength = RtlCompareMemory(
            (PVOID)(address),
            FunctionEnd,
            sizeof(FunctionEnd)
        );

        if (FunctionEndLength == sizeof(FunctionEnd)) {
            IsNotEndOfFunction = FALSE;
        } else if (MessagesSigLength == sizeof(MessagesSig)) {
            UINT64 Temp = address;
            UINT32 Offset = 0;

			RtlCopyMemory(
				&Offset,
				(PVOID)(Temp + MessagesSigLength),
				sizeof(g_BsodInformation->BsodMessageOne)
			);

			g_BsodInformation->BsodMessageOne = (PUNICODE_STRING)(Temp + MessagesSigLength + Offset + sizeof(Offset));
			PUNICODE_STRING TempString = g_BsodInformation->BsodMessageOne;

            for (UINT8 next = 0; next < sizeof(UNICODE_STRING); next++, TempString++) {
				if (wcsstr(TempString->Buffer, L"and then we'll restart for you") != 0)
				{
					g_BsodInformation->BsodMessageTwo = TempString;
				}
            }
        }
        else if (EtwpLastBranchLength == sizeof(EtwpLastBranchSig)) {
            UINT64 Temp = address;
            UINT32 Offset = 0;
            
            RtlCopyMemory(
				&g_BsodInformation->Offset,
				(PVOID)(Temp + 0x12),
				sizeof(g_BsodInformation->Offset)
			);

            Temp += 3;

			RtlCopyMemory(
				&Offset,
				(PVOID)(Temp),
				sizeof(Offset)
			);

			Temp += Offset + sizeof(Offset);
			g_BsodInformation->EtwpLastBranchEntry = Temp;
        }
        else if (ColorOffsetLength == sizeof(ColorOffsetSig)) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Addr %08x\n", address);
            RtlCopyMemory(
                &g_BsodInformation->ColorOffset,
                (PVOID)(address + 4),
                sizeof(g_BsodInformation->ColorOffset)
            );
        }
        else if (SadFaceLength == sizeof(SadFaceSig)) {
            UINT64 Temp = address;
            UINT64 Mask = 0xffffffff00000000;
            UINT32 Offset = 0;

            Temp += SadFaceLength + 1;

			RtlCopyMemory(
				&Offset,
				(PVOID)(Temp),
				sizeof(Offset)
			);

            Mask |= Offset;

			Temp += Mask + sizeof(Offset);

			g_BsodInformation->Sadface = Temp;
        }

        address++;
    }

    return STATUS_SUCCESS;
}