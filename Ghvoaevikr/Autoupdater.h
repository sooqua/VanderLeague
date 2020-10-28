#pragma once
#include <cinttypes>

class Autoupdater {
	uint8_t* find_signature(const wchar_t* szModule, const char* szSignature);
public:
	void Start();
};
