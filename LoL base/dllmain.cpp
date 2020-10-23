#include "stdafx.h"
#include "Windows.h"
#include "Engine.h"
#include "Hooks.h"
#include "detours.h"
#include "ImRender.hpp"
#include "Autoupdater.h"
#include "Orbwalker.h"

#include "Debug.h"

#include <mutex>
#include <algorithm>
#include <vector>

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
COrbWalker orbWalker;

HMODULE g_module = nullptr;
HWND g_hwnd = nullptr;
WNDPROC g_wndproc = nullptr;
bool g_menu_opened = false;
bool g_range = false;
bool g_unload = false;
bool g_2range_objmanager = false;
bool g_champ_info = false;
bool g_move_to_mouse = false;
bool g_w2s_line = false;
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
	render.begin_draw();//begin for draw rende.drawline.... and etc


	char Get_Healthe[100];		
	sprintf_s(Get_Healthe, "%f", Engine::GetLocalObject()->GetHealth());
	if (OnStartMessage == false) {
		Engine::PrintChat("///////  Kmsmym update lolbase");
		Engine::PrintChat("///////  Unknowncheats.me");
		Engine::PrintChat("///////  My health");
		Engine::PrintChat("///////////////////////////////	 ");
		Engine::PrintChat(Get_Healthe);
		Engine::PrintChat("///////////////////////////////	 ");
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
						ImGui::Checkbox("My range demostration", &g_range);
						ImGui::Checkbox("All hero range demostration", &g_2range_objmanager);
						ImGui::Checkbox("Move to mouse demostration", &g_move_to_mouse);
						ImGui::Checkbox("W2S/Line demostration", &g_w2s_line);
						ImGui::Checkbox("Text champ info demostration", &g_champ_info);
					}
					ImGui::EndChild();
				}
				ImGui::End();
			}
		}
	}

	//Below are just examples, for ease of understanding, some are placed in a separate cycle. Do not repeat this. Do one cycle to get objects.

	//orbwalker
	if ((GetAsyncKeyState(VK_MBUTTON) & (1 << 15)) != 0) {

		//orbWalker.drawEvent(); 

		CObject holzer;
		auto obj = holzer.GetFirstObject();
		while (obj)
		{
			if (obj->IsMissile())
			{
				auto objCaster = Engine::GetObjectByID(obj->GetMissileSourceIndex());

				if (objCaster->IsHero() && objCaster->IsEnemyTo(Engine::GetLocalObject()) && !stristr(obj->GetName(), "basic")) {
					Vector start_pos = obj->GetMissileStartPos();
					Vector start_pos_w2s;
					Functions.WorldToScreen(&start_pos, &start_pos_w2s);
					Vector end_pos = obj->GetMissileEndPos();
					Vector end_pos_w2s;
					Functions.WorldToScreen(&end_pos, &end_pos_w2s);
					render.draw_line(start_pos_w2s.X, start_pos_w2s.Y, end_pos_w2s.X, end_pos_w2s.Y, ImColor(255, 255, 255), 5.0f);

					Vector direction = end_pos - start_pos;

					auto localObjPos = Engine::GetLocalObject()->GetPos();
					std::vector<Vector> points;
					points.push_back(start_pos + Vector(direction.Z * -1.0f, direction.Y, direction.X * 1.0f));
					points.push_back(start_pos + Vector(direction.Z * 1.0f, direction.Y, direction.X * -1.0f));
					points.push_back(end_pos + Vector(direction.Z * -1.0f, direction.Y, direction.X * 1.0f));
					points.push_back(end_pos + Vector(direction.Z * 1.0f, direction.Y, direction.X * -1.0f));
					std::sort(points.begin(), points.end(),
						[&localObjPos](Vector first, Vector second)
						{
							return (localObjPos.DistTo(first) < localObjPos.DistTo(second));
						});
					auto closestPos = points.front();

					Engine::MoveTo(&closestPos);
				}
			}

			obj = holzer.GetNextObject(obj);
		}
	}

	//Me Range
	if (g_range == true) {
		if (Engine::GetLocalObject()) {

			if (Engine::GetLocalObject()->IsAlive()) {
				auto color = createRGB(0, 255, 0);
				Functions.DrawCircle(&Engine::GetLocalObject()->GetPos(), Engine::GetLocalObject()->GetAttackRange() + Engine::GetLocalObject()->GetBoundingRadius(), &color, 0, 0.0f, 0, 0.5f);
			}
		}
	}

	//line to mouse
	if (g_w2s_line == true) {
		Vector me_pos = Engine::GetLocalObject()->GetPos();
		Vector mepos_w2s;
		Functions.WorldToScreen(&me_pos, &mepos_w2s);
		Vector mouse_pos_w2s;
		Vector mouse_pos = Engine::GetMouseWorldPosition();
		Functions.WorldToScreen(&mouse_pos, &mouse_pos_w2s);
		render.draw_line(mepos_w2s.X, mepos_w2s.Y, mouse_pos_w2s.X, mouse_pos_w2s.Y, ImColor(15, 150, 40, 255), 5.0f);
	}

	//move to mouse
	if (g_move_to_mouse == true) {
		if (lastmove == NULL || clock() - lastmove > 30.0f) {
			lastmove = clock();
			Engine::MoveTo(&Engine::GetMouseWorldPosition());
		}
	}

	//draw range all hero using getfirst/getnext obj
	if (g_2range_objmanager == true) {
		CObject holzer;
		auto obj = holzer.GetFirstObject();
		while (obj)
		{
			if (obj->IsHero())
			{
				auto color = createRGB(255, 0, 0);
				Functions.DrawCircle(&obj->GetPos(), obj->GetAttackRange(), &color, 0, 0.0f, 0, 0.5f);
			}
			obj = holzer.GetNextObject(obj);
		}
	}
	//champion info demonstration
	if (g_champ_info == true) {
		CObject holzer;
		auto obj = holzer.GetFirstObject();
		while (obj)
		{
			if (obj->IsHero())
			{
				Vector obj_pos = obj->GetPos();
				Vector objpos_w2s;
				Functions.WorldToScreen(&obj_pos, &objpos_w2s);
				render.draw_text(objpos_w2s.X, objpos_w2s.Y + 15, obj->GetName(), true, ImColor(255, 0, 0, 255));
				render.draw_text(objpos_w2s.X, objpos_w2s.Y + 30, obj->GetChampionName(), true, ImColor(255, 0, 0, 255));

				//from float to char///////////////////////////
				char Get_Health[100];						///					
				sprintf_s(Get_Health, "%f", obj->GetHealth());///
				///////////////////////////////////////////////

				render.draw_text(objpos_w2s.X, objpos_w2s.Y + 45, Get_Health, true, ImColor(255, 0, 0, 255));

			}
			obj = holzer.GetNextObject(obj);
		}
	}



	render.end_draw();//end for draw render.drawline.... and etc
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