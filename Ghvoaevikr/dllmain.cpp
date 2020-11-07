#include "Engine.h"
#include "Hooks.h"

#include "Autoupdater.h"
#include "Orbwalker.h"
#include "Evader.h"
#include "ZoomHack.h"
#include "Prediction.h"
#include "CycleManager.h"
#include "Keyboard.h"
#include "SpellPrediction.h"
#include "ESpellSlot.h"
#include "Script.h"
#include "ScriptUtils.h"
#include "Utils.h"
#include "Hvpp.h"

//#include "Detour.h"

#include "Debug.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mutex>
#include <algorithm>

#include "ImRender.h"
#ifndef NO_IMGUI
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\dx9\imgui_impl_dx9.h"
#include "imgui\win32\imgui_impl_win32.h"
#endif

#ifdef _DEBUG
#include "CConsole.h"
#endif

using namespace std;

CObjectManager* ObjManager;
CFunctions Functions;

Autoupdater autoUpdater;
Prediction prediction;
COrbWalker orbWalker;
CEvader evader;
SpellPrediction spellPrediction;
IScript* championScript;

HMODULE g_module = nullptr;
HWND g_hwnd = nullptr;
WNDPROC g_wndproc = nullptr;
#ifndef NO_IMGUI
bool g_menu_opened = false;
#endif
bool g_orbwalker = true;
bool g_range = true;
bool g_2range_objmanager = false;
bool g_champ_info = true;
bool g_turret_range = true;
bool g_auto_evade = true;
bool g_zoom_hack = false;
bool g_spell_prediction = true;

bool g_bInit = false;

DWORD g_BaseAddr;
#ifndef NO_IMGUI
IDirect3DDevice9* myDevice;
#endif

typedef HRESULT(WINAPI* Prototype_Present)(LPDIRECT3DDEVICE9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*);
HRESULT WINAPI Hooked_Present(LPDIRECT3DDEVICE9 Device, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion);
Hvpp presentHvpp;

LRESULT WINAPI WndProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

void __cdecl spoofedDrawCircle(Vector* position, float range, int* color, int a4, float a5, int a6, float alpha);

HRESULT WINAPI Hooked_Present(LPDIRECT3DDEVICE9 Device, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion)
{
	if (g_bInit == false) {
		g_BaseAddr = (DWORD)GetModuleHandle(NULL);

		if (Engine::GetGameTime() < 1.0f || !Engine::GetLocalObject())
			goto exit;

		if (!autoUpdater.TryStart())
			goto exit;

#ifdef _DEBUG
		//CConsole console;
#endif

		g_hwnd = FindWindowA(nullptr, "League of Legends (TM) Client");
		g_wndproc = WNDPROC(SetWindowLongA(g_hwnd, GWL_WNDPROC, LONG_PTR(WndProc)));

		ObjManager = (CObjectManager*)(*(DWORD*)(g_BaseAddr + offsets::global::oObjManager));

		Functions.PrintChat = (Typedefs::fnPrintChat)(g_BaseAddr + offsets::functions::oPrintChat);
		Functions.IsTargetable = (Typedefs::fnIsTargetable)(g_BaseAddr + offsets::functions::oIsTargetable);
		Functions.IsAlive = (Typedefs::fnIsAlive)(g_BaseAddr + offsets::functions::oIsAlive);

		Functions.IsMinion = (Typedefs::fnIsMinion)(g_BaseAddr + offsets::functions::oIsMinion);
		Functions.IsTurret = (Typedefs::fnIsTurret)(g_BaseAddr + offsets::functions::oIsTurret);
		Functions.IsHero = (Typedefs::fnIsHero)(g_BaseAddr + offsets::functions::oIsHero);
		Functions.IsMissile = (Typedefs::fnIsMissile)(g_BaseAddr + offsets::functions::oIsMissile);
		Functions.IsNexus = (Typedefs::fnIsNexus)(g_BaseAddr + offsets::functions::oIsNexus);
		Functions.IsInhibitor = (Typedefs::fnIsInhibitor)(g_BaseAddr + offsets::functions::oIsInhib);
		Functions.IsTroyEnt = (Typedefs::fnIsTroyEnt)(g_BaseAddr + offsets::functions::oIsTroy);

		Functions.CastSpell = (Typedefs::fnCastSpell)(g_BaseAddr + offsets::functions::oCastSpell);
		Functions.IssueOrder = (Typedefs::fnIssueOrder)(g_BaseAddr + offsets::functions::oIssueOrder);
		Functions.DrawCircle = (Typedefs::fnDrawCircle)spoofedDrawCircle;
		Functions.WorldToScreen = (Typedefs::fnWorldToScreen)(g_BaseAddr + (DWORD)offsets::global::oWorldToScreen);

		Functions.GetAttackCastDelay = (Typedefs::fnGetAttackCastDelay)(g_BaseAddr + offsets::functions::oGetAttackCastDelay);
		Functions.GetAttackDelay = (Typedefs::fnGetAttackDelay)(g_BaseAddr + offsets::functions::oGetAttackDelay);

		Functions.GetPing = (Typedefs::fnGetPing)(g_BaseAddr + offsets::functions::oGetPing);

#ifndef NO_IMGUI
		Original_Reset = (Prototype_Reset)DetourFunc((PBYTE)GetDeviceAddress(16), (PBYTE)Hooked_Reset, 5);

		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX9_Shutdown();

		ImGui::DestroyContext(ImGui::GetCurrentContext());
#endif

		championScript = ScriptUtils::GetScriptByChampionName(Engine::GetLocalObject()->GetChampionName());

		//Engine::PrintChat("[ SpaghettiHack ]");
		//Engine::PrintChat("[ Credits: Kmsmym ]");
		//Engine::PrintChat("[ Unknowncheats.me ]");

		g_bInit = true;
	}

#ifndef NO_IMGUI
	myDevice = Device;
	DO_ONCE([&]()
		{
			render.init(Device);
		});

	ImGui::CreateContext();
	render.begin_draw();

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
						ImGui::Checkbox("Orbwalker", &g_orbwalker);
						ImGui::Checkbox("Auto evade", &g_auto_evade);
						ImGui::Checkbox("Zoom hack", &g_zoom_hack);
						ImGui::Checkbox("Spell prediction", &g_spell_prediction);
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
#endif

	CycleManager::NewCycle();

	auto localObj = Engine::GetLocalObject();

	//evader
	if (g_auto_evade == true) {
		if (localObj && localObj->IsAlive() && Engine::IsLeagueInForeground()) {
			evader.drawEvent();
		}
	}

	//orbwalker
	if (g_orbwalker == true) {
		if (localObj && localObj->IsAlive()) {
			if (IsKeyDown(VK_SPACE) && Engine::IsLeagueInForeground()) {
				orbWalker.drawEvent();
			}
		}
	}

	//me Range
	//if (g_range == true) {
	//	if (localObj && localObj->IsAlive()) { 
	//		auto color = createRGB(0, 255, 0);
	//		auto localPos = Engine::GetLocalObject()->GetPos();
	//		Functions.DrawCircle(&localPos, Engine::GetLocalObject()->GetTrueAttackRange(), &color, 0, 0.0f, 0, 0.5f);
	//	}
	//}

	/*for (auto pObject : CycleManager::GetObjects())
	{
		//draw range all hero
		//if (g_2range_objmanager == true) {
		//	if (pObject->IsHero() && pObject->IsAlive() && pObject->IsTargetable() && pObject->IsVisible())
		//	{
		//		auto color = createRGB(255, 0, 0);
		//		auto objPos = pObject->GetPos();
		//		Functions.DrawCircle(&objPos, pObject->GetAttackRange(), &color, 0, 0.0f, 0, 0.5f);
		//	}
		//}

#ifndef NO_IMGUI
		//champion info demonstration
		if (g_champ_info == true) {
			if (pObject->IsHero() && pObject->IsAlive() && pObject->IsTargetable() && pObject->IsVisible())
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
#endif

		//turret range
		//if (g_turret_range == true) {
		//	if (pObject->IsTurret() && pObject->IsEnemyTo(localObj)) {
		//		auto turretRange = pObject->GetStatOwner()->GetStatTotal(EStatType::AttackRange, EStatOutputType::BASE)
		//			+ pObject->GetStatOwner()->GetStatTotal(EStatType::AttackRange, EStatOutputType::BONUS)
		//			+ pObject->GetBoundingRadius();
		//		if (pObject->GetDistanceToMe() < (turretRange + 300.f)) {
		//			auto color = createRGB(255, 255, 255);
		//			auto turretPos = pObject->GetPos();
		//			Functions.DrawCircle(&turretPos, turretRange, &color, 0, 0.0f, 0, 0.5f);
		//		}
		//	}
		//}
	}*/

#ifndef NO_IMGUI
	render.end_draw();
#endif

	exit:
	return ((Prototype_Present)(presentHvpp.GetOriginalAddress()))(Device, pSrcRect, pDestRect, hDestWindow, pDirtyRegion);
}

#ifndef NO_IMGUI
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
#endif

#pragma warning(push)
#pragma warning(disable : 4740)
void __cdecl spoofedDrawCircle(Vector* position, float range, int* color, int a4, float a5, int a6, float alpha) {
	DWORD retnInstruction = g_BaseAddr + offsets::global::oDrawCircleRetAddr;
	DWORD originalAddress = g_BaseAddr + offsets::functions::oDrawCircle;
	__asm {
		push retnHere
		push alpha
		push a6
		push a5
		push a4
		push color
		push range
		push position
		push retnInstruction
		jmp originalAddress
	retnHere:
	}
}
#pragma warning(pop)

#ifndef NO_IMGUI
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
#endif

LRESULT WINAPI WndProc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	switch (u_msg)
	{
#ifndef NO_IMGUI
	case WM_KEYDOWN: {
		switch (w_param) {
		case VK_END: {
			g_menu_opened = !g_menu_opened;
			break;
		}
		default:
			break;
		};
		break;
	}
#endif
	case WM_MOUSEWHEEL: {
		//Zoom hack
		if (g_zoom_hack) { // TODO: check if game menu opened
			auto wheelDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(w_param));
			ZoomHack::ChangeMaximumZoom(-wheelDelta);
		}
		break;
	}
	default:
		break;
	}

#ifndef NO_IMGUI
	if (g_menu_opened && ImGui_ImplDX9_WndProcHandler(hwnd, u_msg, w_param, l_param)) {
		return true;
	}
#endif

	return CallWindowProcA(g_wndproc, hwnd, u_msg, w_param, l_param);
}

void DllMainAttach(HMODULE hModule) {
	g_module = hModule;
	debug::init();

	if (!ColdHook_Service::ServiceGlobalInit(nullptr)) {
		debug::flog("Could not initialize ColdHook global service\n");
		return;
	}
	presentHvpp.Init((PVOID)GetD3D9VTableFunction(17), (PVOID)Hooked_Present);
	presentHvpp.Hook();
	presentHvpp.Hide();
}

void DllMainDetach() {
	presentHvpp.GlobalUnhide();
	ColdHook_Service::ServiceGlobalShutDown(true, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID)
{
	if (hModule != nullptr)
		DisableThreadLibraryCalls(hModule);

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DllMainAttach(hModule);
		return TRUE;
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		DllMainDetach();
		return TRUE;
	}

	return FALSE;
}