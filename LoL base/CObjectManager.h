#pragma once
#include "Utils.h"
#include "CObject.h"

class CObjectManager
{
public:

	short GetHighestIndex()
	{
		short idx = 0;
		auto objectList = *(CObject***)(*(DWORD*)(baseAddr + offsets::global::oObjManager) + offsets::other::objectArray);

		void* c = objectList[idx];
		while ((uintptr_t)c != 0x1)
		{
			c = objectList[++idx];
		}
		return idx;
	}

	CObject* GetObjByIndex(int index)
	{
		__try
		{
			auto objectList = *(CObject***)(*(DWORD*)(baseAddr + offsets::global::oObjManager) + offsets::other::objectArray);
			if (objectList)
			{
				//ENGINE_MSG("ObjectList : %p firstobj %p", objectList, objectList[0]);

				if (index > GetHighestIndex())
					return nullptr;
				auto obj = objectList[index];
				if (obj == nullptr)
					return nullptr;

				if (obj)
					return obj;

			}
		}
		__except (1) {}

		return nullptr;
	}



};

extern CObjectManager* ObjManager;

