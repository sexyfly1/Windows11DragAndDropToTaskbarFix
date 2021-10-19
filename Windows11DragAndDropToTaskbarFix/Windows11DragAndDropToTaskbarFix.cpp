#include <iostream>
#include <windows.h>
#include <string.h>
#include <chrono>
#include <Shlobj.h>
#include <filesystem>
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"
#include <set>
#include <psapi.h>
#include <sstream>
#include <fstream>
#include <shlwapi.h>
#include <regex>
#include <thread>
#include <ole2.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment( linker, "/subsystem:windows" )
using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

#define DONT_INCLUDE_UNUSED_FUNCTIONS_TO_PREVENT_PSEUDO_ANTIVIRUSES_FROM_THROWING_FALSE_POSITIVES 1

//Basic configuration:
bool AutomaticallyRunThisProgramOnStartup = true;
bool ShowConsoleWindowOnStartup = true;
bool PrintDebugInfo = false;
bool UseTheNewBestMethodEver = true;
bool AutoOpenFirstWindowInBestMethodEver = true;
int HowLongSleepBetweenDifferentKeysPressMilliseconds = 10;
int HowLongSleepBetweenTheSameKeysPressMilliseconds = 0;
int HowLongSleepAfterAutoOpenFirstWindowMilliseconds = 100;
int PreviewWindowChangeDetectionMaxMilliseconds = 1000;//Keep it higher. It's non-blocking time.

int HowLongLeftMouseButtonPressedBeforeContinueMilliseconds = 750;
int HowLongKeepMouseOverAppIconBeforeRestoringWindowMilliseconds = 200;//750 before.
int DefaultSleepPeriodInTheLoopMilliseconds = 100;
int SleepPeriodWhenLeftMouseButtonIsPressedInTheLoopMilliseconds = 25;
int SleepPeriodWhenMouseIsOnAppIconInTheLoopMilliseconds = 10;
int DefaultTaskbarIconWidth = 44;
int DefaultTaskbarIconHeight = 48;
int DefaultShowDesktopButtonWidth = 20;

//Unused (or actually can be used), but not important. Warning, some functions are not longer included in the release
bool UseTheNewWMHOTKEYMethod = true;//Not that reliable method as I thought
bool UseTheNewWorkaroundForButtonsElevenPlus = false;//Not needed. Wasted time on it :(
bool CheckIfPinnedAppsWindowsAreVisible = false;//Not needed. Wasted time on it :(
bool DetectIfFileIsCurrentlyDraggedUsingClipboard = true;//Not working for now
int CheckForNewActiveWindowForButtonsElevenPlusMilliseconds = 2500;//Unused by default
int SleepTimeButtonsElevenPlusMilliseconds = 5;//Unused by default
int AnimationLagButtonsElevenPlusMilliseconds = 100;//Unused by default


//Dynamic variables:

wstring CurrentExeWorks = L"";
wstring CurrentExeWorksFilenameOnly = L"";
wstring CurrentExeWorksPath = L"";

bool dirExists(const std::wstring& dirName_in)
{
	DWORD ftyp = GetFileAttributesW(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

BOOL FileExistsW(LPCWSTR szPath)
{
	/*if (boost::filesystem::exists(szPath)) {
		return true;
	}
	return false;*/

	DWORD dwAttrib = GetFileAttributesW(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL FileExists(LPCTSTR szPath)
{
	/*if (boost::filesystem::exists(szPath)) {
		return true;
	}
	return false;*/

	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// https://www.oreilly.com/library/view/c-cookbook/0596007612/ch04s13.html

void toUpper(basic_string<char>& s) {
	for (basic_string<char>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = toupper(*p); // toupper is for char
	}
}

void toUpper(basic_string<wchar_t>& s) {
	for (basic_string<wchar_t>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = towupper(*p); // towupper is for wchar_t
	}
}

void toLower(basic_string<char>& s) {
	for (basic_string<char>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = tolower(*p);
	}
}

void toLower(basic_string<wchar_t>& s) {
	for (basic_string<wchar_t>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = towlower(*p);
	}
}

string Mona_toUpper(string Input) {
	string s = Input;
	for (basic_string<char>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = toupper(*p); // toupper is for char
	}
	return s;
}

wstring Mona_toUpperWs(wstring Input) {
	wstring s = Input;
	for (basic_string<wchar_t>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = towupper(*p); // towupper is for wchar_t
	}
	return s;
}

string Mona_toLower(string Input) {
	string s = Input;
	for (basic_string<char>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = tolower(*p);
	}
	return s;
}

wstring Mona_toLowerWs(wstring Input) {
	wstring s = Input;
	for (basic_string<wchar_t>::iterator p = s.begin();
		p != s.end(); ++p) {
		*p = towlower(*p);
	}
	return s;
}
bool is_number(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

//Mona's config loader:

bool NewIsConfigLineEqualTo(string ConfigLine, string SearchFor, string SearchForValue) {
	string SearchForTmp = SearchFor + "=" + SearchForValue;
	std::size_t sssearch1 = ConfigLine.find(SearchForTmp);
	if (sssearch1 != std::string::npos) {
		return true;
	}
	else {
		SearchForTmp = SearchFor + " = " + SearchForValue;
		sssearch1 = ConfigLine.find(SearchForTmp);
		if (sssearch1 != std::string::npos) {
			return true;
		}
		else {
			SearchForTmp = SearchFor + " =" + SearchForValue;
			sssearch1 = ConfigLine.find(SearchForTmp);
			if (sssearch1 != std::string::npos) {
				return true;
			}
			else {
				SearchForTmp = SearchFor + "= " + SearchForValue;
				sssearch1 = ConfigLine.find(SearchForTmp);
				if (sssearch1 != std::string::npos) {
					return true;
				}
			}
		}
	}
	return false;
}

long long int NewConfigGetIntValueAfter(string ConfigLine, string SearchFor) {
	bool GotCorrectString = false;
	string CorrectString = "";
	string SearchForTmp = SearchFor + "=";
	std::size_t sssearch1 = ConfigLine.find(SearchForTmp);
	if (sssearch1 != std::string::npos) {
		CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
		//MessageBox(Message4, CorrectString.c_str(), "Str Test", MB_OK | MB_ICONERROR);
		GotCorrectString = true;
	}
	else {
		SearchForTmp = SearchFor + " = ";
		sssearch1 = ConfigLine.find(SearchForTmp);
		if (sssearch1 != std::string::npos) {
			CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
			GotCorrectString = true;
		}
		else {
			SearchForTmp = SearchFor + " =";
			sssearch1 = ConfigLine.find(SearchForTmp);
			if (sssearch1 != std::string::npos) {
				CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
				GotCorrectString = true;
			}
			else {
				SearchForTmp = SearchFor + "= ";
				sssearch1 = ConfigLine.find(SearchForTmp);
				if (sssearch1 != std::string::npos) {
					CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
					GotCorrectString = true;
				}
			}
		}
	}

	if (GotCorrectString) {
		//Not needed in this program I guess. Too lazy to use boost in this project.:

		/*find_and_replace(CorrectString, "\r\n", "");
		find_and_replace(CorrectString, "\r", "");
		find_and_replace(CorrectString, "\n", "");*/
		if (CorrectString.length() > 0) {
			if (is_number(CorrectString)) {
				return atoll(CorrectString.c_str());
			}
		}
	}
	return -696969;//Control value
}

double NewConfigGetDoubleValueAfter(string ConfigLine, string SearchFor) {
	bool GotCorrectString = false;
	string CorrectString = "";
	string SearchForTmp = SearchFor + "=";
	std::size_t sssearch1 = ConfigLine.find(SearchForTmp);
	if (sssearch1 != std::string::npos) {
		CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
		GotCorrectString = true;
	}
	else {
		SearchForTmp = SearchFor + " = ";
		sssearch1 = ConfigLine.find(SearchForTmp);
		if (sssearch1 != std::string::npos) {
			CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
			GotCorrectString = true;
		}
		else {
			SearchForTmp = SearchFor + " =";
			sssearch1 = ConfigLine.find(SearchForTmp);
			if (sssearch1 != std::string::npos) {
				CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
				GotCorrectString = true;
			}
			else {
				SearchForTmp = SearchFor + "= ";
				sssearch1 = ConfigLine.find(SearchForTmp);
				if (sssearch1 != std::string::npos) {
					CorrectString = ConfigLine.substr(sssearch1 + SearchForTmp.length());
					GotCorrectString = true;
				}
			}
		}
	}

	if (GotCorrectString) {
		/*find_and_replace(CorrectString, "\r\n", "");
		find_and_replace(CorrectString, "\r", "");
		find_and_replace(CorrectString, "\n", "");*/
		if (CorrectString.length() > 0) {
			//if (is_number(CorrectString)) {
			std::string::size_type sz;     // alias of size_t
			double earth = std::stod(CorrectString, &sz);
			return earth;
			//}
		}
	}
	return -69.69;//Control value
}

//I know people reading the code below will be facepalming, but I don't care. I use the functions which I created when I was 14.

wstring ConfigFile = L"Windows11DragAndDropToTaskbarFixConfig.txt";

void Mona_Load_Configuration() {
	string line = "";
	int rowcnt = 0;
	long long int TmpValueFromNewConfigGetIntFunction = -696969;
	if(FileExists(ConfigFile.c_str())){
	ifstream settingsfile(ConfigFile, ios::binary);
	if (settingsfile.is_open()) {
		while (!settingsfile.eof()) {
			getline(settingsfile, line);
			++rowcnt;
			if (line.length() > 0) {

				if (rowcnt > 1000) {
					//Just in case someone loads a 1000TB file as a config.
					break;
				}


				if (line.length() > 1) {
					string test2 = line.substr(0, 1);
					if (test2 == "/") {
						continue;
					}
					else if (test2 == "#") {
						continue;
					}
					else if (test2 == ";") {
						continue;
					}
					string LastChar = line.substr(line.length() - 1);
					if (LastChar == ";") {
						line = line.substr(0, line.length() - 1);
					}
				}

				if (NewIsConfigLineEqualTo(line, "PrintDebugInfo", "1") || NewIsConfigLineEqualTo(line, "PrintDebugInfo", "true")) {
					PrintDebugInfo = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "PrintDebugInfo", "0") || NewIsConfigLineEqualTo(line, "PrintDebugInfo", "false")) {
					PrintDebugInfo = false;
					continue;
				}

				if (NewIsConfigLineEqualTo(line, "UseTheNewBestMethodEver", "1") || NewIsConfigLineEqualTo(line, "UseTheNewBestMethodEver", "true")) {
					UseTheNewBestMethodEver = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "UseTheNewBestMethodEver", "0") || NewIsConfigLineEqualTo(line, "UseTheNewBestMethodEver", "false")) {
					UseTheNewBestMethodEver = false;
					continue;
				}

				if (NewIsConfigLineEqualTo(line, "UseTheNewWMHOTKEYMethod", "1") || NewIsConfigLineEqualTo(line, "UseTheNewWMHOTKEYMethod", "true")) {
					UseTheNewWMHOTKEYMethod = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "UseTheNewWMHOTKEYMethod", "0") || NewIsConfigLineEqualTo(line, "UseTheNewWMHOTKEYMethod", "false")) {
					UseTheNewWMHOTKEYMethod = false;
					continue;
				}

				if (NewIsConfigLineEqualTo(line, "UseTheNewWorkaroundForButtonsElevenPlus", "1") || NewIsConfigLineEqualTo(line, "UseTheNewWorkaroundForButtonsElevenPlus", "true")) {
					UseTheNewWorkaroundForButtonsElevenPlus = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "UseTheNewWorkaroundForButtonsElevenPlus", "0") || NewIsConfigLineEqualTo(line, "UseTheNewWorkaroundForButtonsElevenPlus", "false")) {
					UseTheNewWorkaroundForButtonsElevenPlus = false;
					continue;
				}

				if (NewIsConfigLineEqualTo(line, "ShowConsoleWindowOnStartup", "1") || NewIsConfigLineEqualTo(line, "ShowConsoleWindowOnStartup", "true")) {
					ShowConsoleWindowOnStartup = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "ShowConsoleWindowOnStartup", "0") || NewIsConfigLineEqualTo(line, "ShowConsoleWindowOnStartup", "false")) {
					ShowConsoleWindowOnStartup = false;
					continue;
				}

				if (NewIsConfigLineEqualTo(line, "CheckIfPinnedAppsWindowsAreVisible", "1") || NewIsConfigLineEqualTo(line, "CheckIfPinnedAppsWindowsAreVisible", "true")) {
					CheckIfPinnedAppsWindowsAreVisible = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "CheckIfPinnedAppsWindowsAreVisible", "0") || NewIsConfigLineEqualTo(line, "CheckIfPinnedAppsWindowsAreVisible", "false")) {
					CheckIfPinnedAppsWindowsAreVisible = false;
					continue;
				}

				if (NewIsConfigLineEqualTo(line, "AutomaticallyRunThisProgramOnStartup", "1") || NewIsConfigLineEqualTo(line, "AutomaticallyRunThisProgramOnStartup", "true")) {
					AutomaticallyRunThisProgramOnStartup = true;
					continue;
				}
				else if (NewIsConfigLineEqualTo(line, "AutomaticallyRunThisProgramOnStartup", "0") || NewIsConfigLineEqualTo(line, "AutomaticallyRunThisProgramOnStartup", "false")) {
					AutomaticallyRunThisProgramOnStartup = false;
					continue;
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "HowLongLeftMouseButtonPressedBeforeContinueMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						HowLongLeftMouseButtonPressedBeforeContinueMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "HowLongKeepMouseOverAppIconBeforeRestoringWindowMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						HowLongKeepMouseOverAppIconBeforeRestoringWindowMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "CheckForNewActiveWindowForButtonsElevenPlusMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						CheckForNewActiveWindowForButtonsElevenPlusMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "SleepTimeButtonsElevenPlusMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						SleepTimeButtonsElevenPlusMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "AnimationLagButtonsElevenPlusMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						AnimationLagButtonsElevenPlusMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "DefaultSleepPeriodInTheLoopMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						DefaultSleepPeriodInTheLoopMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "SleepPeriodWhenLeftMouseButtonIsPressedInTheLoopMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						SleepPeriodWhenLeftMouseButtonIsPressedInTheLoopMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "SleepPeriodWhenMouseIsOnAppIconInTheLoopMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						SleepPeriodWhenMouseIsOnAppIconInTheLoopMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "HowLongSleepBetweenTheSameKeysPressMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > -1) {//For performance purposes disallow 0s for now.
						HowLongSleepBetweenTheSameKeysPressMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "HowLongSleepAfterAutoOpenFirstWindowMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > -1) {//For performance purposes disallow 0s for now.
						HowLongSleepAfterAutoOpenFirstWindowMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "PreviewWindowChangeDetectionMaxMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						PreviewWindowChangeDetectionMaxMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "HowLongSleepBetweenDifferentKeysPressMilliseconds");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > -1) {//For performance purposes disallow 0s for now.
						HowLongSleepBetweenDifferentKeysPressMilliseconds = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "DefaultTaskbarIconWidth");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						DefaultTaskbarIconWidth = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "DefaultTaskbarIconHeight");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						DefaultTaskbarIconHeight = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}

				TmpValueFromNewConfigGetIntFunction = NewConfigGetIntValueAfter(line, "DefaultShowDesktopButtonWidth");
				if (TmpValueFromNewConfigGetIntFunction != -696969) {
					if (TmpValueFromNewConfigGetIntFunction > 0) {//For performance purposes disallow 0s for now.
						DefaultShowDesktopButtonWidth = static_cast<int>(TmpValueFromNewConfigGetIntFunction);
						continue;
					}
				}
			}
		}
	}
	}
}


std::time_t ReturnConfigFileTime() {

	if (FileExists(ConfigFile.c_str())) {
		std::filesystem::path FileWithPath(ConfigFile);
		std::filesystem::path full_p = absolute(FileWithPath);
		auto LastWriteTime = std::filesystem::last_write_time(full_p);
		const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(LastWriteTime);
		const auto time = std::chrono::system_clock::to_time_t(systemTime);
		time_t toreturn = time;
		//MessageBox(Message2, "watchdogs return time", _T("0"), MB_YESNO | MB_ICONQUESTION);
		return toreturn;

	}
	//MessageBox(Message2, "watchdogs return", _T("0"), MB_YESNO | MB_ICONQUESTION);
	return 0;
}

#ifndef DONT_INCLUDE_UNUSED_FUNCTIONS_TO_PREVENT_PSEUDO_ANTIVIRUSES_FROM_THROWING_FALSE_POSITIVES

// https://docs.microsoft.com/en-us/windows/win32/shell/links?redirectedfrom=MSDN

HRESULT ResolveIt(HWND hwnd, LPCSTR lpszLinkFile, LPWSTR lpszPath, int iPathBufferSize)
{
	HRESULT hres;
	IShellLink* psl;
	WCHAR szGotPath[MAX_PATH];
	WCHAR szDescription[MAX_PATH];
	WIN32_FIND_DATA wfd;

	*lpszPath = 0; // Assume failure 

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.
	//printf("0\n");
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		//printf("1\n");
		IPersistFile* ppf;

		// Get a pointer to the IPersistFile interface. 
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

		if (SUCCEEDED(hres))
		{
			//printf("2\n");
			WCHAR wsz[MAX_PATH];

			// Ensure that the string is Unicode. 
			MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);

			// Add code here to check return value from MultiByteWideChar 
			// for success.

			// Load the shortcut. 
			hres = ppf->Load(wsz, STGM_READ);

			if (SUCCEEDED(hres))
			{
				//printf("3\n");
				// Resolve the link. 
				hres = psl->Resolve(hwnd, 0);

				if (SUCCEEDED(hres))
				{
					//printf("4\n");
					// Get the path to the link target. 
					//hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_RAWPATH);

					if (SUCCEEDED(hres))
					{
						//printf("5\n");
						// Get the description of the target. 
						hres = psl->GetDescription(szDescription, MAX_PATH);

						if (SUCCEEDED(hres))
						{
							//printf("6\n");
							hres = StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
							if (SUCCEEDED(hres))
							{
								// Handle success
							}
							else
							{
								// Handle the error
							}
						}
					}
				}
			}

			// Release the pointer to the IPersistFile interface. 
			ppf->Release();
		}

		// Release the pointer to the IShellLink interface. 
		psl->Release();
	}
	return hres;
}

bool AlreadySetAppDataFolders = false;
wstring AppDataFolder = L"";

std::vector<vector<wstring>> Pinned_apps_lnk_to_exe_vector;

//const size_t maxPids = 1024;
const size_t maxPids = 2048;
std::set<std::wstring> processes;

typedef BOOL(WINAPI* LPQueryFullProcessImageName)(
	HANDLE hProcess, DWORD dwFlags, LPSTR lpExeName, PDWORD lpdwSize);

typedef BOOL(WINAPI* LPQueryFullProcessImageNameW)(
	HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);

void Check_if_Full_Exe_Path_is_currently_running() {
	DWORD pids[maxPids] = {};
	DWORD bytesReturned = 0;
	HMODULE hDLL = LoadLibraryA("kernel32.dll");
	//Now use pointer to get access to functions defined in DLL
	LPQueryFullProcessImageNameW fpQueryFullProcessImageName = (LPQueryFullProcessImageNameW)GetProcAddress(hDLL, "QueryFullProcessImageNameW"); //ANSI version
	if (::EnumProcesses(pids, sizeof pids, &bytesReturned))
	{
		DWORD cProcesses = bytesReturned / sizeof * pids;

		// set SeDebug privilege

		for (DWORD i = 0; i < cProcesses; i++)
		{
			DWORD pid = pids[i];

			if (HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid))
			{
				wchar_t filename[MAX_PATH] = { 0 };
				wchar_t filenamenew[MAX_PATH] = { 0 };
				DWORD charsCarried = MAX_PATH;
				if (fpQueryFullProcessImageName)
				{
					if ((*fpQueryFullProcessImageName)(hProcess, 0, filename, &charsCarried))
					{
						wstring filenamewithpathh = filename;

						//Loop by array of .exes from .lnk:
						if (filenamewithpathh.length() > 2) {
							for (size_t iha = 0; iha < Pinned_apps_lnk_to_exe_vector.size(); iha++) {
								if (Mona_toLowerWs(Pinned_apps_lnk_to_exe_vector[iha][1]) == Mona_toLowerWs(filenamewithpathh)) {
									vector<wstring> CurrentArray = Pinned_apps_lnk_to_exe_vector[iha];
									wstring PIDs = L"";
									if (CurrentArray.size() >= 3) {
										PIDs = CurrentArray[2];
									}
									if (PIDs.length() > 0) {
										PIDs = PIDs + L",";
									}
									PIDs = PIDs + to_wstring(pid);
									if (CurrentArray.size() >= 3) {
										CurrentArray[2] = PIDs;
									}
									else {
										CurrentArray.push_back(PIDs);
									}

									Pinned_apps_lnk_to_exe_vector[iha] = CurrentArray;
									if (PrintDebugInfo) {
										//std::wcout << L"PIDs for Process: " << Pinned_apps_lnk_to_exe_vector[iha][1] << L" Are: " << PIDs << std::endl;
									}
								}
							}
						}

					}
					else
					{
						// handle error
					}


				}
				else {
					//Windows XP, no need code for it here.
				}
				::CloseHandle(hProcess);
			}
		}
	}
}

vector<wstring> split_ws(const wstring& s, const wstring& delim, const bool keep_empty = true, std::wstring AddAtTheEnd = L"", std::wstring AddAtTheTop = L"") {
	vector<wstring> result;
	if (delim.empty()) {
		result.push_back(s);
		return result;
	}
	wstring::const_iterator substart = s.begin(), subend;
	while (true) {
		subend = search(substart, s.end(), delim.begin(), delim.end());
		wstring temp(substart, subend);
		if (keep_empty || !temp.empty()) {
			//std::wcout << "temp: " << temp << std::endl;
			if (AddAtTheEnd.length() > 0) {
				wstring temp2 = temp + AddAtTheEnd;
				result.push_back(temp2);
			}
			else if (AddAtTheTop.length() > 0) {
				wstring temp2 = AddAtTheTop + temp;
				result.push_back(temp2);
			}
			else {
				result.push_back(temp);
			}
		}
		if (subend == s.end()) {
			break;
		}
		substart = subend + delim.size();
	}
	return result;
}

inline unsigned int stoui(const std::string& s)
{
	std::istringstream reader(s);
	unsigned int val = 0;
	reader >> val;
	return val;
}

inline unsigned int stouiWs(const std::wstring& s)
{
	std::wistringstream reader(s);
	unsigned int val = 0;
	reader >> val;
	return val;
}

struct handle_data {
	unsigned long process_id;
	HWND best_handle;
};


BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle)) {
		return TRUE;
	}
	data.best_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}

BOOL IsProcessRunningPID(DWORD pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);
	return ret == WAIT_TIMEOUT;
}


void Check_if_any_window_from_these_PIDs_is_open() {
	for (size_t iha = 0; iha < Pinned_apps_lnk_to_exe_vector.size(); iha++) {
			wstring PIDsFound = L"";
			vector<wstring> PiDs_Array;
			if (Pinned_apps_lnk_to_exe_vector[iha].size() >= 3) {
				PIDsFound = Pinned_apps_lnk_to_exe_vector[iha][2];
			}
			if (PIDsFound.length() > 0) {
				PiDs_Array = split_ws(PIDsFound, L",", false);
			}
			//std::wcout << "PIDsFound: " << Pinned_apps_lnk_to_exe_vector[iha][1] << L". PIDs: " << PIDsFound << "Array size: " << PiDs_Array.size() << std::endl;

			if (PiDs_Array.size() > 0) {
				for (size_t iha2 = 0; iha2 < PiDs_Array.size(); iha2++) {
					DWORD NicePID = stouiWs(PiDs_Array[iha2]);
					//std::wcout << "NicePID: " << Pinned_apps_lnk_to_exe_vector[iha][1] << L". PID: " << NicePID << std::endl;
					if (NicePID > 0) {
						if (IsProcessRunningPID(NicePID)) {
							//std::wcout << "Process is running: " << Pinned_apps_lnk_to_exe_vector[iha][1] << L". PID: " << NicePID << std::endl;
							HWND MainWindow = find_main_window(NicePID);
							if (MainWindow) {
								vector<wstring> CurrentVector = Pinned_apps_lnk_to_exe_vector[iha];
								wstring AtLeastOneHWNDFound = L"true";
								if (CurrentVector.size() >= 4) {
									CurrentVector[3] = AtLeastOneHWNDFound;
								}
								else {
									CurrentVector.push_back(AtLeastOneHWNDFound);
								}
								//Update vector. Wonder if it's legal on active loop...
								Pinned_apps_lnk_to_exe_vector[iha] = CurrentVector;

								if (PrintDebugInfo) {
									std::wcout << "Found Window For App: " << Pinned_apps_lnk_to_exe_vector[iha][1] << L". PID: " << NicePID << ". HWND: " << MainWindow << std::endl;
								}
							}
							else {
								if (PrintDebugInfo) {
									std::wcout << "Didn't find any window for App: " << Pinned_apps_lnk_to_exe_vector[iha][1] << L". PID: " << NicePID << std::endl;
								}
							}
						}
					}
				}
			}
			
	}
}

#endif

bool RegistryGetStringValue(std::wstring &valueBuf, HKEY MainKey, const std::wstring& regSubKey, const std::wstring& regValue)
{
	size_t bufferSize = 0xFFF; // If too small, will be resized down below.
	//std::wstring valueBuf; // Contiguous buffer since C++11.
	valueBuf.resize(bufferSize);
	auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
	auto rc = RegGetValueW(
		MainKey,
		regSubKey.c_str(),
		regValue.c_str(),
		RRF_RT_REG_SZ,
		nullptr,
		static_cast<void*>(valueBuf.data()),
		&cbData
	);
	while (rc == ERROR_MORE_DATA)
	{
		// Get a buffer that is big enough.
		cbData /= sizeof(wchar_t);
		if (cbData > static_cast<DWORD>(bufferSize))
		{
			bufferSize = static_cast<size_t>(cbData);
		}
		else
		{
			bufferSize *= 2;
			cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
		}
		valueBuf.resize(bufferSize);
		rc = RegGetValueW(
			MainKey,
			regSubKey.c_str(),
			regValue.c_str(),
			RRF_RT_REG_SZ,
			nullptr,
			static_cast<void*>(valueBuf.data()),
			&cbData
		);
	}
	if (rc == ERROR_SUCCESS)
	{
		cbData /= sizeof(wchar_t);
		valueBuf.resize(static_cast<size_t>(cbData - 1)); // remove end null character
		//return valueBuf;
		return true;
	}
	else
	{
		//throw std::runtime_error("Windows system error code: " + std::to_string(rc));
	}
	return false;
}

bool RegistrySetStringValue(HKEY MainKey, const std::wstring& regSubKey, const std::wstring& regValue, const std::wstring& stringToSet)
{
	auto rc = RegSetKeyValueW(
		MainKey,
		regSubKey.c_str(),
		regValue.c_str(),
		//RRF_RT_REG_SZ,
		REG_SZ,
		stringToSet.c_str(),
		stringToSet.length()*2
	);
	
	if (rc == ERROR_SUCCESS)
	{
		return true;
	}
	else
	{
		//throw std::runtime_error("Windows system error code: " + std::to_string(rc));
	}

	return false;
}

bool RegistryDeleteKeyValue(HKEY MainKey, const std::wstring& regSubKey, const std::wstring& regValue)
{
	auto rc = RegDeleteKeyValueW(
		MainKey,
		regSubKey.c_str(),
		regValue.c_str()
	);

	if (rc == ERROR_SUCCESS)
	{
		return true;
	}
	else
	{
		//throw std::runtime_error("Windows system error code: " + std::to_string(rc));
	}

	return false;
}

void Check_And_Set_Auto_Program_Startup() {
	wstring QueryKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	wstring QueryValue = L"Windows11DragAndDropToTaskbarFix";
	wstring QueryValueOld = L"Windows11DragAndDropToTaskbarPartialFix";

	bool FoundInRegistry = false;
	bool CorrectInRegistry = false;

	wstring CurrentExeWorksForRegistry = L"\"";
	CurrentExeWorksForRegistry = CurrentExeWorksForRegistry + CurrentExeWorks + L"\"";

	//Delete old Autostart:
	if (RegistryDeleteKeyValue(HKEY_CURRENT_USER, QueryKey, QueryValueOld)) {
		if (PrintDebugInfo) {
			std::wcout << L"Successfully removed the old executable name Auto Startup registry key value for: " << CurrentExeWorks << std::endl;
		}
	}


	wstring InAutostart = L"";
	if (RegistryGetStringValue(InAutostart, HKEY_CURRENT_USER, QueryKey, QueryValue)) {
		if (InAutostart.length() > 0) {
			FoundInRegistry = true;
			if (CurrentExeWorksForRegistry == InAutostart) {
				CorrectInRegistry = true;
				if (PrintDebugInfo) {
					std::wcout << L"The Auto Startup registry key value is correct. No need to update." << std::endl;
				}
			}
			else {
				std::wcout << L"The existing Auto Startup registry key value: " << InAutostart << L" Length: " << InAutostart.length() << L" does not match the current process: " << CurrentExeWorksForRegistry << L" Length: " << CurrentExeWorksForRegistry.length() << std::endl;
			}
		}
	}
	if (AutomaticallyRunThisProgramOnStartup) {
		if (!FoundInRegistry || !CorrectInRegistry) {
			//Set the key
			if (RegistrySetStringValue(HKEY_CURRENT_USER, QueryKey, QueryValue, CurrentExeWorksForRegistry)) {
				if (PrintDebugInfo) {
					std::wcout << L"Successfully set the Auto Startup registry key value to: " << CurrentExeWorks << std::endl;
				}
			}
			else {
				if (PrintDebugInfo) {
					std::wcout << L"Error setting the Auto Startup registry key value to: " << CurrentExeWorks << std::endl;
				}
			}
		}

	}
	else {
		if (FoundInRegistry) {
			//Delete key
			if (RegistryDeleteKeyValue(HKEY_CURRENT_USER, QueryKey, QueryValue)) {
				if (PrintDebugInfo) {
					std::wcout << L"Successfully removed the Auto Startup registry key value for: " << CurrentExeWorks << std::endl;
				}
			}
			else {
				if (PrintDebugInfo) {
					std::wcout << L"ERROR: Unable to delete the Auto Startup registry key value for: " << CurrentExeWorks << std::endl;
				}
			}
		}
	}
}


#ifndef DONT_INCLUDE_UNUSED_FUNCTIONS_TO_PREVENT_PSEUDO_ANTIVIRUSES_FROM_THROWING_FALSE_POSITIVES
size_t LastBufferSize = 0;

std::wstring GetBinaryValueFromHKCU(const std::wstring& regSubKey, const std::wstring& regValue)
{
	LastBufferSize = 0;
	size_t bufferSize = 0xFFF; // If too small, will be resized down below.
	std::wstring valueBuf; // Contiguous buffer since C++11.
	valueBuf.resize(bufferSize);
	auto cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
	auto rc = RegGetValueW(
		HKEY_CURRENT_USER,
		regSubKey.c_str(),
		regValue.c_str(),
		RRF_RT_REG_BINARY,
		nullptr,
		static_cast<void*>(valueBuf.data()),
		&cbData
	);
	while (rc == ERROR_MORE_DATA)
	{
		// Get a buffer that is big enough.
		cbData /= sizeof(wchar_t);
		if (cbData > static_cast<DWORD>(bufferSize))
		{
			bufferSize = static_cast<size_t>(cbData);
		}
		else
		{
			bufferSize *= 2;
			cbData = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
		}
		valueBuf.resize(bufferSize);
		rc = RegGetValueW(
			HKEY_CURRENT_USER,
			regSubKey.c_str(),
			regValue.c_str(),
			RRF_RT_REG_BINARY,
			nullptr,
			static_cast<void*>(valueBuf.data()),
			&cbData
		);
	}
	if (rc == ERROR_SUCCESS)
	{
		cbData /= sizeof(wchar_t);
		valueBuf.resize(static_cast<size_t>(cbData - 1)); // remove end null character
		//std::wcout << L"error success: " << valueBuf << std::endl;
		LastBufferSize = bufferSize;
		return valueBuf;
	}
	else
	{
		//throw std::runtime_error("Windows system error code: " + std::to_string(rc));
	}

	return L"";
}

void Check_Pinned_Apps() {
	//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Taskband\Favorites
	wstring QueryKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Taskband";
	wstring QueryValue = L"Favorites";

	wstring FavouritesData = GetBinaryValueFromHKCU(QueryKey, QueryValue);
	std::wcout << L"LastBufferSize: " << LastBufferSize << std::endl;
	if (LastBufferSize > 0) {
		std::size_t Test1 = FavouritesData.find(L".lnk",0, LastBufferSize);
		if (Test1 != std::wstring::npos) {
			wstring TestWstr = FavouritesData.substr(Test1);
			std::wcout << L"TestWstr: " << TestWstr << std::endl;
		}

		std::wcout << L"FavouritesData: " << FavouritesData << std::endl;
		//system("pause");
	}

	if (!AlreadySetAppDataFolders) {
		//%AppData%\Microsoft\Internet Explorer\Quick Launch\User Pinned\TaskBar
		wchar_t appdata[MAX_PATH];
		SHGetSpecialFolderPathW(NULL, appdata, CSIDL_APPDATA, 1);
		AppDataFolder = appdata;
		AppDataFolder = AppDataFolder + L"\\Microsoft\\Internet Explorer\\Quick Launch\\User Pinned\\TaskBar";
		AlreadySetAppDataFolders = true;
	}
	if (dirExists(AppDataFolder)) {
		//std::wcout << L"Directory Exists: " << AppDataFolder << std::endl;

		Pinned_apps_lnk_to_exe_vector.clear();
		for (const auto& entry : fs::directory_iterator(AppDataFolder)) {
			if (entry.path().extension() == ".lnk") {
				//std::wcout << L"Extension for file is .lnk: " << entry.path() << std::endl;

				//LPWSTR ResolvedLink;
				wchar_t ResolvedLink[MAX_PATH];
				wchar_t ResolvedLinkExpanded[MAX_PATH];

				HRESULT ReadLink = ResolveIt(NULL, entry.path().string().c_str(), ResolvedLink, MAX_PATH);
				if (SUCCEEDED(ReadLink)) {
					ExpandEnvironmentStringsW(ResolvedLink, ResolvedLinkExpanded, MAX_PATH);
					wstring ResolvedLinkWstr = ResolvedLinkExpanded;
					if (ResolvedLinkWstr.length() < 1) {
						if (Mona_toLowerWs(entry.path().filename().wstring()) == L"file explorer.lnk") {
							//std::wcout << L"FILE EXPLORER" << std::endl;
							ResolvedLinkWstr = L"C:\\WINDOWS\\explorer.exe";
						}
					}
					if (ResolvedLinkWstr.length() > 2) {
						std::filesystem::path pp = ResolvedLinkWstr;
						std::filesystem::path pp_absolute = std::filesystem::absolute(ResolvedLinkWstr);
						std::filesystem::path p_absolute = std::filesystem::absolute(entry.path());

						if (PrintDebugInfo) {
							std::wcout << "Original: " << ResolvedLink << "Link: " << entry.path() << L" Resolved: " << pp_absolute << std::endl;
						}

						vector<wstring> CurrentAppLinkVector;
						CurrentAppLinkVector.push_back(p_absolute.wstring());
						CurrentAppLinkVector.push_back(pp_absolute.wstring());

						Pinned_apps_lnk_to_exe_vector.push_back(CurrentAppLinkVector);

					}
					else {
						if (PrintDebugInfo) {
							std::wcout << "Couldn't determine path for link: " << entry.path() << L". Resolved: " << ResolvedLinkWstr << std::endl;
						}
					}
				}
				
			}
		}
	}

	//Test
	if (Pinned_apps_lnk_to_exe_vector.size() >= 1) {
		Check_if_Full_Exe_Path_is_currently_running();
		Check_if_any_window_from_these_PIDs_is_open();

		/*for (size_t iha = 0; iha < Pinned_apps_lnk_to_exe_vector.size(); iha++) {
			if (PrintDebugInfo) {
				wstring PIDsFound = L"";
				if (Pinned_apps_lnk_to_exe_vector[iha].size() >= 3) {
					PIDsFound = Pinned_apps_lnk_to_exe_vector[iha][2];
				}
				std::wcout << "Test App Link: " << Pinned_apps_lnk_to_exe_vector[iha][0] << L". Resolved: " << Pinned_apps_lnk_to_exe_vector[iha][1] << ". PIDs: " << PIDsFound << std::endl;
			}
		}*/
	}
}

#endif

HHOOK HandleLowLevelMousePressProc;
bool LeftButtonPressedATM = false;
std::chrono::milliseconds LastTimeClickedLeftMouseButton = std::chrono::milliseconds(0);
POINT MouseClickStartPoint;
POINT MouseClickStartPoint_Client;
long long int Current_UniqueID_of_the_click = 0;
bool LLMP_Temporarily_Dont_Update_UniqueID = false;

long long int Last_UniqueID_session_of_Experimental_Workaround = -1;

//Important: This Callback can slow down mouse move of the system!!! Don't put anything unnecessary here, and use the other thread!
static LRESULT CALLBACK LowLevelMousePressProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION) {

		if (wParam == WM_LBUTTONDOWN)
		{
			LeftButtonPressedATM = true;
			LastTimeClickedLeftMouseButton = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			GetCursorPos(&MouseClickStartPoint);
			if (!LLMP_Temporarily_Dont_Update_UniqueID) {
			Current_UniqueID_of_the_click++;
			}
		}

		else if (wParam == WM_LBUTTONUP)
		{
			LeftButtonPressedATM = false;
			//std::wcout << L"WM_LBUTTONUP was called" << endl;
		}
	}
	return CallNextHookEx(HandleLowLevelMousePressProc, nCode, wParam, lParam);
}

DWORD WINAPI MouseClickWatchdogThread(void* data) {
	
	HINSTANCE hInstLowLevelMousePressProc = GetModuleHandle(NULL);
	HandleLowLevelMousePressProc = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMousePressProc, hInstLowLevelMousePressProc, 0);
	MSG msgMouseClickWatchdog;
	while (GetMessage(&msgMouseClickWatchdog, 0, 0, 0))
	{
		PeekMessage(&msgMouseClickWatchdog, 0, 0, 0, 0x0001);
	}
	UnhookWindowsHookEx(HandleLowLevelMousePressProc);
	return 0;
}

POINT P;
POINT P_Client;
char Keyboard_Keys_One_to_Zero[10] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30 };

const std::vector<std::pair<WPARAM, LPARAM>> New_WM_HOTKEY_Array_LogoWin_CTRL_Num = {
	std::pair<WPARAM, LPARAM> {(WPARAM)0x20F, (LPARAM)0x31000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x210, (LPARAM)0x32000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x211, (LPARAM)0x33000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x212, (LPARAM)0x34000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x213, (LPARAM)0x35000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x214, (LPARAM)0x36000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x215, (LPARAM)0x37000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x216, (LPARAM)0x38000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x217, (LPARAM)0x39000A},
	std::pair<WPARAM, LPARAM> {(WPARAM)0x218, (LPARAM)0x30000A},
};

bool CurrentlyLeftMouseButtonIsPressed = false;
bool DetectedHWNDsForThisMouseClick = false;
HWND hWndTray = 0;
HWND hWndTrayNotify = 0;
HWND hWndRebar = 0;
HWND hWndMSTaskSwWClass = 0;
HWND hWndWindowForShowDesktopArea = 0;
HWND TaskListThumbnailWnd = 0;
RECT desktop;
HWND hDesktop = 0;
int ShowDesktopStartPosition = 0;
int TaskbarWindowWidth = 0;
int NumberOfItemsOnTaskbar = 0;
int LastSimulatedHotkeyPressID = -1;
int PreviousHoveredMouseAppID = -1;
std::chrono::milliseconds FirstTimeClickedLeftMouseButton = std::chrono::milliseconds(0);
std::chrono::milliseconds FirstTimeHoveredOverThisAppIcon = std::chrono::milliseconds(0);
std::chrono::milliseconds TimeNow = std::chrono::milliseconds(0);
std::chrono::milliseconds LastTimeCheckedForConfigUpdate = std::chrono::milliseconds(0);
time_t LastSettingsChangeTime = 0;
bool CheckedConfigTimeAtLeastOneTime = false;

long long int Previous_UniqueID_of_the_click;

int SleepPeriodNow = DefaultSleepPeriodInTheLoopMilliseconds;

long long Previous_UniqueID_of_the_click_Best_Method_Ever = -1;
int Previous_Button_Number = -1;
HWND PreviousForegroundWindow;

int JustClickedEnterForBestMethodEver = 0;
RECT rectPreviousTaskListThumbnailWnd;
bool PreviousTaskListThumbnailWndVisible = false;

//Hmm, we might need to optimize it in the future. Quite pointless to reset it every time left mouse button is unclicked.
void ResetTmpVariables() {
	JustClickedEnterForBestMethodEver = 0;
	PreviousTaskListThumbnailWndVisible = false;
	Previous_UniqueID_of_the_click_Best_Method_Ever = -1;
	Previous_Button_Number = -1;
	PreviousHoveredMouseAppID = -1;
	LastSimulatedHotkeyPressID = -1;
	DetectedHWNDsForThisMouseClick = false;
	SleepPeriodNow = DefaultSleepPeriodInTheLoopMilliseconds;
	FirstTimeClickedLeftMouseButton = std::chrono::milliseconds(0);
}

void find_and_replace(string& source, string const& find, string const& replace)
{
	//boost::replace_all(source, find, replace);
	for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
	{
	source.replace(i, find.length(), replace);
	i += replace.length() - find.length() + 1;
	}
}

void find_and_replace_ws(wstring& source, wstring const& find, wstring const& replace)
{
	//boost::replace_all(source, find, replace);
	for (std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
	{
	source.replace(i, find.length(), replace);
	i += replace.length() - find.length() + 1;
	}
}

void NewFunctionToKill(bool ReLaunch = true) {
	wstring ExeEhe = CurrentExeWorksFilenameOnly;
	wstring ExeEhePath = CurrentExeWorks;

	find_and_replace_ws(ExeEhe, L"\\", L"\\\\");
	find_and_replace_ws(ExeEhePath, L"\\", L"\\\\");

	wstring TaskKill = L"taskkill /F /IM \"" + ExeEhe + L"\"";
	wstring EmptyStringOrWithArgs = L"";

	wstring KillAndLaunch;
	if (ReLaunch) {
		KillAndLaunch = TaskKill + L" & start \"\" \"" + ExeEhePath + L"\" \"killed-restarted" + EmptyStringOrWithArgs + L"\" & exit";
	}
	else {
		KillAndLaunch = TaskKill + L"& exit";
	}

	_wsystem(KillAndLaunch.c_str());
}

void Simulate_Show_Desktop_Behaviour() {
	if (!hWndTray) {
		hWndTray = FindWindow(L"Shell_TrayWnd", nullptr);
	}
	if (hWndTray) {
		LRESULT res = SendMessage(hWndTray, WM_COMMAND, (WPARAM)419, 0);
	}
}

void Simulate_ALT_Plus_TAB_Hotkey(int SleepFor = 50) {
	keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), 0, 0); //Press ALT
	keybd_event(VK_TAB, MapVirtualKey(VK_TAB, 0), 0, 0); //Press TAB
	Sleep(SleepFor);
	keybd_event(VK_TAB, MapVirtualKey(VK_TAB, 0), KEYEVENTF_KEYUP, 0); //Release TAB
	keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0); //Release ALT
}

#ifndef DONT_INCLUDE_UNUSED_FUNCTIONS_TO_PREVENT_PSEUDO_ANTIVIRUSES_FROM_THROWING_FALSE_POSITIVES

HWND Wait_For_The_New_Active_Window(HWND &ActiveWindowBeforeClickingOnAppIcon, wstring DebugStep) {

	//Hotfix:
	HWND ProgManHWND = FindWindow(L"Progman", nullptr);
	if (PrintDebugInfo) {
		std::wcout << L"Found Progman window HWND: " << ProgManHWND << endl;
	}

	//Wait for the new Active Window:
	std::chrono::milliseconds SleepFor(SleepTimeButtonsElevenPlusMilliseconds);

	bool FoundNewActiveWindow = false;
	HWND ActiveWindowNow = NULL;
	HWND ActiveWindowNowToReturn = NULL;

	std::chrono::milliseconds StartedTheLoopAt = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	long long int StartedTheLoopAtCountPlus = StartedTheLoopAt.count() + CheckForNewActiveWindowForButtonsElevenPlusMilliseconds;

	while (true) {
		std::chrono::milliseconds TimeAtTheLoopNow = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		//ActiveWindowNow = GetActiveWindow();
		//ActiveWindowNow = GetTopWindow(NULL);
		ActiveWindowNow = GetForegroundWindow();
		if (ActiveWindowNow) {
			if (ActiveWindowNow != ProgManHWND && ActiveWindowNow != ActiveWindowBeforeClickingOnAppIcon) {
				if (IsWindowVisible(ActiveWindowNow)) {
					FoundNewActiveWindow = true;
					ActiveWindowNowToReturn = ActiveWindowNow;
					break;
				}
			}
		}
		if (TimeAtTheLoopNow.count() >= StartedTheLoopAtCountPlus) {
			if (PrintDebugInfo) {
				long long int Difference = TimeAtTheLoopNow.count() - StartedTheLoopAt.count();
				std::wcout << L"Unfortunately, the Wait_For_The_New_Active_Window() loop has timeouted after: " << Difference << L" ms." << endl;
			}
			break;
		}
		std::this_thread::sleep_for(SleepFor);
	}

	//This one sleeps for too long. Not a good method.
	/*for (int iii = 0; iii < NumberOfAttemptsInThisLoop; iii++) {
		ActiveWindowNow = GetActiveWindow();
		if (ActiveWindowNow) {
			if (ActiveWindowNow != ActiveWindowBeforeClickingOnAppIcon) {
				FoundNewActiveWindow = true;
				break;
			}
		}
		std::this_thread::sleep_for(SleepFor);
	}*/

	if (PrintDebugInfo) {
		wchar_t ActiveWindowBeforeText[MAX_PATH];
		wchar_t ActiveWindowBeforeClassName[MAX_PATH];
		wchar_t ActiveWindowAfterText[MAX_PATH];
		wchar_t ActiveWindowAfterTextClassName[MAX_PATH];
		SendMessageW(ActiveWindowBeforeClickingOnAppIcon, WM_GETTEXT, MAX_PATH, (LPARAM)ActiveWindowBeforeText);
		SendMessageW(ActiveWindowNow, WM_GETTEXT, MAX_PATH, (LPARAM)ActiveWindowAfterText);
		GetClassNameW(ActiveWindowBeforeClickingOnAppIcon, ActiveWindowBeforeClassName, MAX_PATH);
		GetClassNameW(ActiveWindowNow, ActiveWindowAfterTextClassName, MAX_PATH);

		std::wcout << DebugStep << L". ActiveWindowBefore: " << ActiveWindowBeforeClickingOnAppIcon << L" (" << ActiveWindowBeforeText << L")" << L" (" << ActiveWindowBeforeClassName << L")" << L". ActiveWindowNow: " << ActiveWindowNow << " (" << ActiveWindowAfterText << L")" << " (" << ActiveWindowAfterTextClassName << L"). Returning: " << ActiveWindowNowToReturn << endl;
	}

	return ActiveWindowNowToReturn;
}

POINT MouseClickStartPointCopy;
HWND FirstWindowWhereDragged;
bool ItWasDesktopWindow = false;

bool Wait_For_Following_Window_To_Become_Foreground_Under_Mouse(HWND& ActiveWindowBeforeClickingOnAppIcon) {
	std::chrono::milliseconds SleepFor(SleepTimeButtonsElevenPlusMilliseconds);
	std::chrono::milliseconds StartedTheLoopAt = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	long long int StartedTheLoopAtCountPlus = StartedTheLoopAt.count() + CheckForNewActiveWindowForButtonsElevenPlusMilliseconds;
	HWND ActiveWindowNow = NULL;
	HWND TmpWindowFromPoint = NULL;
	while (true) {
		std::chrono::milliseconds TimeAtTheLoopNow = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		ActiveWindowNow = WindowFromPoint(MouseClickStartPointCopy);
		if (ActiveWindowNow) {
			if (ActiveWindowNow == ActiveWindowBeforeClickingOnAppIcon) {
				if (IsWindowVisible(ActiveWindowNow)) {
					return true;
					break;
				}
			}
		}
		if (TimeAtTheLoopNow.count() >= StartedTheLoopAtCountPlus) {
			if (PrintDebugInfo) {
				wchar_t ActiveWindowAfterText[MAX_PATH];
				wchar_t ActiveWindowAfterTextClassName[MAX_PATH];
				SendMessageW(ActiveWindowNow, WM_GETTEXT, MAX_PATH, (LPARAM)ActiveWindowAfterText);
				GetClassNameW(ActiveWindowNow, ActiveWindowAfterTextClassName, MAX_PATH);

				long long int Difference = TimeAtTheLoopNow.count() - StartedTheLoopAt.count();
				std::wcout << L"Unfortunately, the Wait_For_Following_Window_To_Become_Foreground_Under_Mouse() loop has timeouted after: " << Difference << L" ms. Last window: "<< ActiveWindowNow << L" " << ActiveWindowAfterText << L" (" << ActiveWindowAfterTextClassName << L")" << endl;
			}
			break;
		}
		std::this_thread::sleep_for(SleepFor);
	}

	return false;
}

void Experimental_Workaround_for_buttons_Eleven_Plus() {
	//Experimental Workaround Clicks the icon and returns to the previous state.

	HWND destop = GetDesktopWindow();
	HWND hWorkerW = NULL;
	HWND hShellViewWin = NULL;
	do
	{
		hWorkerW = FindWindowEx(destop, hWorkerW, L"WorkerW", NULL);
		hShellViewWin = FindWindowEx(hWorkerW, 0, L"SHELLDLL_DefView", 0);
	} while (hShellViewWin == NULL && hWorkerW != NULL);

	//FolderViewWindow is used to determine if something was dragged from the Desktop window
	HWND FolderViewWindow = FindWindowEx(hShellViewWin, 0, L"SysListView32", nullptr);

	bool StillSameSession = false;
	if (Last_UniqueID_session_of_Experimental_Workaround == Current_UniqueID_of_the_click) {
		//Use old Start Click Point
		StillSameSession = true;
		if (PrintDebugInfo) {
			std::wcout << L"Still the same session in Experimental_Workaround_for_buttons_Eleven_Plus()." << endl;
		}
	}
	else {
		MouseClickStartPointCopy = MouseClickStartPoint;
		StillSameSession = false;

		FirstWindowWhereDragged = WindowFromPoint(MouseClickStartPointCopy);
		if (FirstWindowWhereDragged == FolderViewWindow) {
			if (PrintDebugInfo) {
				std::wcout << L"It's desktop window!" << endl;
			}
			ItWasDesktopWindow = true;
		}
		else {
			if (PrintDebugInfo) {
				std::wcout << L"It's not a desktop window!" << endl;
			}
			ItWasDesktopWindow = false;
		}

		if (PrintDebugInfo) {
			wchar_t ActiveWindowBeforeText[MAX_PATH];
			wchar_t ActiveWindowBeforeClassName[MAX_PATH];
			SendMessageW(FirstWindowWhereDragged, WM_GETTEXT, MAX_PATH, (LPARAM)ActiveWindowBeforeText);
			GetClassNameW(FirstWindowWhereDragged, ActiveWindowBeforeClassName, MAX_PATH);

			std::wcout << L"FirstWindowWhereDragged INFO: " << FirstWindowWhereDragged << L" " << ActiveWindowBeforeText << L" (" << ActiveWindowBeforeClassName << L")" << endl;
		}
	}

	Last_UniqueID_session_of_Experimental_Workaround = Current_UniqueID_of_the_click;

	int screenX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int screenY = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	if (StillSameSession){
		if (ItWasDesktopWindow) {
			Simulate_Show_Desktop_Behaviour();
		}
		//Try to restore original window:
		else {
			SetForegroundWindow(FirstWindowWhereDragged);
			SetActiveWindow(FirstWindowWhereDragged);
			SetFocus(FirstWindowWhereDragged);
		}

		//Wait for Window to become foreground under mouse.
		//Note, it works bad for the Desktop window, so we avoid it.
		if (!ItWasDesktopWindow && Wait_For_Following_Window_To_Become_Foreground_Under_Mouse(FirstWindowWhereDragged)) {
			if (PrintDebugInfo) {
				std::wcout << L"Good. Wait_For_Following_Window_To_Become_Foreground() returned true." << endl;
			}
		}
		
		//Unfortunately, we still need some short (or long?) sleep here, so other windows have time to hide.
		Sleep(AnimationLagButtonsElevenPlusMilliseconds);

	}

	//Get mouse position now on taskbar:
	POINT MousePositionNowOnTaskbar;
	GetCursorPos(&MousePositionNowOnTaskbar);

	INPUT inputt = { 0 };
	inputt.mi.dx = MousePositionNowOnTaskbar.x * (65535 / screenX);
	inputt.mi.dy = MousePositionNowOnTaskbar.y * (65535 / screenY);
	//inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	inputt.type = INPUT_MOUSE;

	//HWND ActiveWindowBeforeClickingOnAppIcon = GetActiveWindow();
	//HWND ActiveWindowBeforeClickingOnAppIcon = GetTopWindow(NULL);
	HWND ActiveWindowBeforeClickingOnAppIcon = GetForegroundWindow();

	//Inform the LowLevelMousePressProc not to update the Unique ID for the left mouse button.
	//It's because we don't want to repeat the same function on the same button. It's just a hotfix:
	LLMP_Temporarily_Dont_Update_UniqueID = true;

	//Release the left mouse click together with the dragged item:
	inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	SendInput(1, &inputt, sizeof INPUT);
	//Sleep(10);
	//Press the left mouse button in order to open the window under it:
	inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &inputt, sizeof INPUT);
	//Sleep(10);
	//Release the left mouse button
	inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTUP;
	SendInput(1, &inputt, sizeof INPUT);

	HWND ActiveWindowNow = Wait_For_The_New_Active_Window(ActiveWindowBeforeClickingOnAppIcon, L"Step 1");

	if (ActiveWindowNow) {
		//Unfortunately, we still need some short (or long?) sleep here, so other windows have time to hide.
		Sleep(AnimationLagButtonsElevenPlusMilliseconds);

		HWND ActiveWindowNextOne = GetForegroundWindow();

		//Return to original window:
		if (ItWasDesktopWindow) {
			if (PrintDebugInfo) {
				std::wcout << L"It was a desktop window, so calling Simulate_Show_Desktop_Behaviour()" << endl;
			}
			Simulate_Show_Desktop_Behaviour();

			//Hope it fixes detection
			SetForegroundWindow(FirstWindowWhereDragged);
			SetActiveWindow(FirstWindowWhereDragged);
			SetFocus(FirstWindowWhereDragged);
		}
		else {
			//ALT + TAB is more reliable method
			if (PrintDebugInfo) {
				std::wcout << L"It wasn't desktop, so calling Simulate_ALT_Plus_TAB_Hotkey()" << endl;
			}
			Simulate_ALT_Plus_TAB_Hotkey();

			//Hope it fixes detection
			SetForegroundWindow(FirstWindowWhereDragged);
			SetActiveWindow(FirstWindowWhereDragged);
			SetFocus(FirstWindowWhereDragged);
		}

		//Sleep(10);
		//Sleep(250);
		//Sleep(250);
		
		//Wait again for window change:
		HWND ActiveWindowNow2 = Wait_For_The_New_Active_Window(ActiveWindowNextOne, L"Step 2");

		if (ActiveWindowNow2) {
			//Unfortunately, we still need some short (or long?) sleep here, so other windows have time to hide.
			Sleep(AnimationLagButtonsElevenPlusMilliseconds);

			HWND ActiveWindowNextOne2 = GetForegroundWindow();

			//Move mouse to the previous location where the left mouse button was clicked to start dragging the file.
			inputt.mi.dx = MouseClickStartPointCopy.x * (65535 / screenX);
			inputt.mi.dy = MouseClickStartPointCopy.y * (65535 / screenY);
			inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
			SendInput(1, &inputt, sizeof INPUT);
			//Sleep(10);

			//Press the left mouse button down in order to start dragging again:
			inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &inputt, sizeof INPUT);

			//Sleep(10);

			//Move the mouse to the last position on the taskbar not to make users confused (or to make them more confused):
			inputt.mi.dx = MousePositionNowOnTaskbar.x * (65535 / screenX);
			inputt.mi.dy = MousePositionNowOnTaskbar.y * (65535 / screenY);
			inputt.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE;
			SendInput(1, &inputt, sizeof INPUT);
			
			Simulate_ALT_Plus_TAB_Hotkey();

			//Confirm that window re-appeared:

			if (PrintDebugInfo) {
				HWND ActiveWindowNow3 = Wait_For_The_New_Active_Window(ActiveWindowNextOne2, L"Step 3");
				if (ActiveWindowNow3) {
					if (PrintDebugInfo) {
						std::wcout << L"Window " << ActiveWindowNow3 << L" re-appeared successfully." << endl;
					}
				}
			}

		//}

		}
	}

	//Restore the default Unique ID behaviour in LowLevelMousePressProc:
	LLMP_Temporarily_Dont_Update_UniqueID = false;
}

#endif


void Finally_The_Best_Method_Ever(int ButtonID, int AllButtonsNumber) {
	HWND CurrentForegroundWindow = GetForegroundWindow();

	bool SameUniqueDragSession = false;
	if (Current_UniqueID_of_the_click == Previous_UniqueID_of_the_click_Best_Method_Ever) {
		if (CurrentForegroundWindow == PreviousForegroundWindow) {
			SameUniqueDragSession = true;
		}
		else if (AutoOpenFirstWindowInBestMethodEver) {
			if (JustClickedEnterForBestMethodEver > 0) {
				JustClickedEnterForBestMethodEver++;
			}
			if (JustClickedEnterForBestMethodEver > 1) {
				//No
			}
		}
		else {
			JustClickedEnterForBestMethodEver = 0;
		}
	}
	else {
		JustClickedEnterForBestMethodEver = 0;
	}

	//Hotfix to not spam the same hotkeys on selected icon
	if (Previous_Button_Number == ButtonID) {
		if (PreviousForegroundWindow == hWndTray && CurrentForegroundWindow == hWndTray) {
			//std::wcout << "Returning to prevent hotkeys spam..." << endl;
			return;
		}
		else if (AutoOpenFirstWindowInBestMethodEver) {
			if (JustClickedEnterForBestMethodEver > 2) {
				//std::wcout << "Returning to prevent hotkeys spam after ENTER was clicked..." << endl;
				return;
			}
			else if (JustClickedEnterForBestMethodEver > 0) {
				//Sleep after the previous call, because it just pressed enter, and we didn't want to block that function cus buttons could change.
				//Also IsWindowVisible check would return incorrect value because animation had no time to disappear.

				//EDIT. It can't be in the new call, unfortunately. When button changes and mouse quickly moved to an inactive app icon, it would open it:
				/*if (HowLongSleepAfterAutoOpenFirstWindowMilliseconds > 0) {
					Sleep(HowLongSleepAfterAutoOpenFirstWindowMilliseconds);
				}*/
			}
		}
	}

	//Hotfix, because this window will get focus when hotkey is pressed
	if (!SameUniqueDragSession) {
		if (CurrentForegroundWindow != hWndTray) {
			CurrentForegroundWindow = hWndTray;
		}
	}

	if (PrintDebugInfo) {
		std::wcout << L"SameUniqueDragSession True/False: " << SameUniqueDragSession << L" Window:" << CurrentForegroundWindow << endl;
	}

	Previous_UniqueID_of_the_click_Best_Method_Ever = Current_UniqueID_of_the_click;

	bool WithShift = false;
	float Half_of_Buttons = AllButtonsNumber / 2;

	int RealNumberToClick = ButtonID;

	if (!SameUniqueDragSession) {
		//Hotfix, it needs + 1;
		RealNumberToClick = RealNumberToClick + 1;
		if ((float)ButtonID <= Half_of_Buttons) {
			WithShift = false;
		}
		else {
			WithShift = true;
			RealNumberToClick = AllButtonsNumber - ButtonID;
			//Hotfix, it needs + 1;
			RealNumberToClick = RealNumberToClick + 1;
		}
	}
	else {
		RealNumberToClick = abs(Previous_Button_Number - ButtonID);
		if (ButtonID == Previous_Button_Number) {
			//Do nothing
			RealNumberToClick = 0;
			WithShift = false;
		}
		else if (ButtonID > Previous_Button_Number) {
			WithShift = false;
		}
		else if (ButtonID < Previous_Button_Number) {
			WithShift = true;
		}
	}

	PreviousForegroundWindow = CurrentForegroundWindow;

	if (PrintDebugInfo) {
		std::wcout << L"Half_of_Buttons: " << Half_of_Buttons << L". ButtonID:" << ButtonID << L". AllButtonsNumber: " << AllButtonsNumber << L". RealNumberToClick: " << RealNumberToClick << L". WithShift:" << WithShift << endl;
	}

	//Win+SHIFT+T is reverse order.
	bool ArrowsMethod = true;

	if (ArrowsMethod) {
		//We need to use arrows instead
		if (RealNumberToClick > 0) {

			//Hotfix ver. 1.3.1, set focus to hWndTray to fix Task Manager causing hotkey not activating issues.
			SetForegroundWindow(hWndTray);
			SetActiveWindow(hWndTray);
			SetFocus(hWndTray);

			//Unfortunately, WM_HOTKEY is not so perfect. Buggy when switching to window and back to taskbar without dropping.
			/*LRESULT SendMessageReturn;
			if (WithShift) {
				SendMessageReturn = SendMessage(hWndTray, WM_HOTKEY, (WPARAM)0x1FF, (LPARAM)0x0054000C);
			}
			else {
				SendMessageReturn = SendMessage(hWndTray, WM_HOTKEY, (WPARAM)0x1FE, (LPARAM)0x00540008);
			}
			Sleep(50);
			std::wcout << L"RealNumberToClick > 0. SendMessageReturn: " << SendMessageReturn << endl;*/

			//Check if CTRL is pressed, because this hotkey won't activate with it:
			bool CTRL_Was_Down = false;
			if (GetKeyState(VK_LCONTROL) & 0x8000)
			{
				CTRL_Was_Down = true;
				keybd_event(VK_LCONTROL, MapVirtualKey(VK_LCONTROL, 0), KEYEVENTF_KEYUP, 0);//Release CTRL
			}

			//For some reasons doesn't seem to work, but we can leave it in code.
			bool ALT_Was_Down = false;
			if (GetKeyState(VK_MENU) & 0x8000)
			{
				ALT_Was_Down = true;
				keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);//Release ALT
			}

			if (CTRL_Was_Down || ALT_Was_Down) {
				if (HowLongSleepBetweenDifferentKeysPressMilliseconds > 0) {
					Sleep(HowLongSleepBetweenDifferentKeysPressMilliseconds);
				}
			}

			keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), 0, 0);//Pres Logo Win
			if (WithShift) {
				keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, 0), 0, 0);//Pres Shift
			}
			keybd_event(0x54, MapVirtualKey(0x54, 0), 0, 0);//Press T
			if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
				Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
			}
			keybd_event(0x44, MapVirtualKey(0x54, 0), KEYEVENTF_KEYUP, 0); //Release T
			if (WithShift) {
				keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, 0), KEYEVENTF_KEYUP, 0);//Release Shift
			}
			keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), KEYEVENTF_KEYUP, 0);//Release Logo Win
			
			if (HowLongSleepBetweenDifferentKeysPressMilliseconds > 0) {
				Sleep(HowLongSleepBetweenDifferentKeysPressMilliseconds);
			}

			//Push CTRL again not to interrupt user's action
			if (CTRL_Was_Down) {
				keybd_event(VK_LCONTROL, MapVirtualKey(VK_LCONTROL, 0), 0, 0);//Pres Ctrl
			}
			if (ALT_Was_Down) {
				keybd_event(VK_MENU, MapVirtualKey(VK_MENU, 0), 0, 0);//Pres Alt
			}

			for (int iii = 0; iii < RealNumberToClick - 1; iii++) {
				if (WithShift) {
					keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), 0, 0);
					if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
						Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
					}
					keybd_event(VK_LEFT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_KEYUP, 0);
				}
				else {
					keybd_event(VK_RIGHT, MapVirtualKey(VK_LEFT, 0), 0, 0);
					if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
						Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
					}
					keybd_event(VK_RIGHT, MapVirtualKey(VK_LEFT, 0), KEYEVENTF_KEYUP, 0);
				}
				//std::wcout << L"Click" << iii << endl;
				if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
					Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
				}
			}

			//ver 1.3.0

			if (AutoOpenFirstWindowInBestMethodEver) {
				//This way we still get the preview window of active apps
				if (Previous_Button_Number != ButtonID) {
						//Sleep to make sure arrow arrive at the location.
						if (HowLongSleepBetweenDifferentKeysPressMilliseconds > 0) {
							Sleep(HowLongSleepBetweenDifferentKeysPressMilliseconds);
						}

						keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), 0, 0);
						if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
							Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
						}
						keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_KEYUP, 0);
						if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
							Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
						}

						//Press UP second time (just in case):
						keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), 0, 0);
						if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
							Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
						}
						keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_KEYUP, 0);

						//Sleep to make sure arrow arrives at the location. should be before IsWindowVisible!
						if (HowLongSleepBetweenDifferentKeysPressMilliseconds > 0) {
							Sleep(HowLongSleepBetweenDifferentKeysPressMilliseconds);
						}
						
						RECT rectAtTheMoment;

						//Loop to make sure rect change:
						//std::chrono::milliseconds SleepFor(SleepTimeButtonsElevenPlusMilliseconds);
						std::chrono::milliseconds SleepFor(5);
						std::chrono::milliseconds StartedTheLoopAt = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
						long long int StartedTheLoopAtCountPlus = StartedTheLoopAt.count() + PreviewWindowChangeDetectionMaxMilliseconds;

						//Yeah, get it in case loop is skipped
						GetWindowRect(TaskListThumbnailWnd, &rectAtTheMoment);

						if (PreviousTaskListThumbnailWndVisible) {
							while (IsWindowVisible(TaskListThumbnailWnd)) {
								std::chrono::milliseconds TimeAtTheLoopNow = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
								GetWindowRect(TaskListThumbnailWnd, &rectAtTheMoment);
								if (rectAtTheMoment.left != rectPreviousTaskListThumbnailWnd.left) {
									if (PrintDebugInfo) {
										std::wcout << L"Detected TaskListThumbnailWnd rect change. Interrupting the loop. Before: " << rectPreviousTaskListThumbnailWnd.left << ". After: " << rectAtTheMoment.left << endl;
									}
									break;
								}
								if (TimeAtTheLoopNow.count() >= StartedTheLoopAtCountPlus) {
									if (PrintDebugInfo) {
										long long int Difference = TimeAtTheLoopNow.count() - StartedTheLoopAt.count();
										std::wcout << L"The IsWindowVisible() loop timeouted after: " << Difference << L" ms." << endl;
									}
									break;
								}
								std::this_thread::sleep_for(SleepFor);
							}
						}

						rectPreviousTaskListThumbnailWnd = rectAtTheMoment;
						PreviousTaskListThumbnailWndVisible = IsWindowVisible(TaskListThumbnailWnd);
						if (PrintDebugInfo) {
							std::cout << "TaskListThumbnailWnd Rect: " << rectAtTheMoment.left << ":" << rectAtTheMoment.right << ":" << rectAtTheMoment.bottom << ":" << rectAtTheMoment.top << ". Window visible bool: " << PreviousTaskListThumbnailWndVisible << "\n";
						}

						if (IsWindowVisible(TaskListThumbnailWnd)) {

						//Longer sleep
						if (HowLongSleepBetweenDifferentKeysPressMilliseconds > 0) {
							Sleep(HowLongSleepBetweenDifferentKeysPressMilliseconds);
						}

						//Hotfix to prevent hotkeys spam
						JustClickedEnterForBestMethodEver = 1;

						if (PrintDebugInfo) {
							std::wcout << L"TaskListThumbnailWnd is visible. Simulating ENTER key..." << endl;
						}

						//Simulate the Space key. It then only opens the active window, not starting apps!!!
						keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, 0), 0, 0);
						if (HowLongSleepBetweenTheSameKeysPressMilliseconds) {
							Sleep(HowLongSleepBetweenTheSameKeysPressMilliseconds);
						}
						keybd_event(VK_RETURN, MapVirtualKey(VK_RETURN, 0), KEYEVENTF_KEYUP, 0);
						

						//We still need some sleep here, but different than window rect change detection look:
						if (HowLongSleepAfterAutoOpenFirstWindowMilliseconds > 0) {
							Sleep(HowLongSleepAfterAutoOpenFirstWindowMilliseconds);
						}
					}

					else {
						if (PrintDebugInfo) {
							std::wcout << L"TaskListThumbnailWnd is INVISIBLE. Skipping..." << endl;
						}
					}

					//Longer sleep again to prevent new keys simulation when window is getting restored
					//No, we must move it to above call
				}
				else {
					std::wcout << L"Button is the same so skipping ENTER." << endl;
				}
			}
		}

		/*for (int iii = 0; iii < RealNumberToClick; iii++) {
			if (WithShift) {
				LRESULT SendMessageReturn = SendMessage(hWndTray, WM_HOTKEY, (WPARAM)0x1FF, (LPARAM)0x0054000C);
			}
			else {
				LRESULT SendMessageReturn = SendMessage(hWndTray, WM_HOTKEY, (WPARAM)0x1FE, (LPARAM)0x00540008);
			}
			std::wcout << L"Click" << iii << endl;
			Sleep(1);
		}*/

		/*keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), 0, 0); //Press Up
		Sleep(1);
		keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_KEYUP, 0); //Release Up*/

		//HWND FocusedWindow = GetFocus();
		if (PrintDebugInfo) {
			HWND FocusedWindow = GetForegroundWindow();
			wchar_t ActiveWindowBeforeText[MAX_PATH];
			wchar_t ActiveWindowBeforeClassName[MAX_PATH];
			SendMessageW(FocusedWindow, WM_GETTEXT, MAX_PATH, (LPARAM)ActiveWindowBeforeText);
			GetClassNameW(FocusedWindow, ActiveWindowBeforeClassName, MAX_PATH);
			//std::wcout << L"Finally_The_Best_Method_Ever() Focused Window INFO: " << FocusedWindow << endl;
			std::wcout << L"Finally_The_Best_Method_Ever() Foreground Window INFO: " << FocusedWindow << L" " << ActiveWindowBeforeText << L" (" << ActiveWindowBeforeClassName << L")" << endl;
		}
	}
	else {
		for (int iii = 0; iii < RealNumberToClick; iii++) {
			keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), 0, 0);//Pres Logo Win
			Sleep(1);
			if (WithShift) {
				keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, 0), 0, 0);//Pres Shift
			}
			Sleep(1);
			keybd_event(0x54, MapVirtualKey(0x54, 0), 0, 0);//Press T
			Sleep(1);
			keybd_event(0x44, MapVirtualKey(0x54, 0), KEYEVENTF_KEYUP, 0); //Release T
			Sleep(1);
			if (WithShift) {
				keybd_event(VK_LSHIFT, MapVirtualKey(VK_LSHIFT, 0), KEYEVENTF_KEYUP, 0);//Pres Shift
			}
			Sleep(1);
			keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), KEYEVENTF_KEYUP, 0);//Release Logo Win
			Sleep(1);
		}
		/*Sleep(10);
		//Press arrow up to reset selection
		keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), 0, 0); //Press Up
		Sleep(1);
		keybd_event(VK_UP, MapVirtualKey(VK_UP, 0), KEYEVENTF_KEYUP, 0); //Release Up*/
	}
	Previous_Button_Number = ButtonID;
}

#ifndef DONT_INCLUDE_UNUSED_FUNCTIONS_TO_PREVENT_PSEUDO_ANTIVIRUSES_FROM_THROWING_FALSE_POSITIVES
long long int Unique_ID_of_Click_with_File_Dragged = -1;
long long int Unique_ID_of_Click_with_File_NOT_Dragged = -1;

bool Detect_if_Clipboard_Has_Dragged_File_Data() {
	WORD m_cfHIDA = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	std::wcout << L"Drag format should be: " << m_cfHIDA << endl;

	LPDATAOBJECT lpDataObject;

	HRESULT OleClipboard = OleGetClipboard(&lpDataObject);
	if (OleClipboard == S_OK) {
		LPENUMFORMATETC lpEnumFORMATETC;

		if (lpDataObject->EnumFormatEtc(DATADIR_GET, &lpEnumFORMATETC) != S_OK) {
			std::wcout << L"Failed to EnumFormatEtc" << endl;
			Unique_ID_of_Click_with_File_NOT_Dragged = Current_UniqueID_of_the_click;
			return false;
		}
		if (lpEnumFORMATETC == NULL) {
			std::wcout << L"Error. lpEnumFORMATETC is NULL" << endl;
			Unique_ID_of_Click_with_File_NOT_Dragged = Current_UniqueID_of_the_click;
			return false;
		}

		FORMATETC formatEtc;

		while (lpEnumFORMATETC->Next(1, &formatEtc, NULL) == S_OK)
		{
			//formatEtc
			std::wcout << L"Format: " << formatEtc.cfFormat << endl;
			//if (formatEtc.cfFormat == CFSTR_SHELLIDLIST) {

			//}

			STGMEDIUM stgMedium;
			if (lpDataObject->GetData(&formatEtc, &stgMedium) != S_OK)
			{
				// data is not available
				CoTaskMemFree(formatEtc.ptd);
			}
			else if (stgMedium.pUnkForRelease != NULL)
			{
				// don't cache data with pUnkForRelease != NULL
				::ReleaseStgMedium(&stgMedium);
				CoTaskMemFree(formatEtc.ptd);
			}
			else
			{
				//std::wcout << L"Filename: " << stgMedium.hGlobal << endl;

				// cache the data (now we own the stgMedium)
				//pDataSource->CacheData(0, &stgMedium, &formatEtc);
			}
		}

		std::wcout << L"Done" << endl;

		// cleanup
		lpEnumFORMATETC->Release();

	}
	else {
		//No need.
	}

	return false;
}

void TestCall() {
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	while (true) {
		Detect_if_Clipboard_Has_Dragged_File_Data();
		Sleep(3000);
	}
}

#endif

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR lpCmdLine, int nShowCmd)
{
	//Important to make reading .lnk possible:s
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	//TestCall();
	//system("pause");
	//exit(0);

	//NewTests();
	//system("pause");
	//exit(0);

	wchar_t result[MAX_PATH];
	CurrentExeWorks = std::wstring(result, GetModuleFileNameW(NULL, result, MAX_PATH));
	std::filesystem::path CurrentPath(CurrentExeWorks);
	CurrentExeWorksFilenameOnly = CurrentPath.filename().wstring();
	//std::wcout << CurrentExeWorksFilenameOnly << endl;

	wchar_t CurrentWorkingDirectoryWhereExeIs[MAX_PATH];
	wstring ress = std::wstring(CurrentWorkingDirectoryWhereExeIs, GetModuleFileNameW(NULL, CurrentWorkingDirectoryWhereExeIs, MAX_PATH));
	PathRemoveFileSpecW(CurrentWorkingDirectoryWhereExeIs);
	SetCurrentDirectoryW(CurrentWorkingDirectoryWhereExeIs);
	CurrentExeWorksPath = CurrentWorkingDirectoryWhereExeIs;

	//Test:
	// Unused in ver 1.1. Still working on it :(
	//Check_Pinned_Apps();
	//system("pause");

	//Load configuration:
	Mona_Load_Configuration();

	if (!ShowConsoleWindowOnStartup) {
		//ShowWindow(GetConsoleWindow(), SW_HIDE);
		if (PrintDebugInfo) {
			PrintDebugInfo = false;
		}
	}
	else {
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);

		if (!PrintDebugInfo) {
			HMENU hmenu = GetSystemMenu(GetConsoleWindow(), FALSE);
			EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
		}
	}

	HANDLE handleMutex = CreateMutex(NULL, TRUE, L"MonaWindows11DragToTaskbar-AlreadyRunning");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		printf("Windows11DragAndDropToTaskbarFix is already running. Exiting this instance...\n");
		return 1;
	}

	//Welcome!
	bool HideConsoleWindowSoon = false;
	std::chrono::milliseconds ProgrmStartTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	printf("Windows11DragAndDropToTaskbarFix, ver. 1.3.1, created by Dr.MonaLisa.\n");
	printf("https://github.com/HerMajestyDrMona/Windows11DragAndDropToTaskbarFix\n\n");
	printf("You can disable the console window. Please read the GitHub page to learn how to configure this program.\n");
	if (!PrintDebugInfo) {
		printf("Debug output is disabled, so the console window will be hidden in 10 seconds.\nIn order to terminate this program, please kill \"Windows11DragAndDropToTaskbarFix.exe\" in the Task Manager.\n");
		if (ShowConsoleWindowOnStartup) {
			HideConsoleWindowSoon = true;
		}
	}

	//Check auto start:
	Check_And_Set_Auto_Program_Startup();

	//Start Mouse Click Watchdog Thread 
	HANDLE ThreadHandleMouseClickWatchdogThread = CreateThread(NULL, 0, MouseClickWatchdogThread, NULL, 0, NULL);

	//Test:
	//TestTryToRestoreWindowsUsingWMCommand();
	//system("pause");
	//exit(0);

	bool ConfigFileChangeTimeMonitorAllowed = true;

	while (true) {
		TimeNow = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

		if (HideConsoleWindowSoon) {
			if (TimeNow.count() >= (ProgrmStartTime.count() + 10000)) {
				HideConsoleWindowSoon = false;
				ShowWindow(GetConsoleWindow(), SW_HIDE);
			}
		}
		//Check for config file update:
		if (ConfigFileChangeTimeMonitorAllowed && (TimeNow.count() > LastTimeCheckedForConfigUpdate.count() + 5000)) {
			LastTimeCheckedForConfigUpdate = TimeNow;
			if (FileExists(ConfigFile.c_str())) {
				std::time_t NowSettingsChangeTime = ReturnConfigFileTime();
				if (NowSettingsChangeTime != 0) {
					if (NowSettingsChangeTime > LastSettingsChangeTime) {
						if (CheckedConfigTimeAtLeastOneTime) {
							//File changed, restart the program?
							wstring ReloadChangesTitie = L"Windows11DragAndDropToTaskbarFix.exe by Dr.MonaLisa:";
							wstring ReloadChangesQuestion = L"The configuration file \"";
							ReloadChangesQuestion = ReloadChangesQuestion + ConfigFile;
							ReloadChangesQuestion = ReloadChangesQuestion + L"\" has been modified by another program.\n\nDo you want to restart \"Windows 11 Drag & Drop to the Taskbar (Fix)\" in order to reload settings?\n\n- Click \"YES\" to reload settings from the file\n\n- Click \"NO\" to keep the current settings\n\n- Click \"CANCEL\" to keep the current settings and to not display this warning again";

							int ReloadChangesConfUtx = MessageBoxW(NULL, ReloadChangesQuestion.c_str(), ReloadChangesTitie.c_str(), MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_TOPMOST | MB_SETFOREGROUND | MB_DEFBUTTON1);
							if (ReloadChangesConfUtx == IDYES) {
								NewFunctionToKill(true);

							}
							else if (ReloadChangesConfUtx == IDNO) {

							}
							else {//Cancel
								ConfigFileChangeTimeMonitorAllowed = false;
							}

						}
					}
					LastSettingsChangeTime = NowSettingsChangeTime;
				}
			}

			CheckedConfigTimeAtLeastOneTime = true;
		}


		//Check if left mouse button is pressed:
		//if (!GetAsyncKeyState(VK_LBUTTON)) {
		//In ver 1.1 we use a new thread
		if(!LeftButtonPressedATM){
			if (CurrentlyLeftMouseButtonIsPressed) {
				ResetTmpVariables();
				CurrentlyLeftMouseButtonIsPressed = false;
			}
		}
		else {
			if (PrintDebugInfo) {
				std::cout << "Left Mouse Button is pressed (SessionID: " << Current_UniqueID_of_the_click << "). Detecting for how long...\n";
			}
			if (!CurrentlyLeftMouseButtonIsPressed) {
				//FirstTimeClickedLeftMouseButton = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
				//In ver 1.1 we use more precise time.
				ResetTmpVariables();
				FirstTimeClickedLeftMouseButton = LastTimeClickedLeftMouseButton;
				Previous_UniqueID_of_the_click = Current_UniqueID_of_the_click;
			}
			else {
				//ver 1.1, so we know if it's the same click as before, without worrying about the thread sleep.
				if (Previous_UniqueID_of_the_click != Current_UniqueID_of_the_click) {
					FirstTimeClickedLeftMouseButton = LastTimeClickedLeftMouseButton;
					Previous_UniqueID_of_the_click = Current_UniqueID_of_the_click;
					if (PrintDebugInfo) {
						std::cout << "Different mouse click unique ID detected: " << Current_UniqueID_of_the_click << ". Resetting...\n";
					}
				}
			}
			SleepPeriodNow = SleepPeriodWhenLeftMouseButtonIsPressedInTheLoopMilliseconds;
			CurrentlyLeftMouseButtonIsPressed = true;
			if ((FirstTimeClickedLeftMouseButton.count() != 0) && (TimeNow.count() >= (FirstTimeClickedLeftMouseButton.count() + HowLongLeftMouseButtonPressedBeforeContinueMilliseconds))) {
				//Continue:

				if (PrintDebugInfo) {
					std::cout << "Left Mouse Button was pressed for > " << HowLongLeftMouseButtonPressedBeforeContinueMilliseconds << " milliseconds...\n";
				}

				if (!DetectedHWNDsForThisMouseClick) {
					DetectedHWNDsForThisMouseClick = true;
					hWndTray = FindWindow(L"Shell_TrayWnd", nullptr);
					if (hWndTray) {
						hWndTrayNotify = FindWindowEx(hWndTray, 0, L"TrayNotifyWnd", nullptr);
					}
					else {
						hWndTrayNotify = 0;
					}
					//HWND hWndSysPager = FindWindowEx(hWndTrayNotify, 0, L"SysPager", nullptr);
					//HWND hWndToolbar = FindWindowEx(hWndSysPager, 0, L"ToolbarWindow32", nullptr);
					if (hWndTray) {
						hWndRebar = FindWindowEx(hWndTray, 0, L"ReBarWindow32", nullptr);
					}
					else {
						hWndRebar = 0;
					}
					if (hWndRebar) {
						hWndMSTaskSwWClass = FindWindowEx(hWndRebar, 0, L"MSTaskSwWClass", nullptr);
					}
					else {
						hWndMSTaskSwWClass = 0;
					}

					//Find a window that should be invisible when app has no active window!
					TaskListThumbnailWnd = FindWindowEx(NULL, 0, L"TaskListThumbnailWnd", nullptr);

					//1.1 fix, actually not needed, it uses hWndTrayNotify HWND.
					/*if (hWndTrayNotify) {
						hWndWindowForShowDesktopArea = FindWindowEx(hWndTrayNotify, 0, L"Windows.UI.Composition.DesktopWindowContentBridge", nullptr);
					}
					else {
						hWndWindowForShowDesktopArea = 0;
					}*/

					std::cout << "Found hWndWindowForShowDesktopArea Window: " << hWndWindowForShowDesktopArea << "\n";

					//HWND hWndMSTaskListWClass = FindWindowEx(hWndMSTaskSwWClass, 0, L"MSTaskListWClass", nullptr);
					if (PrintDebugInfo) {
						std::cout << "Found Taskbar Window: " << hWndMSTaskSwWClass << "\n";
					}
					RECT rect;
					GetWindowRect(hWndMSTaskSwWClass, &rect);
					//std::cout << "Taskbar Window Rect: " << rect.left << ":" << rect.right << ":" << rect.bottom << ":" << rect.top << "\n";
					TaskbarWindowWidth = rect.right - rect.left;
					if (PrintDebugInfo) {
						std::cout << "Taskbar Window Width: " << TaskbarWindowWidth << "\n";
					}

					NumberOfItemsOnTaskbar = TaskbarWindowWidth / DefaultTaskbarIconWidth;
					if (PrintDebugInfo) {
						std::cout << "Number of icons on taskbar: " << NumberOfItemsOnTaskbar << "\n";
					}
					hDesktop = GetDesktopWindow();
					GetWindowRect(hDesktop, &desktop);
					ShowDesktopStartPosition = desktop.right - DefaultShowDesktopButtonWidth;
				}
				
				if (hWndMSTaskSwWClass) {

					//Check if taskbar area is visible, not to continue when playing games, etc.
					//if (IsWindowVisible(hWndMSTaskSwWClass)) {
					//if (IsWindowVisible(hWndTray)) {

					//For some reasons IsWindowVisible doesn't work as intended, so let's workaround it:

					GetCursorPos(&P);
					HWND WindowUnderMouse = WindowFromPoint(P);

					if(WindowUnderMouse == hWndMSTaskSwWClass || WindowUnderMouse == hWndTrayNotify){

						P_Client = P;
						ScreenToClient(hWndMSTaskSwWClass, &P_Client);
		
						MouseClickStartPoint_Client = MouseClickStartPoint;
						ScreenToClient(hWndMSTaskSwWClass, &MouseClickStartPoint_Client);

						//Check if user clicked the left mouse button in the taskbar area, so should not continue:
						//std::cout << "Left Mouse Click Started in:" << MouseClickStartPoint.x << " Y: " << MouseClickStartPoint.y << "\n";

						if (MouseClickStartPoint_Client.x >= 0 && MouseClickStartPoint_Client.y >= 0) {
							if (PrintDebugInfo) {
								std::cout << "Left Mouse Click Started in the taskbar area: X: " << MouseClickStartPoint_Client.x << " Y: " << MouseClickStartPoint_Client.y << ", so skipping.\n";
							}
							//The sleep was missing there causing heavy CPU usage. Fixed in ver 1.1.1
							Sleep(SleepPeriodNow);
							continue;
						}

						/*if (DetectIfFileIsCurrentlyDraggedUsingClipboard && !Detect_if_Clipboard_Has_Dragged_File_Data()) {
							//Check if currently file is being dragged using clipboard
							if (PrintDebugInfo) {
								std::cout << "No dragged file data type in the clipboard, so skipping.\n";
							}
							Sleep(SleepPeriodNow);
							continue;
						}*/

						if (PrintDebugInfo) {
							std::cout << "Client Mouse position. X:" << P_Client.x << " Y: " << P_Client.y << "\n";
						}

						//Check if maybe in the "show desktop" area:
						bool ShowDesktopPositionNow = false;
						if (P_Client.y >= 0 && P.x >= ShowDesktopStartPosition) {
							ShowDesktopPositionNow = true;
						}

						if ((P_Client.x >= 0 && P_Client.y >= 0 && P_Client.x <= TaskbarWindowWidth) || ShowDesktopPositionNow) {

							SleepPeriodNow = SleepPeriodWhenMouseIsOnAppIconInTheLoopMilliseconds;

							int CurrentAppIcon = P_Client.x / DefaultTaskbarIconWidth;
							int CurrentAppIconPlusOne = CurrentAppIcon + 1;

							if (ShowDesktopPositionNow) {
								CurrentAppIcon = 999;
								CurrentAppIconPlusOne = 1000;
							}

							if (PreviousHoveredMouseAppID != CurrentAppIcon) {
								PreviousHoveredMouseAppID = CurrentAppIcon;
								FirstTimeHoveredOverThisAppIcon = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
							}

							//No need for long long it here I guess :D
							int HowLongOverThisIconCount = TimeNow.count() - FirstTimeHoveredOverThisAppIcon.count();

							if (PrintDebugInfo) {
								std::cout << "Currently in the taskbar area! App icon ID: " << CurrentAppIconPlusOne << ". Pressed for milliseconds: " << HowLongOverThisIconCount << "\n";
							}

							if (HowLongOverThisIconCount >= HowLongKeepMouseOverAppIconBeforeRestoringWindowMilliseconds) {
								if ((CurrentAppIcon != LastSimulatedHotkeyPressID || UseTheNewBestMethodEver)) { //UseTheNewBestMethodEver might cause a bug bug
									LastSimulatedHotkeyPressID = CurrentAppIcon;
									if (UseTheNewBestMethodEver) {
										//Finally found a perfect solution:
										if (CurrentAppIcon == 999) {
											if (hWndTray) {
												Simulate_Show_Desktop_Behaviour();
												//LRESULT res = SendMessage(hWndTray, WM_COMMAND, (WPARAM)419, 0);
											}
											else {
												//It should never be executed, because hWndTray couldn't be NULL at higher steps...
												//Show desktop thing:
												keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), 0, 0);
												keybd_event(0x44, MapVirtualKey(0x44, 0), 0, 0);
												Sleep(50);
												keybd_event(0x44, MapVirtualKey(0x44, 0), KEYEVENTF_KEYUP, 0);
												keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), KEYEVENTF_KEYUP, 0);
											}

											if (PrintDebugInfo) {
												std::cout << "Simulating Logo Win + " << CurrentAppIconPlusOne << " key\n";
											}
										}
										else {
											Finally_The_Best_Method_Ever(CurrentAppIcon, NumberOfItemsOnTaskbar);
										}
									}
									else if (CurrentAppIcon <= 9) {
										if (UseTheNewWMHOTKEYMethod && hWndTray) {
											LRESULT SendMessageReturn = SendMessage(hWndTray, WM_HOTKEY, New_WM_HOTKEY_Array_LogoWin_CTRL_Num[CurrentAppIcon].first, New_WM_HOTKEY_Array_LogoWin_CTRL_Num[CurrentAppIcon].second);
											if (PrintDebugInfo) {
												std::cout << "Sending WM_HOTKEY message for Logo Win + CTRL + " << CurrentAppIconPlusOne << " key\n";
											}
										}
										else {
											keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), 0, 0); //Press windows key
											keybd_event(VK_LCONTROL, MapVirtualKey(VK_LCONTROL, 0), 0, 0); //Press CTRL key
											keybd_event(Keyboard_Keys_One_to_Zero[CurrentAppIcon], MapVirtualKey(Keyboard_Keys_One_to_Zero[CurrentAppIcon], 0), 0, 0); //left Press
											Sleep(50);
											keybd_event(Keyboard_Keys_One_to_Zero[CurrentAppIcon], MapVirtualKey(Keyboard_Keys_One_to_Zero[CurrentAppIcon], 0), KEYEVENTF_KEYUP, 0); // left Release
											keybd_event(VK_LCONTROL, MapVirtualKey(VK_LCONTROL, 0), KEYEVENTF_KEYUP, 0); // left Release
											keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), KEYEVENTF_KEYUP, 0); // left Release
											if (PrintDebugInfo) {
												std::cout << "Simulating Logo Win + CTRL + " << CurrentAppIconPlusOne << " key\n";
											}
										}

									}
									else if (CurrentAppIcon == 999) {
										if (hWndTray) {
											Simulate_Show_Desktop_Behaviour();
											//LRESULT res = SendMessage(hWndTray, WM_COMMAND, (WPARAM)419, 0);
										}
										else {
											//It should never be executed, because hWndTray couldn't be NULL at higher steps...
											//Show desktop thing:
											keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), 0, 0);
											keybd_event(0x44, MapVirtualKey(0x44, 0), 0, 0);
											Sleep(50);
											keybd_event(0x44, MapVirtualKey(0x44, 0), KEYEVENTF_KEYUP, 0); 
											keybd_event(VK_LWIN, MapVirtualKey(VK_LWIN, 0), KEYEVENTF_KEYUP, 0);
										}

										if (PrintDebugInfo) {
											std::cout << "Simulating Logo Win + " << CurrentAppIconPlusOne << " key\n";
										}
									}

									#ifndef DONT_INCLUDE_UNUSED_FUNCTIONS_TO_PREVENT_PSEUDO_ANTIVIRUSES_FROM_THROWING_FALSE_POSITIVES
									else if (CurrentAppIcon > 9) {
										if (UseTheNewWorkaroundForButtonsElevenPlus) {
											Experimental_Workaround_for_buttons_Eleven_Plus();
										}
									}
									#endif
									else {
										if (PrintDebugInfo) {
											std::cout << "Unfortunately, can't restore the window because App icon ID is greater than 10 :(\n";
										}
									}
								}
							}
						}
						else {
							SleepPeriodNow = SleepPeriodWhenLeftMouseButtonIsPressedInTheLoopMilliseconds;
						}
					}
				}
			}
		}
		Sleep(SleepPeriodNow);
	}
}
