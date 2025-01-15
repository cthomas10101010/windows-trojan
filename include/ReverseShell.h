#ifndef REVERSE_SHELL_H
#define REVERSE_SHELL_H

void RunShell(const char* C2Server, int C2Port);
void ReconnectToListener(const char* C2Server, int C2Port);
#endif // REVERSE_SHELL_H