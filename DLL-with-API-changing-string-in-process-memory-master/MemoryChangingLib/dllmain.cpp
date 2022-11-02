// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "GUI.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (!libHasBeenInitialized()) {//сделано для того, чтобы было лишь одно окно
        createWindow();//из GUI.h
    }
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH://попытка подзагрузки
        break;
    case DLL_THREAD_ATTACH://подзагрузка
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

