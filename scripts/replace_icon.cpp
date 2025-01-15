#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

#pragma pack(push, 2)
struct ICONDIR {
    WORD idReserved;
    WORD idType;
    WORD idCount;
    struct ICONDIRENTRY {
        BYTE  bWidth;
        BYTE  bHeight;
        BYTE  bColorCount;
        BYTE  bReserved;
        WORD  wPlanes;
        WORD  wBitCount;
        DWORD dwBytesInRes;
        WORD  nID; // references the icon resource
    } idEntries[1];
};
#pragma pack(pop)

bool ReplaceIcon(const wchar_t* exePath, const wchar_t* iconPath) {
    std::wcout << L"[INFO] Attempting to replace icon in: " << exePath << std::endl;
    std::wcout << L"[INFO] Using icon file: " << iconPath << std::endl;

    // Open the EXE for resource updating
    HANDLE hResource = BeginUpdateResourceW(exePath, FALSE);
    if (!hResource) {
        std::cerr << "[ERROR] BeginUpdateResourceW failed with error code: " 
                  << GetLastError() << std::endl;
        return false;
    } else {
        std::cout << "[DEBUG] Successfully opened EXE for resource updating." << std::endl;
    }

    // Load the icon file
    std::ifstream iconFile(iconPath, std::ios::binary | std::ios::ate);
    if (!iconFile.is_open()) {
        std::cerr << "[ERROR] Failed to open icon file: " << iconPath << std::endl;
        EndUpdateResourceW(hResource, TRUE);  // discard changes
        return false;
    } else {
        std::cout << "[DEBUG] Icon file opened successfully." << std::endl;
    }

    // Read the entire icon file into memory
    size_t fileSize = (size_t)iconFile.tellg();
    if (fileSize == 0) {
        std::cerr << "[ERROR] Icon file is empty or unreadable." << std::endl;
        iconFile.close();
        EndUpdateResourceW(hResource, TRUE);  // discard changes
        return false;
    } else {
        std::cout << "[DEBUG] Icon file size: " << fileSize << " bytes." << std::endl;
    }

    iconFile.seekg(0, std::ios::beg);
    std::vector<BYTE> iconData(fileSize);
    iconFile.read(reinterpret_cast<char*>(iconData.data()), fileSize);
    iconFile.close();
    std::cout << "[DEBUG] Icon file data read into memory." << std::endl;

    // Update the RT_ICON resource
    // Note: RT_ICON = MAKEINTRESOURCEW(3). The numeric ID is arbitrary (we use 1 here).
    // If your icon file actually contains multiple icons (different sizes / bit-depths),
    // you should parse and add them individually. This example just inserts the whole file
    // as a single resource.
    if (!UpdateResourceW(
            hResource,
            MAKEINTRESOURCEW(3),               // RT_ICON
            MAKEINTRESOURCEW(1),               // ID for the icon resource
            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
            iconData.data(),
            (DWORD)fileSize))
    {
        std::cerr << "[ERROR] UpdateResourceW for RT_ICON (ID=1) failed with error code: "
                  << GetLastError() << std::endl;
        EndUpdateResourceW(hResource, TRUE);  // discard changes
        return false;
    } else {
        std::cout << "[DEBUG] RT_ICON resource updated successfully." << std::endl;
    }

    // Prepare the icon group data
    ICONDIR groupIcon;
    groupIcon.idReserved = 0;
    groupIcon.idType = 1;      // 1 for icons
    groupIcon.idCount = 1;     // number of icons in the group

    // We set fields for a single icon; 
    // actual .ico parsing would let you map multiple images if needed.
    groupIcon.idEntries[0].bWidth      = 255;  // 0 or 255 can signify 'auto-detect' for .ico
    groupIcon.idEntries[0].bHeight     = 255;
    groupIcon.idEntries[0].bColorCount = 0;
    groupIcon.idEntries[0].bReserved   = 0;
    groupIcon.idEntries[0].wPlanes     = 1;
    groupIcon.idEntries[0].wBitCount   = 32;
    groupIcon.idEntries[0].dwBytesInRes = (DWORD)fileSize;
    groupIcon.idEntries[0].nID         = 1;    // must match the resource ID used above

    std::cout << "[DEBUG] Icon group struct prepared. size: " << sizeof(groupIcon) 
              << " bytes." << std::endl;

    // Update RT_GROUP_ICON resource
    // Note: RT_GROUP_ICON = MAKEINTRESOURCEW(14). 
    // We choose 1 as the resource ID for the icon group.
    if (!UpdateResourceW(
            hResource,
            MAKEINTRESOURCEW(14),              // RT_GROUP_ICON
            MAKEINTRESOURCEW(1),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
            &groupIcon,
            sizeof(groupIcon)))
    {
        std::cerr << "[ERROR] UpdateResourceW for RT_GROUP_ICON (ID=1) failed with error code: "
                  << GetLastError() << std::endl;
        EndUpdateResourceW(hResource, TRUE);  // discard changes
        return false;
    } else {
        std::cout << "[DEBUG] RT_GROUP_ICON resource updated successfully." << std::endl;
    }

    // Commit the changes
    if (!EndUpdateResourceW(hResource, FALSE)) {
        std::cerr << "[ERROR] EndUpdateResourceW failed with error code: "
                  << GetLastError() << std::endl;
        return false;
    } else {
        std::cout << "[DEBUG] Resource update committed successfully." << std::endl;
    }

    std::cout << "[SUCCESS] Icon has been replaced in the target EXE." << std::endl;
    return true;
}

int main() {
    const wchar_t* exePath  = L"C:\\Users\\alexa\\windows-trojan\\Document10.exe";
    const wchar_t* iconPath = L"C:\\Users\\alexa\\windows-trojan\\favicon.ico";

    bool result = ReplaceIcon(exePath, iconPath);
    if (result) {
        std::cout << "[INFO] Icon replacement succeeded." << std::endl;
    } else {
        std::cerr << "[ERROR] Icon replacement failed." << std::endl;
    }
    return (result ? 0 : 1);
}
