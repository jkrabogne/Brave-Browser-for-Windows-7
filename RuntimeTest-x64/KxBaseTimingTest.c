#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

typedef VOID (WINAPI *QUERY_INTERRUPT_TIME_PRECISE)(
    PULONGLONG
);

static int Win32Failure(const char *Operation)
{
    DWORD Error = GetLastError();

    printf("FAIL: %s\n", Operation);
    printf("Win32 error: %lu (0x%08lX)\n", Error, Error);

    return 1;
}

int wmain(int argc, wchar_t **argv)
{
    WCHAR KexPath[MAX_PATH];
    WCHAR KxBasePath[MAX_PATH];
    WCHAR LoadedPath[MAX_PATH];

    UINT Length;
    DWORD PathLength;

    HMODULE KexModule;
    HMODULE KxBaseModule;

    FARPROC ByName;
    FARPROC ByOrdinal;

    QUERY_INTERRUPT_TIME_PRECISE QueryTime;

    ULONGLONG First;
    ULONGLONG Current;
    ULONGLONG Previous;
    ULONGLONG MinimumDelta;
    ULONGLONG Delta;

    ULONGLONG Before;
    ULONGLONG After;
    ULONGLONG TickBefore;
    ULONGLONG TickAfter;

    ULONGLONG ApiElapsed;
    ULONGLONG ReferenceElapsed;
    ULONGLONG Difference;

    ULONG i;
    ULONG BackwardCount = 0;
    ULONG PositiveCount = 0;
    ULONG EqualCount = 0;

    setvbuf(stdout, NULL, _IONBF, 0);

    printf("=== VXKEX TIMING API RUNTIME TEST ===\n");
    printf("Process architecture: %u-bit\n",
           (unsigned int)(sizeof(void *) * 8));

    if (argc != 2) {
        printf("Usage: KxBaseTimingTest.exe <KxBase.dll>\n");
        return 2;
    }

    if (sizeof(void *) != 8) {
        printf("FAIL: Expected x64 process.\n");
        return 3;
    }

    if (GetModuleHandleW(L"KexDll.dll") != NULL ||
        GetModuleHandleW(L"KxBase.dll") != NULL) {

        printf("FAIL: VxKex modules already loaded.\n");
        printf("Run this harness without VxKex enabled.\n");
        return 4;
    }

    printf("\n=== LOCATE INSTALLED KEXDLL ===\n");

    Length = GetSystemDirectoryW(
        KexPath,
        MAX_PATH
    );

    if (Length == 0 || Length >= MAX_PATH) {
        return Win32Failure("GetSystemDirectoryW");
    }

    if (wcscat_s(
            KexPath,
            MAX_PATH,
            L"\\KexDll.dll") != 0) {

        printf("FAIL: KexDll path too long.\n");
        return 5;
    }

    printf("KexDll requested path: %ls\n", KexPath);

    printf("\n=== LOAD INSTALLED KEXDLL ===\n");

    KexModule = LoadLibraryW(KexPath);

    if (KexModule == NULL) {
        return Win32Failure("LoadLibraryW(KexDll.dll)");
    }

    PathLength = GetModuleFileNameW(
        KexModule,
        LoadedPath,
        MAX_PATH
    );

    if (PathLength == 0 || PathLength >= MAX_PATH) {
        return Win32Failure("GetModuleFileNameW(KexDll)");
    }

    printf("Loaded KexDll: %ls\n", LoadedPath);

    if (_wcsicmp(KexPath, LoadedPath) != 0) {
        printf("FAIL: Unexpected KexDll location.\n");
        return 6;
    }

    printf("KexDll initialization: PASSED\n");

    printf("\n=== LOAD ISOLATED CUSTOM KXBASE ===\n");

    PathLength = GetFullPathNameW(
        argv[1],
        MAX_PATH,
        KxBasePath,
        NULL
    );

    if (PathLength == 0 || PathLength >= MAX_PATH) {
        return Win32Failure("GetFullPathNameW");
    }

    printf("Requested KxBase: %ls\n", KxBasePath);

    KxBaseModule = LoadLibraryW(KxBasePath);

    if (KxBaseModule == NULL) {
        return Win32Failure("LoadLibraryW(KxBase.dll)");
    }

    PathLength = GetModuleFileNameW(
        KxBaseModule,
        LoadedPath,
        MAX_PATH
    );

    if (PathLength == 0 || PathLength >= MAX_PATH) {
        return Win32Failure("GetModuleFileNameW(KxBase)");
    }

    printf("Loaded KxBase: %ls\n", LoadedPath);

    if (_wcsicmp(KxBasePath, LoadedPath) != 0) {
        printf("FAIL: Unexpected KxBase location.\n");
        return 7;
    }

    printf("KxBase initialization: PASSED\n");

    printf("\n=== RESOLVE CUSTOM API ===\n");

    ByName = GetProcAddress(
        KxBaseModule,
        "QueryInterruptTimePrecise"
    );

    if (ByName == NULL) {
        return Win32Failure("GetProcAddress by name");
    }

    ByOrdinal = GetProcAddress(
        KxBaseModule,
        (LPCSTR)(ULONG_PTR)1943
    );

    if (ByOrdinal == NULL) {
        return Win32Failure("GetProcAddress by ordinal");
    }

    if (ByName != ByOrdinal) {
        printf("FAIL: Name and ordinal resolve differently.\n");
        return 8;
    }

    printf("Name resolution: PASSED\n");
    printf("Ordinal 1943 resolution: PASSED\n");
    printf("Function addresses match: YES\n");

    QueryTime = (QUERY_INTERRUPT_TIME_PRECISE)ByName;

    printf("\n=== INITIAL OUTPUT TEST ===\n");

    First = ~0ULL;

    QueryTime(&First);

    printf("First value: %I64u (100 ns units)\n", First);
    printf("Approximate uptime: %I64u ms\n", First / 10000ULL);

    if (First == 0 || First == ~0ULL) {
        printf("FAIL: Invalid initial output.\n");
        return 9;
    }

    printf("\n=== MONOTONICITY AND RESOLUTION TEST ===\n");

    Previous = First;
    MinimumDelta = ~0ULL;

    for (i = 0; i < 100000; i++) {

        Current = ~0ULL;
        QueryTime(&Current);

        if (Current == ~0ULL) {
            printf("FAIL: Output was not written.\n");
            return 10;
        }

        if (Current < Previous) {
            BackwardCount++;
        } else if (Current == Previous) {
            EqualCount++;
        } else {

            Delta = Current - Previous;
            PositiveCount++;

            if (Delta < MinimumDelta) {
                MinimumDelta = Delta;
            }
        }

        Previous = Current;
    }

    for (i = 0; i < 100; i++) {

        Sleep(1);

        QueryTime(&Current);

        if (Current < Previous) {
            BackwardCount++;
        } else if (Current == Previous) {
            EqualCount++;
        } else {

            Delta = Current - Previous;
            PositiveCount++;

            if (Delta < MinimumDelta) {
                MinimumDelta = Delta;
            }
        }

        Previous = Current;
    }

    printf("Total samples: 100100\n");
    printf("Backward samples: %lu\n", BackwardCount);
    printf("Equal samples: %lu\n", EqualCount);
    printf("Positive increments: %lu\n", PositiveCount);

    if (MinimumDelta != ~0ULL) {

        printf("Smallest observed increment: %I64u\n",
               MinimumDelta);

        printf("Observed increment in ms: %.4f\n",
               (double)MinimumDelta / 10000.0);
    }

    if (BackwardCount != 0 || PositiveCount == 0) {
        printf("FAIL: Monotonicity or progress test.\n");
        return 11;
    }

    printf("Monotonicity test: PASSED\n");

    printf("\n=== ELAPSED TIME COMPARISON ===\n");

    TickBefore = GetTickCount64();
    QueryTime(&Before);

    Sleep(2000);

    QueryTime(&After);
    TickAfter = GetTickCount64();

    if (After < Before || TickAfter < TickBefore) {
        printf("FAIL: Unexpected backward time.\n");
        return 12;
    }

    ApiElapsed = After - Before;
    ReferenceElapsed = (TickAfter - TickBefore) * 10000ULL;

    if (ApiElapsed >= ReferenceElapsed) {
        Difference = ApiElapsed - ReferenceElapsed;
    } else {
        Difference = ReferenceElapsed - ApiElapsed;
    }

    printf("Custom API elapsed: %.4f ms\n",
           (double)ApiElapsed / 10000.0);

    printf("GetTickCount64 elapsed: %.4f ms\n",
           (double)ReferenceElapsed / 10000.0);

    printf("Absolute difference: %.4f ms\n",
           (double)Difference / 10000.0);

    if (Difference > 1000000ULL) {
        printf("FAIL: Elapsed difference exceeds 100 ms.\n");
        return 13;
    }

    printf("Elapsed time comparison: PASSED\n");

    printf("\n=== RUNTIME TEST PASSED ===\n");

    /*
     * Leave both DLLs loaded until normal process exit.
     * Avoid exercising explicit unload behavior in this test.
     */

    return 0;
}
