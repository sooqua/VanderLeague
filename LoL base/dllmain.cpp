#include "stdafx.h"
#include "Engine.h"
#include "Hooks.h"
#include "detours.h"
#include "ImRender.hpp"

#include "Autoupdater.h"
#include "Orbwalker.h"
#include "Evader.h"
#include "Prediction.h"
#include "CycleManager.h"
#include "ESpellSlot.h"

#include "Debug.h"

#include <Windows.h>
#include <mutex>

#ifdef _DEBUG
#include "CConsole.h"
#endif

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\dx9\imgui_impl_dx9.h"
#include "imgui\win32\imgui_impl_win32.h"

#pragma comment(lib, "detours.lib")
using namespace std;
#define DO_ONCE(todo) do { \
   static std::once_flag _flag ;\
   std::call_once(_flag, todo); \
} while (false)

CObjectManager* ObjManager;
CFunctions Functions;

Autoupdater autoUpdater;
Prediction prediction;
COrbWalker orbWalker;
CEvader evader;

HMODULE g_module = nullptr;
HWND g_hwnd = nullptr;
WNDPROC g_wndproc = nullptr;
bool g_menu_opened = false;
bool g_range = false;
bool g_unload = false;
bool g_2range_objmanager = false;
bool g_champ_info = true;
bool g_turret_range = true;
bool g_auto_evade = true;
bool OnStartMessage = false;

bool g_interface = false;
IDirect3DDevice9* myDevice;
clock_t lastmove = NULL;
using namespace std;
typedef HRESULT(WINAPI* Prototype_Present)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
Prototype_Present Original_Present;


HRESULT WINAPI Hooked_Present(LPDIRECT3DDEVICE9 Device, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion)
{
	myDevice = Device;
	DO_ONCE([&]()
		{
			render.init(Device);
		});

	ImGui::CreateContext();
	render.begin_draw();

	if (OnStartMessage == false) {
		Engine::PrintChat("[ SpaghettiHack ]");
		Engine::PrintChat("[ Credits: Kmsmym ]");
		Engine::PrintChat("[ Unknowncheats.me ]");
		OnStartMessage = true;
	}

	if (ImGui_ImplWin32_Init(g_hwnd))
	{
		if (ImGui_ImplDX9_Init(Device))
		{
			if (g_menu_opened)
			{
				ImGui::Begin("kmsmym::unknowncheats.me", &g_menu_opened, ImGuiWindowFlags_NoSavedSettings);
				{
					ImGui::BeginChild("##child", ImVec2(450.0f, 450.0f), false, ImGuiWindowFlags_NoSavedSettings);
					{
						ImGui::Checkbox("Auto evade", &g_auto_evade);
						ImGui::Checkbox("My range", &g_range);
						ImGui::Checkbox("All hero range", &g_2range_objmanager);
						ImGui::Checkbox("Text champ info", &g_champ_info);
						ImGui::Checkbox("Turret range", &g_turret_range);
					}
					ImGui::EndChild();
				}
				ImGui::End();
			}
		}
	}

	CycleManager::NewCycle();

	auto localObj = Engine::GetLocalObject();

	bool noAction = false;

	//evader
	if (g_auto_evade == true) {
		if (localObj && localObj->IsAlive()) {
			noAction = evader.drawEvent();
		}
	}

	//orbwalker
	if (!noAction && localObj && localObj->IsAlive()) {
		if ((GetAsyncKeyState(0x58) & (1 << 15)) != 0) {
			orbWalker.drawEvent();
		}
	}

	//me Range
	if (g_range == true) {
		if (localObj && localObj->IsAlive()) {
			auto color = createRGB(0, 255, 0);
			Functions.DrawCircle(&Engine::GetLocalObject()->GetPos(), Engine::GetLocalObject()->GetAttackRange() + Engine::GetLocalObject()->GetBoundingRadius(), &color, 0, 0.0f, 0, 0.5f);
		}
	}

	for (auto pObject : CycleManager::GetObjects())
	{
		//draw range all hero
		if (g_2range_objmanager == true) {
			if (pObject->IsHero())
			{
				auto color = createRGB(255, 0, 0);
				Functions.DrawCircle(&pObject->GetPos(), pObject->GetAttackRange(), &color, 0, 0.0f, 0, 0.5f);
			}
		}

		//champion info demonstration
		if (g_champ_info == true) {
			if (pObject->IsHero())
			{
				Vector obj_pos = pObject->GetPos();
				Vector objpos_w2s;
				Functions.WorldToScreen(&obj_pos, &objpos_w2s);

				auto objSpellBook = pObject->GetSpellBook();

				auto spellQ = objSpellBook->GetSpellByID(static_cast<int>(ESpellSlot::Q));
				auto spellW = objSpellBook->GetSpellByID(static_cast<int>(ESpellSlot::W));
				auto spellE = objSpellBook->GetSpellByID(static_cast<int>(ESpellSlot::E));
				auto spellR = objSpellBook->GetSpellByID(static_cast<int>(ESpellSlot::R));

				//render.draw_text(objpos_w2s.X, objpos_w2s.Y + 15, pObject->GetChampionName(), true, ImColor(255, 0, 0, 255));
				render.draw_text(objpos_w2s.X, objpos_w2s.Y + 30, "[Q]", true, spellQ->IsSpellReady() ? ImColor(0, 255, 0, 255) : ImColor(255, 255, 255, 255));
				render.draw_text(objpos_w2s.X + 20, objpos_w2s.Y + 30, "[W]", true, spellW->IsSpellReady() ? ImColor(0, 255, 0, 255) : ImColor(255, 255, 255, 255));
				render.draw_text(objpos_w2s.X + 40, objpos_w2s.Y + 30, "[E]", true, spellE->IsSpellReady() ? ImColor(0, 255, 0, 255) : ImColor(255, 255, 255, 255));
				render.draw_text(objpos_w2s.X + 60, objpos_w2s.Y + 30, "[R]", true, spellR->IsSpellReady() ? ImColor(0, 255, 0, 255) : ImColor(255, 255, 255, 255));
			}
		}

		//turret range
		if (g_turret_range == true) {
			if (pObject->IsTurret() && pObject->IsEnemyTo(localObj)) {
				static auto turretRange = 850.f;
				if (pObject->GetDistanceToMe() < (turretRange + 300.f)) {
					auto color = createRGB(255, 255, 255);
					Functions.DrawCircle(&pObject->GetPos(), turretRange, &color, 0, 0.0f, 0, 0.5f);
				}
			}
		}
	}

	render.end_draw();
	return Original_Present(Device, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}

typedef HRESULT(WINAPI* Prototype_Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
Prototype_Reset Original_Reset;

HRESULT WINAPI Hooked_Reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	HRESULT result = Original_Reset(pDevice, pPresentationParameters);

	if (result >= 0)
		ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}

DWORD FindDevice(DWORD Len)
{
	DWORD dwObjBase = 0;

	dwObjBase = (DWORD)LoadLibrary(TEXT("d3d9.dll"));
	while (dwObjBase++ < dwObjBase + Len)
	{
		if ((*(WORD*)(dwObjBase + 0x00)) == 0x06C7
			&& (*(WORD*)(dwObjBase + 0x06)) == 0x8689
			&& (*(WORD*)(dwObjBase + 0x0C)) == 0x8689
			) {
			dwObjBase += 2; break;
		}
	}
	return(dwObjBase);
}

DWORD GetDeviceAddress(int VTableIndex)
{
	PDWORD VTable;
	*(DWORD*)&VTable = *(DWORD*)FindDevice(0x128000);
	return VTable[VTableIndex];
}

LRESULT WINAPI WndProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);
typedef long(__stdcall* tEndScene)(LPDIRECT3DDEVICE9);
void __stdcall Start() {
	debug::init();
	autoUpdater.Start();

	while (Engine::GetGameTime() < 1.0f || !Engine::GetLocalObject())
		Sleep(1);

#ifdef _DEBUG
	//CConsole console;
#endif

	g_hwnd = FindWindowA(nullptr, "League of Legends (TM) Client");
	g_wndproc = WNDPROC(SetWindowLongA(g_hwnd, GWL_WNDPROC, LONG_PTR(WndProc)));

	ObjManager = (CObjectManager*)(*(DWORD*)(baseAddr + offsets::global::oObjManager));

	Functions.PrintChat = (Typedefs::fnPrintChat)(baseAddr + offsets::functions::oPrintChat);
	Functions.IsTargetable = (Typedefs::fnIsTargetable)(baseAddr + offsets::functions::oIsTargetable);
	Functions.IsAlive = (Typedefs::fnIsAlive)(baseAddr + offsets::functions::oIsAlive);

	Functions.IsMinion = (Typedefs::fnIsMinion)(baseAddr + offsets::functions::oIsMinion);
	Functions.IsTurret = (Typedefs::fnIsTurret)(baseAddr + offsets::functions::oIsTurret);
	Functions.IsHero = (Typedefs::fnIsHero)(baseAddr + offsets::functions::oIsHero);
	Functions.IsMissile = (Typedefs::fnIsMissile)(baseAddr + offsets::functions::oIsMissile);
	Functions.IsNexus = (Typedefs::fnIsNexus)(baseAddr + offsets::functions::oIsNexus);
	Functions.IsInhibitor = (Typedefs::fnIsInhibitor)(baseAddr + offsets::functions::oIsInhib);
	Functions.IsTroyEnt = (Typedefs::fnIsTroyEnt)(baseAddr + offsets::functions::oIsTroy);

	//Functions.CastSpell = (Typedefs::fnCastSpell)((DWORD)GetModuleHandle(NULL) + oCastSpell);
	Functions.IssueOrder = (Typedefs::fnIssueOrder)((DWORD)GetModuleHandle(NULL) + offsets::functions::oIssueOrder);
	Functions.DrawCircle = (Typedefs::fnDrawCircle)((DWORD)GetModuleHandle(NULL) + offsets::functions::oDrawCircle);
	Functions.WorldToScreen = (Typedefs::fnWorldToScreen)(baseAddr + (DWORD)offsets::global::oWorldToScreen);

	Functions.GetAttackCastDelay = (Typedefs::fnGetAttackCastDelay)((DWORD)GetModuleHandle(NULL) + offsets::functions::oGetAttackCastDelay);
	Functions.GetAttackDelay = (Typedefs::fnGetAttackDelay)((DWORD)GetModuleHandle(NULL) + offsets::functions::oGetAttackDelay);

	Original_Present = (Prototype_Present)DetourFunction((PBYTE)GetDeviceAddress(17), (PBYTE)Hooked_Present);
	Original_Reset = (Prototype_Reset)DetourFunction((PBYTE)GetDeviceAddress(16), (PBYTE)Hooked_Reset);

	while (!g_unload)
		Sleep(1000);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1));

	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX9_Shutdown();

	ImGui::DestroyContext(ImGui::GetCurrentContext());

	debug::cleanUp();

	FreeLibraryAndExitThread(g_module, 0);
}

LRESULT ImGui_ImplDX9_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto& io = ImGui::GetIO();

	switch (msg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		io.MouseDown[0] = true;
		return true;
	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		return true;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		io.MouseDown[1] = true;
		return true;
	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		return true;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		io.MouseDown[2] = true;
		return true;
	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		return true;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = true;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = true;
		return true;
	case WM_XBUTTONUP:
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON1) == MK_XBUTTON1)
			io.MouseDown[3] = false;
		else if ((GET_KEYSTATE_WPARAM(wParam) & MK_XBUTTON2) == MK_XBUTTON2)
			io.MouseDown[4] = false;
		return true;
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
		return true;
	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);
		return true;
	case WM_KEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		return true;
	case WM_KEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return true;
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000)
			io.AddInputCharacter((unsigned short)wParam);
		return true;
	}

	return 0;
}

LRESULT WINAPI WndProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	switch (u_msg)
	{
	case WM_KEYDOWN:
		if (w_param == VK_END) /* твоя кнопка тут */
			g_menu_opened = !g_menu_opened;
		break;
	default:
		break;
	}

	if (g_menu_opened && ImGui_ImplDX9_WndProcHandler(hwnd, u_msg, w_param, l_param))
		return true;

	return CallWindowProcA(g_wndproc, hwnd, u_msg, w_param, l_param);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (hModule != nullptr)
		DisableThreadLibraryCalls(hModule);

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		g_module = hModule;
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Start, 0, 0, 0);
		return TRUE;
	}

	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		g_unload = true;
		return TRUE;
	}

	return FALSE;
}