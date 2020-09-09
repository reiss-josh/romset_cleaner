// a script for organizing romsets quickly
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
static TCHAR szTitle[] = _T("ROMSET Sorter");
HINSTANCE hInst;

//i define my variables here because i'm gross
fs::path outputPath = "N/A";
fs::path srcPath = "";
fs::path destPath = "";
STRLIST countryHierarchy = { "USA" };
STRLIST excludedStrings = { "(Beta)", "(Prototype)", "(Proto)", "(Demo)", "(Wii Virtual Console)" };
STRLIST fileExts = { ".z64" };
STRLIST versionCodes = { "Rev", "ver" };
char folderHolder[2048];
char initialDirectory[2048] = { 'C',':','\\' };

//UTILITIES
//aka functions that could be useful elsewhere
//which i should really really not forget about
//all of which need a lil cleaning up
//but will probably leave here forever

//takes a list of strings and stitches them together
//delimiter: is placed between each string
//upFront: is placed before the first string
//outBack: is placed after the last string
std::string StrlistDelimiter(STRLIST lst, std::string delimiter, std::string upFront = "", std::string outBack = "") {
    std::string lstSum = upFront;
    for (auto const& i : lst) {
        lstSum += i;
        lstSum += delimiter;
    }
    lstSum.erase(lstSum.length() - delimiter.length());
    lstSum += outBack;
    return lstSum;
}

//takes a STRLST and checks if any elements show up in the input string
//this should probably be more generalized in terms of naming
//dependent on StrlistDelimiter
bool MatchExtensions(std::string input, STRLIST fileExtensions) {
    std::string extSum = StrlistDelimiter(fileExtensions, "|");
    std::regex ext(extSum);
    return std::regex_search(input, ext);;
}

//
//https://cpp.hotexamples.com/examples/-/IFileDialog/-/cpp-ifiledialog-class-examples.html
//
#include <shlobj.h>
#include <shtypes.h>
bool getOpenDirectory(char* out, int max_size, const char* starting_dir)
{
    bool ret = false;
    IFileDialog* pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        if (starting_dir)
        {
            PIDLIST_ABSOLUTE pidl;
            WCHAR wstarting_dir[MAX_PATH];
            WCHAR* wc = wstarting_dir;
            for (const char* c = starting_dir; *c && wc - wstarting_dir < MAX_PATH - 1; ++c, ++wc)
            {
                *wc = *c == '/' ? '\\' : *c;
            }
            *wc = 0;

            HRESULT hresult = ::SHParseDisplayName(wstarting_dir, 0, &pidl, SFGAO_FOLDER, 0);
            if (SUCCEEDED(hresult))
            {
                IShellItem* psi;
                hresult = ::SHCreateShellItem(NULL, NULL, pidl, &psi);
                if (SUCCEEDED(hresult))
                {
                    pfd->SetFolder(psi);
                }
                ILFree(pidl);
            }
        }

        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }
        if (SUCCEEDED(pfd->Show(NULL)))
        {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
                WCHAR* tmp;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &tmp)))
                {
                    char* c = out;
                    while (*tmp && c - out < max_size - 1)
                    {
                        *c = (char)*tmp;
                        ++c;
                        ++tmp;
                    }
                    *c = '\0';
                    ret = true;
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    return ret;
}

/* this works but it's just not very convenient. bad dialog.*/
/*
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
*/

/* converts wstring to string
https://codereview.stackexchange.com/questions/419/converting-between-stdwstring-and-stdstring
*/
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

//takes a comma-delimited string and converts it to a list of strings
#include<sstream>
STRLIST CommaDelimitedStrings_ToSTRLST(std::string inputStr) {
    inputStr = std::regex_replace(inputStr, std::regex(", "), ",");
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

//takes a char array and returns a string
std::string charArrtoString(char* boop) {
    std::string s(boop);
    return s;
}

//put elements in a map of lists to a file
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

//put elements in a list to a file
template<typename T>
void print_list(std::list<T> const& m, fs::path filename) {
    std::ofstream outfile;
    outfile.open(filename);
    for (auto const& i : m) {
        outfile << i << "\n";
    }
    outfile.close();
}

//it's print_list, but specifically for filepaths.
void print_fs_list(std::list<fs::path> m, fs::path filename) {
    std::ofstream outfile;
    outfile.open(filename);
    for (auto const& i : m) {
        outfile << i.u8string() << "\n";
    }
    outfile.close();
}

//this isn't actually used because i made it too generalized
//it iterates over picks out the first entry in SET that contains the frontmost-present entry in codeSet
//eg for a codeSet of ("ab", "cd", "ef"), it will first check for the first item in SET that contains "ab"
//if no item contains "ab", then it would check for the first item with "cd", and so on
std::string SearchOrderedSubstringSet(STRLIST set, STRLIST codeSet) {
    for (const auto& code : codeSet) {
        for (const auto& ver : set) {
            if (ver.find(code) != std::string::npos)
                return ver;
        }
    }
    return ""; //if no appropriate match is found
}

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
//  WM_DESTROY  - post a quit message and return

//this is where all the ui stuff happens
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR title[] = _T("ROMSET SORTER");

    switch (message)
    {
        //create all the ui
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

            CreateWindow(TEXT("EDIT"), TEXT("(Beta), (Proto), (Demo)"),
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

            CreateWindowA("STATIC", "<- Country codes, in ranked order.",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 140, 250, 35,
                hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindowA("STATIC", "<- Tags that will be excluded.",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 180, 250, 35,
                hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindowA("STATIC", "<- File extensions to look for.",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 220, 250, 35,
                hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            CreateWindowA("STATIC", "<- Versioning tags for further sorting.",
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                300, 260, 250, 35,
                hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            
            break;
        }
        //make the ui do things
        case WM_COMMAND:
        {
            //fs::path outputPath = "";
            switch (wParam)
            {
                case 1:
                    //srcPath = BrowseFolder("C:\\");
                    getOpenDirectory(folderHolder, 2048, initialDirectory);
                    srcPath = charArrtoString(folderHolder);
                    SetWindowText(GetDlgItem(hWnd, 11), srcPath.c_str());
                    break;
                case 2:
                    getOpenDirectory(folderHolder, 2048, initialDirectory);
                    destPath = charArrtoString(folderHolder);
                    SetWindowText(GetDlgItem(hWnd, 12), destPath.c_str());
                    break;
                case 3:
                    getOpenDirectory(folderHolder, 2048, initialDirectory);
                    outputPath = charArrtoString(folderHolder);
                    SetWindowText(GetDlgItem(hWnd, 13), outputPath.c_str());
                    break;
                case 4:
                    TCHAR   buff[2048];
                    for (int i = 21; i < 25; i++) {
                        GetDlgItemText(hWnd, i, buff, 2048);
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

//always removes region and language
//removes versioncodes if any are specified.
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

//this generated the 'ROMCACHE' which is my fancy name for a 'a map of type <string, strlist>'
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

//this is like SearchOrderedSubstringSet, but i'm also checking for the best versionCode
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

//GetList returns the list of roms that were chosen in the end
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

//move roms is the Scary Function of Great Horror that actually moves the files
//feeding this weird paths can actually fuck up your filesystem
void MoveRoms(PATHLIST listOfChosenRoms, fs::path src, fs::path dst) {
    for (auto const& currRom : listOfChosenRoms) {
        fs::rename(src/currRom, dst/currRom);
    }
}

//bigWorker is really the Main() of this program.
void bigWorker(fs::path sourcePath, fs::path destinationPath, STRLIST countryCodes, STRLIST exclusionStrings, STRLIST fileExtensions, STRLIST versionCodes = {}, fs::path outputPath = "N/A") {
    
    if ((sourcePath == "") || (destPath == "")) return;

    if (fs::exists(sourcePath)) OutputDebugString(L"exists!\n");
    else OutputDebugString(L"source invalid!\n");

    if (!fs::exists(destinationPath)) {
        OutputDebugString(L"creating dest path!\n");
        fs::create_directory(destinationPath);
    }

    if ((!fs::exists(outputPath)) && outputPath != "N/A") {
        OutputDebugString(L"creating output path!\n");
        fs::create_directory(outputPath);
    }

    ROMCACHE cache = GenerateCache(sourcePath, fileExtensions, versionCodes, exclusionStrings);
    PATHLIST listOfChosenRoms = GetList(cache, countryCodes, versionCodes);
    MoveRoms(listOfChosenRoms, sourcePath, destinationPath); //comment this out if you don't want to endanger yourself
    if (outputPath != "N/A")
    {
        print_map(cache, outputPath / "cache.txt"); //debug cache
        print_fs_list(listOfChosenRoms, outputPath / "chosen.txt"); //debug chosen
        //print_list(countryCodes, outputPath / "countrycodes.txt");
        //print_list(fileExtensions, outputPath / "filextensions.txt");
        //print_list(exclusionStrings, outputPath / "exclusionStrings.txt");
    }
}