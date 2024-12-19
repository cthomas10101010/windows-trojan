#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <windows.h>
#include <thread>
#include <iostream>

// Function to run stealth tasks in the background
void runStealthTasks();

// Function to launch stealth tasks as a separate detached process
void launchStealthProcess();

#endif // MAIN_H
