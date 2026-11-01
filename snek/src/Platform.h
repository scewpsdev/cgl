#pragma once

#include <filesystem>


std::filesystem::path GetExecutablePath();
uint64_t GetTimeNS();
void SleepMS(uint64_t ms);

#ifndef _WIN32
void __debugbreak();
#endif
