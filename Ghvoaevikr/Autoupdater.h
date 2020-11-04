#pragma once
#include <cinttypes>
#include <chrono>

class Autoupdater {
	std::chrono::milliseconds m_lastScanTime;
	uint8_t* find_signature(const wchar_t* szModule, const char* szSignature);
public:
	bool TryStart();
};
