#include "Autokey.h"
#include "Vector.h"
#include "Hooks.h"

#include "Debug.h"

#include <Windows.h>
#include <ctime>
#include <thread>
#include <chrono>

void Autokey::ResetMouse(int x, int y)
{
	double fScreenWidth = static_cast<double>(GetSystemMetrics(SM_CXSCREEN)) - 1.0;
	double fScreenHeight = static_cast<double>(GetSystemMetrics(SM_CYSCREEN)) - 1.0;
	double fx = x * (65535.0f / fScreenWidth);
	double fy = y * (65535.0f / fScreenHeight);
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = static_cast<LONG>(fx);
	Input.mi.dy = static_cast<LONG>(fy);
	SendInput(1, &Input, sizeof(INPUT));
	ZeroMemory(&Input, sizeof(INPUT));
}

UINT Autokey::PressKey(WORD scanCode)
{
	INPUT input[1] = { 0 };
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0;
	input[0].ki.wScan = scanCode;
	input[0].ki.dwFlags = KEYEVENTF_SCANCODE;

	UINT ret = SendInput(1, input, sizeof(INPUT));

	return ret;
}

UINT Autokey::ReleaseKey(WORD scanCode)
{
	INPUT input[1] = { 0 };
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0;
	input[0].ki.wScan = scanCode;
	input[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

	UINT ret = SendInput(1, input, sizeof(INPUT));

	return ret;
}

void Autokey::ClickThere(Vector pos, int returnAfterMs)
{
	POINT mousePos;
	GetCursorPos(&mousePos);

	MoveMouse(pos);
	Click();

	std::this_thread::sleep_for(std::chrono::milliseconds(returnAfterMs));

	ResetMouse(mousePos.x, mousePos.y);
}

void Autokey::Click()
{
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1, &Input, sizeof(INPUT));
	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1, &Input, sizeof(INPUT));
}

void Autokey::MoveMouse(Vector pos)
{
	Vector pos_w2s;
	Functions.WorldToScreen(&pos, &pos_w2s);

	double fScreenWidth = static_cast<double>(GetSystemMetrics(SM_CXSCREEN)) - 1.0;
	double fScreenHeight = static_cast<double>(GetSystemMetrics(SM_CYSCREEN)) - 1.0;
	double fx = pos_w2s.X * (65535.0f / fScreenWidth);
	double fy = pos_w2s.Y * (65535.0f / fScreenHeight);
	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = static_cast<LONG>(fx);
	Input.mi.dy = static_cast<LONG>(fy);
	SendInput(1, &Input, sizeof(INPUT));
	ZeroMemory(&Input, sizeof(INPUT));
}
