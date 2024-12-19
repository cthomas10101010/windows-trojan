#ifndef ANALYSISEVASION_H
#define ANALYSISEVASION_H

#include <windows.h> // Include Windows headers for DWORD and BOOL

class AntiAnalysis {
public:
    void IntroduceDelay(DWORD milliseconds);
    BOOL CheckSystemUptime(DWORD minUptimeSeconds);
    bool CheckForDebugger();
    bool DetectVirtualMachine();
    bool EvadeSandbox();
    bool checkScreenResolution();
    bool checkProcessCount();
};

#endif // ANALYSISEVASION_H