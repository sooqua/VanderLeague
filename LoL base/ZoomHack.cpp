#include "ZoomHack.h"

#include "Utils.h"

#include "Debug.h"

void ZoomHack::ChangeMaximumZoom(float amount) {
	DWORD* zoomInstance = reinterpret_cast<DWORD*>(g_BaseAddr + offsets::global::oZoomClass);
	float* zoomAmount = reinterpret_cast<float*>(*zoomInstance + 0x28);
	float newAmount = *zoomAmount + amount;
	if (newAmount < 4500.f && newAmount > 1000.f) {
		*zoomAmount = newAmount;
	}
}
