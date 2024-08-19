#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <iomanip>

// make std::wstring into std::string
std::string wideToNarrow(const std::wstring& wideStr) {
    if (wideStr.empty()) {
        return {};
    }

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), nullptr, 0, nullptr, nullptr);
    std::string narrowStr(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), static_cast<int>(wideStr.size()), &narrowStr[0], sizeNeeded, nullptr, nullptr);
    return narrowStr;
}

// Get the current timestamp
std::string getCurrentTimestamp() {
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);

    std::ostringstream timestamp;
    timestamp << std::setw(4) << std::setfill('0') << systemTime.wYear << "-"
        << std::setw(2) << std::setfill('0') << systemTime.wMonth << "-"
        << std::setw(2) << std::setfill('0') << systemTime.wDay << " "
        << std::setw(2) << std::setfill('0') << systemTime.wHour << ":"
        << std::setw(2) << std::setfill('0') << systemTime.wMinute << ":"
        << std::setw(2) << std::setfill('0') << systemTime.wSecond << "."
        << std::setw(3) << std::setfill('0') << systemTime.wMilliseconds;
    return timestamp.str();
}

// Kill Roblox processes
void killRobloxProcesses() {
    const std::string logFileName = "log.txt";

    // Create the log file if it doesn't exist
    if (!std::filesystem::exists(logFileName)) {
        std::ofstream createFile(logFileName);
        createFile.close();
    }

    // Open the log file in append mode
    std::ofstream logFile(logFileName, std::ios_base::app);

    // Take a snapshot of all the shit running
    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE) {
        logFile << getCurrentTimestamp() << " INFO [CreateToolhelp32Snapshot] Failed to take process snapshot." << std::endl;
        logFile.close();
        return;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve the FirstP information
    if (!Process32First(processSnapshot, &processEntry)) {
        logFile << getCurrentTimestamp() << " INFO [Process32First] Failed to retrieve process information." << std::endl;
        CloseHandle(processSnapshot);
        logFile.close();
        return;
    }

    do {
        // Case Insensitive conversion or something mayb
        std::wstring processName(processEntry.szExeFile);
        std::transform(processName.begin(), processName.end(), processName.begin(), ::towlower);

        // ooga boog name is roblox ja?
        if (processName.find(L"roblox") != std::wstring::npos) {
            HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
            if (processHandle != nullptr) {
                try {
                    // Try to kill the fucking process
                    if (TerminateProcess(processHandle, 0)) {
                        std::wstring processInfo = processEntry.szExeFile;
                        processInfo += L" (" + std::to_wstring(processEntry.th32ProcessID) + L")";

                        logFile << getCurrentTimestamp() << " INFO [TerminateProcess] Successfully terminated " << wideToNarrow(processInfo) << std::endl;
                    }
                    else {
                        logFile << getCurrentTimestamp() << " ERROR [" << processEntry.th32ProcessID << "] [TerminateProcess] Failed to terminate process." << std::endl;
                    }
                }
                catch (const std::exception& e) {
                    logFile << getCurrentTimestamp() << " ERROR [" << processEntry.th32ProcessID << "] [TerminateProcess] Exception: " << e.what() << std::endl;
                }

                CloseHandle(processHandle);
            }
            else {
                logFile << getCurrentTimestamp() << " ERROR [" << processEntry.th32ProcessID << "] [OpenProcess] Failed to open process." << std::endl;
            }
        }
    } while (Process32Next(processSnapshot, &processEntry)); // SEARCH NIGGA!

    // Close the snapshot handle
    CloseHandle(processSnapshot);
    logFile.close();
}

int main() {
    killRobloxProcesses();
    return 0;
}
