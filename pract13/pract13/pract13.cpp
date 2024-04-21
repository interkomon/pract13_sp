#include "framework.h"
#include "pract13.h"
#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <string>
#include "Resource.h"
#include <windows.h>
#include <commdlg.h>

using namespace std;

#define MAX_LOADSTRING 100
#define ID_MOVE_DIRECTORY_BUTTON 2
#define ID_READ_REGISTRY_LISTBOX 3
#define ID_INSTALLED_PROGRAMS_LISTBOX 104
#define ID_STARTUP_PROGRAMS_LISTBOX 105

HWND hMoveDirectoryButton;
HWND hReadRegistryListBox;
HWND hInstalledProgramsListBox;
HWND hStartupProgramsListBox;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM    MyRegisterClass(HINSTANCE hInstance);
BOOL    InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

class FileManager {
public:
    // Функция для перемещения файла или каталога
    bool MoveFileOrDirectory(const wstring& source, const wstring& destination) {
        if (MoveFileEx(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING) == 0) {
            wcerr << L"Move file error: " << endl;
            return false;
        }
        return true;
    }
};

FileManager fileManager = FileManager();

void ListPrograms(HKEY hKey, const std::wstring& subkey, HWND hwndListBox) {
    HKEY hSubKey;
    LONG result = RegOpenKeyExW(hKey, subkey.c_str(), 0, KEY_READ, &hSubKey);
    if (result != ERROR_SUCCESS) {
        std::wcout << L"Ошибка при открытии ключа реестра: " << subkey << std::endl;
        return;
    }

    DWORD index = 0;
    WCHAR szDisplayName[MAX_PATH];
    DWORD dwDisplayNameSize = ARRAYSIZE(szDisplayName);
    while (RegEnumKeyExW(hSubKey, index, szDisplayName, &dwDisplayNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        HKEY hProgramKey;
        // Открываем подключь каждой программы
        LONG programResult = RegOpenKeyExW(hSubKey, szDisplayName, 0, KEY_READ, &hProgramKey);
        if (programResult == ERROR_SUCCESS) {
            // Получаем значение DisplayName
            WCHAR szProgramName[MAX_PATH];
            DWORD dwProgramNameSize = ARRAYSIZE(szProgramName);
            DWORD dwType;
            programResult = RegQueryValueExW(hProgramKey, L"DisplayName", nullptr, &dwType, reinterpret_cast<LPBYTE>(szProgramName), &dwProgramNameSize);
            if (programResult == ERROR_SUCCESS && dwType == REG_SZ) {
                // Добавляем название программы в ListBox
                SendMessage(hwndListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(szProgramName));
            }
            RegCloseKey(hProgramKey);
        }
        dwDisplayNameSize = ARRAYSIZE(szDisplayName);
        index++;
    }

    RegCloseKey(hSubKey);
}

void AutoRun(HKEY hKey, const std::wstring& subkey, HWND hwndListBox) {
    HKEY hSubKey;
    LONG result = RegOpenKeyExW(hKey, subkey.c_str(), 0, KEY_READ, &hSubKey);
    if (result != ERROR_SUCCESS) {
        std::wcout << L"Ошибка при открытии ключа реестра: " << subkey << std::endl;
        return;
    }

    DWORD index = 0;
    WCHAR szValueName[1024];
    DWORD dwValueNameSize = ARRAYSIZE(szValueName);
    DWORD dwType;
    BYTE lpData[1024];
    DWORD dwDataSize = sizeof(lpData);

    while (RegEnumValueW(hSubKey, index, szValueName, &dwValueNameSize, nullptr, &dwType, lpData, &dwDataSize) == ERROR_SUCCESS) {
        // Проверяем, что значение - строка
        if (dwType == REG_SZ) {
            // Добавляем строку в ListBox
            SendMessage(hwndListBox, LB_ADDSTRING, 0, (LPARAM)szValueName);
        }

        // Сбрасываем размеры для следующей итерации
        dwValueNameSize = ARRAYSIZE(szValueName);
        dwDataSize = sizeof(lpData);
        index++;
    }

    RegCloseKey(hSubKey);
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR  lpCmdLine,
    _In_ int    nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PRACT13, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PRACT13));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_PRACT13));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PRACT13);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);


    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void AddWinWidgets(HWND hWnd) {
    hMoveDirectoryButton = CreateWindowEx(0, _T("BUTTON"), _T("Move Directory"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 0, 200, 30, hWnd, (HMENU)ID_MOVE_DIRECTORY_BUTTON, NULL, NULL);

    hInstalledProgramsListBox = CreateWindowEx(WS_EX_CLIENTEDGE, _T("LISTBOX"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        10, 40, 500, 500, hWnd, (HMENU)ID_INSTALLED_PROGRAMS_LISTBOX, NULL, NULL);

    hStartupProgramsListBox = CreateWindowEx(WS_EX_CLIENTEDGE, _T("LISTBOX"), _T(""),
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
        520, 40, 500, 500, hWnd, (HMENU)ID_STARTUP_PROGRAMS_LISTBOX, NULL, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   // wstring source = L"C:\\Users\\Andrey\\Desktop\\Тестирование_Отчет"; // Исходный каталог
   // wstring destination = L"C:\\Users\\Andrey\\Desktop\\Вариант 9"; // Целевой каталог

    static OPENFILENAME ofn;       // common dialog box structure
    static wchar_t szFile[260];    // buffer for file name
    wstring source;                // Исходный каталог
    wstring destination;           // Целевой каталог


    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_MOVE_DIRECTORY_BUTTON:
            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            // Display the Open dialog box.
            if (GetOpenFileName(&ofn) == TRUE) {
                source = ofn.lpstrFile;
                // Display the Save dialog box.
                if (GetSaveFileName(&ofn) == TRUE) {
                    destination = ofn.lpstrFile;
                    if (!fileManager.MoveFileOrDirectory(source, destination)) {
                        MessageBoxA(0, "Ошибка перемещения каталога файла или каталога", "Error", MB_OK);
                    }
                    else {
                        MessageBoxA(0, "Операции выполнены успешно.", "Information", MB_OK);
                    }
                }
            }
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE:

        AddWinWidgets(hWnd);
        ListPrograms(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", hInstalledProgramsListBox);

        AutoRun(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", hStartupProgramsListBox);
        
        
        
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
