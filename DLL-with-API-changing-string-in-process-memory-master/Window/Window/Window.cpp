// Window.cpp : Определяет точку входа для приложения.
//

#include "framework.h"//
#include "Window.h"//dll для WinApi

#include <windowsx.h>//работа с dll
#include <tlhelp32.h>//работа с памятью
#include <tchar.h>//работа со строками и charами
#include <stdarg.h>//

//#define DYNAMIC_DLL_IMPORT//динамическая подгрузка библиотеки

#ifndef DYNAMIC_DLL_IMPORT//если отключено динамическое
#include "D:\\DLL-with-API-changing-string-in-process-memory-master\\MemoryChangingLib\\MemChanger.h"//хранит самму функцию работы с памятью
#include "D:\DLL-with-API-changing-string-in-process-memory-master\MemoryChangingLib\\GUI.h"//вывод графического интерфейса
#endif

#define MAX_LOADSTRING 2//хз

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

#define DLL_NAME "D:\\MemoryChangingLib.dll"//полное имя с адресом библиотеки

#define IDM_GO 9275//код кнопки GO
#define IDM_INJECT 9276//код кнопки INJECT

HWND hWnd;//хэндлер окна

HWND hButGo;//хэндлеры кнопок
HWND hButInject;//
HWND pidNo;//хэндлер строки для ввода

int k;//?
unsigned long searchPid;//pid в который будем inject
//Локальные функции:
int injectDLLIntoProcess(unsigned long pid);//инъекция библиотеки
void setWindowElements(HWND hWnd);//установка элементов окна
int getInputStrings();//установка значения searchPid
int getWStrigFromWindow(HWND window, std::wstring* str);//получение строки из элемента окна и записываем в str
WCHAR* charToWchar(const char* str);//преобразует массив char в массив WCHAR
char* WideStringToAnsi(std::wstring& Str, unsigned int CodePage = CP_ACP);//преобразование строки полученной из окна в массив char
unsigned long WideStringToULong(std::wstring& Str, unsigned int CodePage);//преобразование строки в long
int GetStrLength(char* str);//получение длины строки

WCHAR* NumToText(DWORD pid) {//принимает в себя long и преобразует в строку
	int pidLength;
	unsigned long inversedPid = 0;
	for (pidLength = 0; pid; pidLength++) {
		inversedPid *= 10;
		inversedPid += pid % 10;
		pid /= 10;
	}
	WCHAR* res = new WCHAR[pidLength + 1]; int i;
	for (i = 0; inversedPid; i++) {
		res[i] = '0' + (inversedPid % 10);
		inversedPid /= 10;
	}
	for (; i < pidLength; i++) {
		res[i] = '0';
	}
	res[pidLength] = '\0';
	return res;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hDC;
	switch (uMsg) {
		case WM_CREATE:
			setWindowElements(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_COMMAND:
		{
			// Разобрать выбор:
			switch (LOWORD(wParam))
			{
				case IDM_GO://
					{
#ifndef DYNAMIC_DLL_IMPORT
						createWindow();//если не динамически, то мы создаем окно
#endif
#ifdef DYNAMIC_DLL_IMPORT
						LoadLibrary(L"D:\\MemoryChangingLib.dll");//если динамически, то загружаем библиотеку через LoadLibrary
#endif
						break;
					}
				case IDM_INJECT:
					{
						try {
							getInputStrings();//получаем наш pid из окна
							if (!injectDLLIntoProcess(searchPid)) {//пробуем заинжектить
								
								MessageBox(hWnd, L"Can not inject DLL", NumToText(GetLastError()) , MB_ICONERROR);//если вылезла ошибка
							}
						}
						catch (...) {
							MessageBox(nullptr, L"Injecting error!", L"Error", MB_ICONERROR);//ошибка в инжектировании
						}
						break;
					}
				default:
					return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR  lpCmdLine, _In_ int  nCmdShow) {
	hInst = hInstance;
	WNDCLASSEX wcex;
	MSG msg;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MyWindowClass";
	wcex.hIconSm = 0;
	RegisterClassEx(&wcex);

	hWnd = CreateWindowEx(0, L"MyWindowClass", L"Memory Rewriter", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 400, 180, NULL, NULL, hInstance, NULL);

	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;

	return 0;
}

void setWindowElements(HWND hWnd) {//элементы окна
	hButGo = CreateWindow(
		L"BUTTON",//тип
		L"Go",//текст
		WS_CHILD | WS_BORDER | WS_VISIBLE | BS_PUSHBUTTON,//параметры (дочернее, с границей, видимое, тип кнопки)
		90, 100, 100, 30, hWnd, reinterpret_cast<HMENU>(IDM_GO), nullptr, nullptr//(координаты, размеры, окно, 
	);

	hButInject = CreateWindow(
		L"BUTTON",
		L"Inject",
		WS_CHILD | WS_BORDER | WS_VISIBLE | BS_PUSHBUTTON,
		190, 100, 100, 30, hWnd, reinterpret_cast<HMENU>(IDM_INJECT), nullptr, nullptr
	);

	pidNo = CreateWindow(
		L"EDIT",
		L"0",
		WS_CHILD | WS_BORDER | WS_VISIBLE | ES_AUTOHSCROLL,
		10, 0, 360, 20, hWnd, reinterpret_cast<HMENU>(1), nullptr, nullptr
	);
}

int injectDLLIntoProcess(unsigned long pid) {//принимает long
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);//получение хэндлера процесса с правами PROCESS_ALL_ACCESS
	if (process == NULL) return false;//не удалось открыть

	// "Вытягивание" функции из системной библиотеки для динамической  
	// подгрузки DLL в адресное пространство открытого процесса
	LPVOID fp = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");//LoadLibraryA подгружается только динамически, используется
	// для динамического импорта, она загружается и в этот проект и туда, куда мы инжектим
	if (fp == NULL) return false;//не удалось загрузить из kernel32

	// Выделение участка памяти размером strlen(_dll_name) для последующей 
	// записи имени библеотеки в память процесса.
	LPVOID alloc = (LPVOID)VirtualAllocEx(process, 0, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);//выделение памяти в начале, туда мы
	//загружаем ссылку на поток, который выполняет замену
	if (alloc == NULL) return false;

	// Запись имени инжектируемой DLL в память
	BOOL w = WriteProcessMemory(process, (LPVOID)alloc, DLL_NAME, strlen(DLL_NAME), 0);//запись самого имени
	if (w == NULL) return false;

	// Создание "удаленного" потока в адресном пространстве
	// открытого процесса и последующая подгрузка нашей DLL.
	HANDLE thread = CreateRemoteThread(process, 0, 0, (LPTHREAD_START_ROUTINE)fp, (LPVOID)alloc, 0, 0);//запуск dll
	if (thread == NULL) {
		MessageBox(hWnd, L"Can not inject DLL", NumToText(GetLastError()), MB_ICONERROR);//чаще всего вылетает (защита может стоять)
		return false;
	}

	CloseHandle(process);
	return true;
}

int getInputStrings() {
	std::wstring pid{};


	if (getWStrigFromWindow(pidNo, &pid)) return -1;//получаем строку из окна и записываем, если без ошибок

	searchPid = WideStringToULong(pid, 0);
}

int getWStrigFromWindow(HWND window, std::wstring* str) {
	str->resize(MAX_PATH);//расширяем строку до макс длины, если выход за нее, то обрезаем
	GetWindowText(window, &(*str)[0], 20);//указатель на 0-символ
	str->erase(remove(begin(*str), end(*str), 0), end(*str));//перемещаем 0-символ в реальный конец

	if (str->empty()) {
		MessageBox(hWnd, L"Input missed!", L"Err", MB_ICONINFORMATION);//пустой ввод
		return -1;
	}
}

unsigned long WideStringToULong(std::wstring& Str, unsigned int CodePage) {
	char* tempAnsiStr = WideStringToAnsi(Str, CodePage);//получили массив чаров

	unsigned long res = 0;
	for (int i = 0; tempAnsiStr[i] != '\0'; i++) {
		res *= 10;
		res += tempAnsiStr[i] - '0';
	}
	return res;//возвращаем лонг
}

char* WideStringToAnsi(std::wstring& Str, unsigned int CodePage)//похуй
{
	DWORD BuffSize = WideCharToMultiByte(CodePage, 0, Str.c_str(), -1, NULL, 0, NULL, NULL);
	if (!BuffSize)
		return NULL;
	char* Buffer = new char[BuffSize];

	if (!WideCharToMultiByte(CodePage, 0, Str.c_str(), -1, Buffer, BuffSize, NULL, NULL))
		return NULL;
	return (Buffer);
}