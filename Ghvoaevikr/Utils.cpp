#include "Utils.h"

#include <math.h>

char* stristr(const char* str1, const char* str2)
{
    const char* p1 = str1;
    const char* p2 = str2;
    const char* r = *p2 == 0 ? str1 : 0;

    while (*p1 != 0 && *p2 != 0)
    {
        if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
        {
            if (r == 0)
            {
                r = p1;
            }

            p2++;
        }
        else
        {
            p2 = str2;
            if (r != 0)
            {
                p1 = r + 1;
            }

            if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2))
            {
                r = p1;
                p2++;
            }
            else
            {
                r = 0;
            }
        }

        p1++;
    }

    return *p2 == 0 ? (char*)r : 0;
}

float calculate2dDistance(float x1, float y1, float x2, float y2)
{
    float diffY = y1 - y2;
    float diffX = x1 - x2;
    return sqrtf((diffY * diffY) + (diffX * diffX));
}

DWORD FindD3D9VTable(DWORD Len)
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

DWORD GetD3D9VTableFunction(int VTableIndex)
{
    PDWORD VTable;
    *(DWORD*)&VTable = *(DWORD*)FindD3D9VTable(0x128000);
    return VTable[VTableIndex];
}
