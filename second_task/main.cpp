#include <windows.h>
#include <iostream>

using namespace std;

BOOL LoadPeFile(LPCWSTR FilePath, PUCHAR* ppImageBase) {
    HANDLE hFile = CreateFile(FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        cout << "ERROR: LoadPeFile: CreateFile fails with " << GetLastError() << " error \n";
        return FALSE;
    }

    HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY | SEC_IMAGE_NO_EXECUTE, 0, 0, NULL);
    if (hFileMapping == NULL) {
        cout << "ERROR: LoadPeFile: CreateFileMapping fails with " << GetLastError() << " error \n";
        CloseHandle(hFile);
        return FALSE;
    }

    LPVOID p = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (p == NULL) {
        cout << "ERROR: LoadPeFile: MapViewOfFile fails with " << GetLastError() << " error \n";
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return FALSE;
    }

    *ppImageBase = (PUCHAR)p;

    CloseHandle(hFileMapping);
    CloseHandle(hFile);
    return TRUE;
}

LPCWSTR g_FilePath = L"\\?\\C:\\PROJECTS\\Autumn-2024\\119-MIPT\\FirstLab\\x64\\Release\\Hello.exe";

#define TO_PSTRUCT(TYPE, offset) (TYPE)(pImageBase + (offset)) // RVA
#define VAR_OF_PSTRUCT(var, TYPE, offset) TYPE var = TO_PSTRUCT(TYPE, offset)

int wmain(int argc, wchar_t* argv[]) {
#if 0
    if (argc != 2) {
        cout << "Usage: SeconLab PeFilePath \n";
        return -1;
    }

    LPCWSTR g_FilePath = argv[1];
#endif
    PUCHAR pImageBase = nullptr;
    if (!LoadPeFile(g_FilePath, &pImageBase)) return -1;

    cout << "MS-DOS Signature: " << pImageBase[0] << pImageBase[1] << endl;
    // TODO check

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;
    VAR_OF_PSTRUCT(pTempPeHeader, PIMAGE_NT_HEADERS, pDosHeader->e_lfanew); // offset to PE Header

    PUCHAR p = (PUCHAR)(&pTempPeHeader->Signature); // TODO macro or template to read byte/word/dword from pointer
    cout << "PE Signature: " << p[0] << p[1] << " " << hex << (int)p[2] << (int)p[3] << endl;

    WORD nSections = pTempPeHeader->FileHeader.NumberOfSections;
    cout << "PE Sections total " << nSections << endl;
    PIMAGE_SECTION_HEADER pSectionHeader = nullptr;

    switch (pTempPeHeader->FileHeader.Machine) {
    case IMAGE_FILE_MACHINE_I386:
        cout << "PE Architecture: x86 \n";
        pSectionHeader = (PIMAGE_SECTION_HEADER)(((PUCHAR)pTempPeHeader) + sizeof(IMAGE_NT_HEADERS32));
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        cout << "PE Architecture: x64 \n";
        pSectionHeader = (PIMAGE_SECTION_HEADER)(((PUCHAR)pTempPeHeader) + sizeof(IMAGE_NT_HEADERS64));
        break;
    default:
        cout << "PE Architecture: unknown \n";
        return -1;
    }

    CHAR nmSection[9] = { 0 };
    for (int i = 0; i < nSections; i++) {
        memcpy(nmSection, pSectionHeader->Name, 8);
        cout << "section #" << i << " " << nmSection << endl;
        pSectionHeader++;
    }
    return 0;
}
