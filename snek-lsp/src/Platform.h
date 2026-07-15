#pragma once

#include <filesystem>


std::filesystem::path GetExecutablePath();
uint64_t GetTimeNS();
void SleepMS(uint64_t ms);
