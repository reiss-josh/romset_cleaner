// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <regex>
#include<map>

#define ROMCACHE ::std::map<std::string,std::list<std::string>>
#define STRLIST ::std::list<std::string>
#define PATHLIST ::std::list<fs::path>
namespace fs = std::filesystem;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Windows Desktop Guided Tour Application");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void bigWorker(fs::path sourcePath, fs::path destinationPath, STRLIST countryCodes, STRLIST exclusionStrings, STRLIST fileExtensions, bool exclusiveRegions, STRLIST versionCodes);

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindow explained:
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    TCHAR title[] = _T("Hello, Windows desktop!");
    TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message)
    {
        case WM_CREATE:
        {
            CreateWindow(TEXT("button"), TEXT("Show Title"),
            WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            20, 20, 185, 35,
            hWnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            CheckDlgButton(hWnd, 1, BST_CHECKED);

            CreateWindow(L"BUTTON", L"OK", // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                10,         // x position 
                100,         // y position 
                50,        // Button width
                25,        // Button height
                hWnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            break;
        }
        case WM_COMMAND:
        {
            bool checked = IsDlgButtonChecked(hWnd, 1);
            switch (wParam)
            {
                case 1:
                    OutputDebugString(L"bluh");
                    
                    if (checked) {
                        CheckDlgButton(hWnd, 1, BST_UNCHECKED);
                        SetWindowText(hWnd, TEXT(""));
                    }
                    else {
                        CheckDlgButton(hWnd, 1, BST_CHECKED);
                        SetWindowText(hWnd, title);
                    }
                    break;
                case 2:
                    OutputDebugString(L"bloh");
                    std::string srcPath = "D:/Downloads/n64romsets/nointro/testing";
                    std::string destPath = "D:/Downloads/n64romsets/nointro/testing/cleaned";
                    STRLIST countryHierarchy = { "USA", "Japan" };
                    STRLIST excludedStrings = { "(Beta)", "(Prototype)" };
                    STRLIST fileExts = { ".z64" };
                    bool exclusiveRegions = true;
                    STRLIST versionCodes = {"Rev", "ver"};
                    bigWorker(srcPath, destPath, countryHierarchy, excludedStrings, fileExts, exclusiveRegions, versionCodes);
                    break;
            }
            break;
        }
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);

            // Here your application is laid out.
            // For this introduction, we just print out "Hello, Windows desktop!"
            // in the top left corner.
            TextOut(hdc,
                5, 5,
                greeting, _tcslen(greeting));
            // End application-specific layout section.

            EndPaint(hWnd, &ps);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }

    return 0;
}

template<typename K, typename V>
void print_map(std::map<K, V> const& m, fs::path filename) {
    std::ofstream outfile;
    outfile.open(filename);
    for (auto const& pair : m) {
        outfile << pair.first << " {\n";
        for (auto const& i : pair.second) {
            outfile << "\t" << i << "\n";
        }
        outfile << "}\n";
    }
    outfile.close();
}

template<typename T>
void print_list(std::list<T> const& m, fs::path filename) {
    std::ofstream outfile;
    outfile.open(filename);
    for (auto const& i : m) {
        outfile << i << "\n";
    }
}

std::string StrlistDelimiter(STRLIST lst, std::string delimiter, std::string upFront = "", std::string outBack = "") {
    std::string lstSum = upFront;
    for (auto const& i : lst) {
        lstSum += i;
        lstSum += delimiter;
    }
    lstSum.erase(lstSum.length()-delimiter.length());
    lstSum += outBack;
    return lstSum;
}

std::string RemoveRegionAndLang(std::string inputString, STRLIST versionCodes = {}) {
    std::regex cty(" \\([^)]*\\)"); //remove countries
    std::regex lng(" \\(En,[^)]*\\)"); //remove langs
    std::string result =  std::regex_replace(std::regex_replace(inputString, cty, "", std::regex_constants::format_first_only), lng, "");
    if (versionCodes.empty()) return result;
    else {
        std::string verSum = StrlistDelimiter(versionCodes, "[^)]*\\)| \\(", " \\(", "[^)]*\\)");
        std::regex rev(verSum);
        return std::regex_replace(result, rev, "");
    }
}

bool MatchExtensions(std::string input, STRLIST fileExtensions) {
    std::string extSum = StrlistDelimiter(fileExtensions, "|");
    std::regex ext(extSum);
    return std::regex_search(input, ext);
}

fs::path SelectBestVersion(STRLIST versions, STRLIST countryCodes, bool excludeRegion) {
    return versions.front();
}

ROMCACHE GenerateCache(fs::path source, STRLIST fileExtensions, STRLIST versionCodes = {}) {
    ROMCACHE cache;
    for (const auto& entry : fs::directory_iterator(source)) {
        std::string filename = entry.path().u8string();
        if (MatchExtensions(filename, fileExtensions)) {
            std::string noRegion = RemoveRegionAndLang(filename, versionCodes);
            auto search = cache.find(noRegion);
            if (search == cache.end()) {
                cache.insert({ noRegion, {} });
                search = cache.find(noRegion);
            }
            search->second.push_back(filename);
        }
    }
    return cache;
}

PATHLIST GetList(ROMCACHE cache, STRLIST countryCodes, bool excludeRegion, STRLIST versionCodes = {}) {
    PATHLIST lst = {};
    for (auto const& pair : cache) {
        fs::path best = SelectBestVersion(pair.second, countryCodes, excludeRegion);
        if (best != "N/A") {
            lst.push_back(best);
        }
    }
    return lst;
}

void MoveRoms(PATHLIST listOfChosenRoms, fs::path src, fs::path dst) {

}

void bigWorker(fs::path sourcePath, fs::path destinationPath, STRLIST countryCodes, STRLIST exclusionStrings, STRLIST fileExtensions, bool exclusiveRegions, STRLIST versionCodes = {}) {
    
    if (fs::exists(sourcePath)) OutputDebugString(L"exists!");
    else OutputDebugString(L"source invalid!");

    if (!fs::exists(destinationPath)) {
        OutputDebugString(L"creating dest path!");
        fs::create_directory(destinationPath);
    }

    fs::path outputPath = "D:/Downloads/n64romsets/nointro/testing/textfiles";
    if (!fs::exists(outputPath)) {
        OutputDebugString(L"creating output path!");
        fs::create_directory(outputPath);
    }

    ROMCACHE cache = GenerateCache(sourcePath, fileExtensions, versionCodes);
    PATHLIST listOfChosenRoms = GetList(cache, countryCodes, exclusiveRegions, versionCodes);
    MoveRoms(listOfChosenRoms, sourcePath, destinationPath);
    print_map(cache, outputPath/"cache.txt");
    print_list(listOfChosenRoms, outputPath / "chosen.txt");
}