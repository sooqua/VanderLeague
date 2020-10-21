#pragma once

#ifndef RENDER_H
#define RENDER_H

#include "ImGui/imgui.h"
#include "d3d9.h"
#include "ImGui/win32/imgui_impl_win32.h"
#include "ImGui/dx9/imgui_impl_dx9.h"


class c_renderer
{
public:
	void init(LPDIRECT3DDEVICE9 device);
	void free(void);

public:
	void begin_draw(void);
	void end_draw(void);

public:
	ImFont* m_default = nullptr;
	ImFont* m_esp_name = nullptr;
	ImFont* m_weapon_icons = nullptr;

public:
	enum e_flags
	{
		// circle
		circle_normal,
		circle_filled,
		circle_3d,

		// rectangle
		rect_normal,
		rect_filled,

		// triangle
		triangle_normal,
		triangle_filled,

		// text
		text_normal,
		text_with_font
	};

public:
	void draw_text(float_t x, float_t y, const char* text, bool outlined,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = text_normal, ImFont* font = nullptr, ...);

	void draw_line(float_t x1, float_t y1, float_t x2, float_t y2,
		ImColor color = ImColor(255, 255, 255, 255), float_t thickness = 1.0f);

	void draw_rect(float_t x, float_t y, float_t w, float_t h,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = rect_normal, float_t rounding = 0.0f, uintptr_t points = 12, float_t thickness = 1.0f);

	void draw_triangle(float_t x1, float_t y1, float_t x2, float_t y2, 
		float_t x3, float_t y3, ImColor color = ImColor(255, 255, 255, 255), e_flags flags = triangle_normal, float_t thickness = 1.0f);

	void draw_circle(float_t x, float_t y, float_t radius,
		ImColor color = ImColor(255, 255, 255, 255), e_flags flags = circle_normal, uintptr_t points = 12, float_t thickness = 1.0f);

public:
	float get_width(void) { return m_width; }
	float get_height(void) { return m_height; }

	LPDIRECT3DDEVICE9 get_device(void) { return m_device; }

private:
	LPDIRECT3DDEVICE9 m_device = nullptr;

public:
	HWND m_hwnd = nullptr;

private:
	float m_width = 0.0f;
	float m_height = 0.0f;
};

extern c_renderer render;

#endif // RENDER_H