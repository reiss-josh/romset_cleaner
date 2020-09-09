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
void bigWorker(fs::path sourcePath, fs::path destinationPath, STRLIST countryCodes, STRLIST exclusionStrings, STRLIST fileExtensions, STRLIST versionCodes, fs::path outputPath);

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
        600, 500,
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

#include <shlobj.h>
fs::path BrowseFolder(std::string saved_path)
{
    BROWSEINFO   bi;
    ZeroMemory(&bi, sizeof(bi));
    TCHAR   szDisplayName[MAX_PATH];

    bi.hwndOwner = NULL;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szDisplayName;
    bi.lpszTitle = _T("Please select a folder for storing received files :");
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lParam = NULL;
    bi.iImage = 0;

    LPITEMIDLIST   pidl = SHBrowseForFolder(&bi);
    TCHAR   szPathName[MAX_PATH];
    if (NULL != pidl)
    {
        BOOL bRet = SHGetPathFromIDList(pidl, szPathName);
        if (FALSE == bRet)
            return"";
        OutputDebugString(szPathName);
        std::wstring arr_w(szPathName);
        return arr_w;
    }
    return "";
}

std::string ws2s(const std::wstring& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, buf, len, 0, 0);
    std::string r(buf);
    delete[] buf;
    return r;
}

#include<sstream>
STRLIST CommaDelimitedStrings_ToSTRLST(std::string inputStr) {
    inputStr = std::regex_replace(inputStr, std::regex(", "), ",");
    OutputDebugStringA(inputStr.c_str());
    STRLIST result = {};
    std::stringstream ss(inputStr);
    while (ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        result.push_back(substr);
    }
    return result;
}

//need to get all these parameters somehow
fs::path outputPath = "N/A";
fs::path srcPath = "";
fs::path destPath = "";
STRLIST countryHierarchy = { "USA" };
STRLIST excludedStrings = { "(Beta)", "(Prototype)", "(Proto)", "(Demo)", "(Wii Virtual Console)" };
STRLIST fileExts = { ".z64" };
STRLIST versionCodes = { "Rev", "ver" };
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR title[] = _T("Hello, Windows desktop!");
    TCHAR greeting[] = _T("Hello, Windows desktop!");

    switch (message)
    {
        case WM_CREATE:
        {
            CreateWindow(TEXT("button"), TEXT("Select source directory..."),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 20, 250, 35,
                hWnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindowA("STATIC", "SOURCE UNSET",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 20, 250, 35,
                hWnd, (HMENU)11, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(TEXT("button"), TEXT("Select destination directory..."),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 60, 250, 35,
                hWnd, (HMENU)2, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindowA("STATIC", "DESTINATION UNSET",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 60, 250, 35,
                hWnd, (HMENU)12, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(TEXT("button"), TEXT("Select output log directory (optional)..."),
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20, 100, 250, 35,
                hWnd, (HMENU)3, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindowA("STATIC", "N/A",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 100, 250, 35,
                hWnd, (HMENU)13, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(L"BUTTON", L"LET'S DO IT!!", // Button text 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
                20,         // x position 
                300,         // y position 
                250,        // Button width
                35,        // Button height
                hWnd, (HMENU)4, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(TEXT("EDIT"), TEXT("USA, JAPAN, EUROPE"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                20, 140, 250, 35,
                hWnd, (HMENU)21, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(TEXT("EDIT"), TEXT("(BETA), (PROTO), (DEMO)"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                20, 180, 250, 35,
                hWnd, (HMENU)22, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(TEXT("EDIT"), TEXT(".z64"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                20, 220, 250, 35,
                hWnd, (HMENU)23, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindow(TEXT("EDIT"), TEXT("Rev, ver"),
                WS_VISIBLE | WS_CHILD | WS_BORDER,
                20, 260, 250, 35,
                hWnd, (HMENU)24, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            
            break;
        }
        case WM_COMMAND:
        {
            //fs::path outputPath = "";
            switch (wParam)
            {
                case 1:
                    srcPath = BrowseFolder("C:\\");
                    SetWindowText(GetDlgItem(hWnd, 11), srcPath.c_str());
                    break;
                case 2:
                    destPath = BrowseFolder("C:\\");
                    SetWindowText(GetDlgItem(hWnd, 12), destPath.c_str());
                    break;
                case 3:
                    outputPath = BrowseFolder("C:\\");
                    SetWindowText(GetDlgItem(hWnd, 13), outputPath.c_str());
                    break;
                case 4:
                    TCHAR   buff[MAX_PATH];
                    for (int i = 21; i < 25; i++) {
                        GetDlgItemText(hWnd, i, buff, 1024);
                        std::wstring arr_w(buff);
                        STRLIST result = CommaDelimitedStrings_ToSTRLST(ws2s(arr_w));
                        if (i == 21) countryHierarchy = result;
                        if (i == 22) excludedStrings = result;
                        if (i == 23) fileExts = result;
                        if (i == 24) versionCodes = result;
                    }
                    bigWorker(srcPath, destPath, countryHierarchy, excludedStrings, fileExts, versionCodes, outputPath);
                    break;
            }
            break;
        }
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
    outfile.close();
}

//for file weirdness
void print_fs_list(std::list<fs::path> m, fs::path filename) {
    std::ofstream outfile;
    outfile.open(filename);
    for (auto const& i : m) {
        outfile << i.u8string() << "\n";
    }
    outfile.close();
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
    return std::regex_search(input, ext);;
}

ROMCACHE GenerateCache(fs::path source, STRLIST fileExtensions, STRLIST versionCodes, STRLIST exclusionStrings) {
    ROMCACHE cache;
    int srcLength = source.u8string().length() + 1;
    std::regex exc(StrlistDelimiter(exclusionStrings, "|"));

    for (const auto& entry : fs::directory_iterator(source)) {
        std::string filename = entry.path().u8string();
        filename.erase(0, srcLength);
        if (MatchExtensions(filename, fileExtensions) && !(std::regex_search(filename, exc))) {
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

std::string SearchOrderedSubstringSet(STRLIST set, STRLIST codeSet) {
    for (const auto& code : codeSet) {
        for (const auto& ver : set) {
            if (ver.find(code) != std::string::npos)
                return ver;
        }
    }
    return ""; //if no appropriate match is found
}

fs::path SelectBestVersion(STRLIST versions, STRLIST countryCodes, std::regex versionCodesEx) {
    std::smatch m;
    std::string bestVer = "";
    std::string highestRev = "";
    for (const auto& country : countryCodes) {
        for (const auto& ver : versions) {
            if (ver.find(country) != std::string::npos)
            {
                std::regex_search(ver, m, versionCodesEx);
                if (m.str() >= highestRev) //if the version code is higher than the one for highestRev
                {
                    bestVer = ver;
                    highestRev = m.str();
                }
            }
        }
        if (bestVer != "") return bestVer;
    }
    return bestVer; //if no appropriate match is found
}

PATHLIST GetList(ROMCACHE cache, STRLIST countryCodes, STRLIST versionCodes) {
    PATHLIST lst = {};
    for (auto const& pair : cache) {
        fs::path best = SelectBestVersion(pair.second, countryCodes, std::regex(StrlistDelimiter(versionCodes, "[^)]*\\)| \\(", " \\(", "[^)]*\\)")));
        if (best != std::string()) {
            lst.push_back(best);
        }
    }
    return lst;
}

void MoveRoms(PATHLIST listOfChosenRoms, fs::path src, fs::path dst) {
    for (auto const& currRom : listOfChosenRoms) {
        fs::rename(src/currRom, dst/currRom);
    }
}

void bigWorker(fs::path sourcePath, fs::path destinationPath, STRLIST countryCodes, STRLIST exclusionStrings, STRLIST fileExtensions, STRLIST versionCodes = {}, fs::path outputPath = "N/A") {
    
    if (fs::exists(sourcePath)) OutputDebugString(L"exists!");
    else OutputDebugString(L"source invalid!");

    if (!fs::exists(destinationPath)) {
        OutputDebugString(L"creating dest path!");
        fs::create_directory(destinationPath);
    }

    if ((!fs::exists(outputPath)) && outputPath != "N/A") {
        OutputDebugString(L"creating output path!");
        fs::create_directory(outputPath);
    }

    ROMCACHE cache = GenerateCache(sourcePath, fileExtensions, versionCodes, exclusionStrings);
    PATHLIST listOfChosenRoms = GetList(cache, countryCodes, versionCodes);
    //MoveRoms(listOfChosenRoms, sourcePath, destinationPath);
    if (outputPath != "N/A")
    {
        print_map(cache, outputPath / "cache.txt"); //debug cache
        print_fs_list(listOfChosenRoms, outputPath / "chosen.txt"); //debug chosen
        print_list(countryCodes, outputPath / "countrycodes.txt");
        print_list(fileExtensions, outputPath / "filextensions.txt");
        print_list(exclusionStrings, outputPath / "exclusionStrings.txt");
    }
}