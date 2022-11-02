#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <utility>
#include <limits.h>
#include "MemChanger.h"

int fMatchCheck(char* mainstr, int mainstrLen, char* checkstr, int checkstrLen) {//�������� �� ���������� ������� ������ ������������, ���� ������� �� ������
    /*
    �������� ������� ��������� � ������.
    ��� ���� ��� "�������" ���������������
    ������ ������������������ ����.
    */
    BOOL fmcret = TRUE;
    int x, y;//�-����� ������� � mainStr, �-����� ������� � checkStr

    for (x = 0; x < mainstrLen; x++) {
        fmcret = TRUE;

        for (y = 0; y < checkstrLen; y++) {
            if (checkstr[y] != mainstr[x + y]) {
                fmcret = FALSE;
                break;
            }
        }

        if (fmcret)
            return x + checkstrLen;//���������� ����� ���������
    }
    return -1;
}

char* getMem(char* buff, size_t buffLen, int from, int to) {//������� ����� ������, ��� ��, ��� �� �������
    /*
    �������� � ���� ������, � ������� ����� �������
    ����� ������ �� ������ ����� ���������.
    */
    size_t ourSize = buffLen * 2;
    char* ret = (char*)malloc(ourSize);//��������� ������

    if (!ret) return ret;

    memset(ret, 0, ourSize);//0 ���������

    memcpy(ret, &buff[from], buffLen - from);//�� ������ � �������
    memset(&ret[to - from], 0, to - from);//0 � �����

    return ret;
}

char* delMem(char* buff, size_t buffLen, int from, int to) {//������� ��������� ������
    /*
    ����������� ������.
    */
    size_t ourSize = buffLen * 2;
    char* ret = (char*)malloc(ourSize);
    int i, x = 0;
    if (!ret) return ret;

    memset(ret, 0, ourSize);

    for (i = 0; i < buffLen; i++) {
        if (!(i >= from && i < to)) {
            ret[x] = buff[i];
            x++;
        }
    }

    return ret;
}

char* addMem(char* buff, size_t buffLen, char* buffToAdd, size_t addLen, int addFrom) {//������ �� getMem
    /*
    ������ � ������.
    */
    size_t ourSize = (buffLen + addLen) * 2;
    char* ret = (char*)malloc(ourSize);
    int i, x = 0;

    if (!ret) return ret;

    memset(ret, 0, ourSize);

    memcpy(ret, getMem(buff, buffLen, 0, addFrom), addFrom);

    x = 0;
    for (i = addFrom; i < addLen + addFrom; i++) {
        ret[i] = buffToAdd[x];
        x++;
    }

    x = 0;
    for (i; i < addFrom + buffLen; i++) {
        ret[i] = buff[addFrom + x];
        x++;
    }

    return ret;
}

char* replaceMem(char* buff, size_t buffLen, int from, int to, char* replaceBuff, size_t replaceLen) {//���� ������
    /*
    �������� ��������� "������" �� ����.
    */
    size_t ourSize = (buffLen) * 2;
    char* ret = (char*)malloc(ourSize);

    if (!ret) return ret;

    memset(ret, 0, ourSize);

    memcpy(ret, buff, buffLen); // copy 'buff' into 'ret'

    ret = delMem(ret, buffLen, from, to); // delete all memory from 'ret' betwen 'from' and 'to'
    ret = addMem(ret, buffLen - to + from, replaceBuff, replaceLen, from);

    return ret;
}

BOOL DoRtlAdjustPrivilege() {//������ ���������� ��������(2 �������), �� ������� 3 �������
    /*
    ������ �������. �������� ���������� ���������.
    ������ ��� �������� ��� �������� ������ ����������
    � ����������� ������.
    */
#define SE_DEBUG_PRIVILEGE 20L
#define AdjustCurrentProcess 0
    BOOL bPrev = FALSE;
    LONG(WINAPI * RtlAdjustPrivilege)(DWORD, BOOL, INT, PBOOL);
    *(FARPROC*)&RtlAdjustPrivilege = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlAdjustPrivilege");
    if (!RtlAdjustPrivilege) return FALSE;
    RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE, TRUE, AdjustCurrentProcess, &bPrev);
    return TRUE;
}

int replaceStringInProcessMemory(long pid, char* lpOrig, char* lpReplacement, int lpOrigLength) {//�������
    /*** VARIABLES ***/
    HANDLE hProc;//����� ��������

    MEMORY_BASIC_INFORMATION mbi;//���������� �� ������� ������ ��������
    SYSTEM_INFO msi;//���������� � �������
    ZeroMemory(&mbi, sizeof(mbi));//���������� ������ � ������, ������������
    GetSystemInfo(&msi);//�������� ����������, � ������� �������
    /*
    �������� ���������� � ������ � ������� �������.
    */

    DWORD dwRead = 0;//��������� 0

    char* lpData = (char*)GlobalAlloc(GMEM_FIXED, MAX_READ);//���������� ������� ������ ��� ������ (����� �����)

    int x, at;//at-������� � ������, �-������� � �������� dwRead
    /*****************/

    if (!lpData)
        return -1;

    ZeroMemory(lpData, MAX_READ);

    // ��������� �������
    hProc = OpenProcess(PROCESS_ALL_ACCESS,
        FALSE, pid);
    if (!hProc) {
        //Cant open process!
        return -1;
    }

    if (DoRtlAdjustPrivilege()) {
        /*
        ���������� ��������� ��� ������ � �������.
        */

        //Process opened sucessfully. Scanning memory

        for (LPBYTE lpAddress = (LPBYTE)msi.lpMinimumApplicationAddress; lpAddress <= (LPBYTE)msi.lpMaximumApplicationAddress; lpAddress += mbi.RegionSize) {
            /*
            ���� ���� �������� ��� ��� �� ��, ��� ���� ��������� �� ��������
            ������ ��������. ������ � Windows � �������� ������� �� "�������".
            � ������� ������� ���� ������� �������: � ������-�� ������ ��������,
            �����-�� ����� ������ ���������. ��� ����� ������� ��������� ��� ������.
            ��� �������� � ���� �������� ������ ������ �� ������ � �������� ������
            ������ � ������. ������ ��� �������� ArtMoney.
            */

            if (VirtualQueryEx(hProc, lpAddress, &mbi, sizeof(mbi))) {//��������� ����������
                /*
                ������ � ������� ������� ������.
                */

                if ((mbi.Protect & PAGE_READWRITE) || (mbi.Protect & PAGE_WRITECOPY)) {
                    /*
                    ���� �� �������� ��� ������, �������� � ���.
                    */

                    for (lpAddress; lpAddress < (lpAddress + mbi.RegionSize); lpAddress += MAX_READ) {
                        /*
                        �������� �� ������� ���������� � ������ ������ �������� �� ������, �� ����� �������
                        � ���������, �� � ��� �� ������ ������.
                        */

                        dwRead = 0;
                        if (ReadProcessMemory(hProc,
                            (LPCVOID)lpAddress,
                            lpData,
                            MAX_READ, &
                            dwRead) == TRUE) {
                            /*
                            ������ �� 128 ���� �� ������ ������ �������� �� ������
                            � ���������, �� � ��� �� ������ ������.
                            */

                            if (fMatchCheck(lpData, dwRead, lpOrig, lpOrigLength) != -1) {
                                /*�����, ������� �� ������ � �������� � ����� �������� ������� ������ �� ����.*/

                                at = fMatchCheck(lpData,//�������
                                    dwRead,
                                    lpOrig,
                                    lpOrigLength);

                                if (at != -1) {
                                    at -= lpOrigLength;

                                    lpData = replaceMem(lpData,
                                        dwRead,
                                        at,
                                        at + lpOrigLength,
                                        lpReplacement,
                                        /*sizeof(lpReplacement)-1*/
                                        lpOrigLength);

                                    if (!WriteProcessMemory(hProc,
                                        (LPVOID)lpAddress,
                                        lpData,
                                        /*dwRead-sizeof(lpOrig)-1+sizeof(lpReplacement)-1*/
                                        dwRead, &
                                        dwRead)) return -1;
                                }
                                if (hProc)
                                    CloseHandle(hProc);
                                return 0;
                            }
                            else {
                                //puts("Error: not found in: ");
                                //puts(lpData);
                                //puts("\n");
                            }

                        }
                    }

                }
            }
        }
    }

    // // // // //
    if (hProc)
        CloseHandle(hProc);
    /*if (lpData)
        GlobalFree(lpData);*/
        ///////////////
    return 1;
}