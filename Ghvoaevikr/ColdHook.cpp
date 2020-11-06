/*
	Copyright (c) 2020 Rat431 (https://github.com/Rat431).
	This software is under the MIT license, for more informations check the LICENSE file.
*/

#include "ColdHook.h"
#include "HvppIoctl.h"

HvppIoctl hvppIoctl;

#ifdef _WIN64
#define VALID_MACHINE IMAGE_FILE_MACHINE_AMD64
typedef signed long long DisplacementVar;
static const BYTE AbsJumpRaw[14] = {
	0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};
#else
static const BYTE AbsJumpRaw[10] = {
	0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define VALID_MACHINE IMAGE_FILE_MACHINE_I386
typedef signed long DisplacementVar;
#endif

static const BYTE OffsetJumpLong[OFFSET_JUMP_LONG_HOOK_SIZE] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
static const BYTE OffsetJumpShort[OFFSET_JUMP_SHORT_HOOK_SIZE] = { 0xEB, 0x00 };


namespace ColdHook_Vars
{
	bool Inited = false;
	int32_t CurrentID = 0;
	ZydisDecoder decoder;

	std::multimap<int32_t, Hook_Info*> RegisteredHooks;
	std::mutex Thread;

	const char* pSystemMods[3] = { "kernel32.dll", "kernelbase.dll", "ntdll.dll" };
}

namespace ColdHook_Service
{
	// Private functions:
	static void* WalkThroughJumpIfPossible(void* pMemory)
	{
		void* curaddr				= pMemory;
		OffsetTypes Type			= TUNKNOWN;

		// get real address from jump table and stop when real code is found 
		__try {
			if (curaddr) {
				Type = GetInstructionOffType(curaddr);
				if (Type == TOFFSET_LONG_JUMP || Type == TOFFSET_SHORT_JUMP || Type == TABSOLUTE_JUMP || Type == TABSOLUTE_JUMP_CUSTOM) {
					curaddr = GetAddressFromOffset(curaddr, Type, 0, true, true);
					if (curaddr != pMemory) {
						return WalkThroughJumpIfPossible(curaddr);
					}
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			curaddr = nullptr;
			return curaddr;
		}
		return curaddr;
	}
	static void* GetAddressFromOffset(void* Base, OffsetTypes Type, size_t DispOffset, bool bReturnDefault, bool bGetInternalP)
	{
		void* ReturnAddress					= nullptr;
		void* Addr							= nullptr;
		void* DefaultAddress				= nullptr;

		ULONG_PTR uBase						= (ULONG_PTR)Base;

		if (uBase)
		{
			switch (Type)
			{
			case TSPECIAL_JUMP_C:
				Addr = (void*)((uBase + OFFSET_JUMP_COND_SHORT) + (*(char*)(uBase + sizeof(BYTE))));
				break;
			case TCONDITIONAL_LONG_JUMP:
				Addr = (void*)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP) + (*(int*)(uBase + sizeof(WORD))));
				break;
			case TCONDITIONAL_SHORT_JUMP:
				Addr = (void*)((uBase + OFFSET_JUMP_COND_SHORT) + (*(char*)(uBase + sizeof(BYTE))));
				break;
			case TOFFSET_SHORT_JUMP:
				Addr = (void*)((uBase + OFFSET_JUMP_COND_SHORT) + (*(char*)(uBase + sizeof(BYTE))));
				break;
			case TOFFSET_LONG_JUMP:
				Addr = (void*)((uBase + OFFSET_JUMP_LONG_AND_OFFSET_CALL) + (*(int*)(uBase + sizeof(BYTE))));
				break;
			case TOFFSET_CALL:
				Addr = (void*)((uBase + OFFSET_JUMP_LONG_AND_OFFSET_CALL) + (*(int*)(uBase + sizeof(BYTE))));
				break;
			case TABSOLUTE_CALL:
				Addr = (bGetInternalP == true) ? *(void**)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP) + (*(int*)(uBase + sizeof(WORD)))) :
					(void*)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP) + (*(int*)(uBase + sizeof(WORD))));
				break;
			case TABSOLUTE_JUMP:
				Addr = (bGetInternalP == true) ? *(void**)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP) + (*(int*)(uBase + sizeof(WORD)))) :
					(void*)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP) + (*(int*)(uBase + sizeof(WORD))));
				break;
			case TABSOLUTE_JUMP_CUSTOM:
				Addr = (bGetInternalP == true) ? *(void**)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP + sizeof(BYTE)) + (*(int*)(uBase + sizeof(BYTE) + sizeof(WORD)))) :
					(void*)((uBase + ABS_CALL_AND_COND_OFFSET_LONG_JUMP + sizeof(BYTE)) + (*(int*)(uBase + sizeof(BYTE) + sizeof(WORD))));
				break;
			default:
				Addr = *(void**)(uBase + DispOffset);
				break;
			}

			DefaultAddress = (bReturnDefault == true) ? Base : nullptr;
			ReturnAddress = (IsValidMem(Addr, false) == true) ? Addr : DefaultAddress;
		}
		return ReturnAddress;
	}
	static void* BeckupOriginalInstructions(void* pTarget, void* pTrampolineStart, size_t JumpHookS, size_t* pOutDLength)
	{
		void* TargetDest					= nullptr;
		size_t DSize						= 0;
		size_t RDSize						= 0;

		ZyanUSize offset					= 0;
		ZyanUSize length					= 0x1000;
		ZydisDecodedInstruction instruction;
		ULONG_PTR CurInsP					= (ULONG_PTR)pTarget;
		ULONG_PTR CurTInsP					= (ULONG_PTR)pTrampolineStart;

		while (ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&ColdHook_Vars::decoder, (void*)((ULONG_PTR)pTarget + offset), length - offset,
			&instruction))) {
			bool HasDisp = false;
			size_t EncodedInsSize = 0;
			int DispValue = 0;

			if (DSize >= JumpHookS) {
				TargetDest = (void*)CurInsP;
				break;
			}

			// search if the current instruction has a displacement
			HasDisp = (instruction.attributes & ZYDIS_ATTRIB_IS_RELATIVE) ? ZYAN_TRUE : ZYAN_FALSE;
			if (HasDisp) {
				for (int i = 0; i < instruction.operand_count; i++) {
					if (instruction.operands[i].type == ZYDIS_OPERAND_TYPE_MEMORY) {
						if (instruction.operands[i].mem.type != ZYDIS_MEMOP_TYPE_INVALID) {
							if (instruction.operands[i].mem.disp.has_displacement) {
								DispValue = (int)instruction.operands[i].mem.disp.value;
								break;
							}
						}
					}
				}
			}

			EncodedInsSize = FixInstruction((void*)CurInsP, (void*)CurTInsP, HasDisp, DispValue, instruction.length);
			if (!EncodedInsSize) {
				break;
			}

			CurTInsP += EncodedInsSize;
			CurInsP += instruction.length;

			RDSize += EncodedInsSize;
			offset += instruction.length;
			DSize += instruction.length;
		}

		if (pOutDLength) {
			*pOutDLength = RDSize;
		}
		return TargetDest;
	}
	static void* FindTrampoline(void* StartBaseAddress, int* pAllocated, DWORD* pCaveOProtection)
	{
		int Allocated						= 0;
		DWORD CaveOProtection				= 0;

		void* Trampoline					= nullptr;

		// get mem infos
		if (IsValidMem(StartBaseAddress, false)) {
			// code cave wasn't requested, try to allocate. 
			Trampoline = VirtualAlloc(nullptr, MAX_TRAMPSIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			Allocated = RANDOM_ALLOCATED_32;
		}
		
		if (pAllocated) {
			*pAllocated = Allocated;
		}
		if (pCaveOProtection) {
			*pCaveOProtection = CaveOProtection;
		}
		return Trampoline;
	}

	static size_t FixInstruction(void* pTarget, void* pNewPointer, bool bIsRelative, int DispValue, size_t CurrentInsLength)
	{
		if (!pTarget || !pNewPointer || !CurrentInsLength) { return 0; }

		size_t EncodedInsSize						= 0;
		int NewDisplaceMent							= 0;
		void* DestAddress							= nullptr;

		bool ReCompileJumpN							= false;
		bool IsLong									= false;
		bool Failed									= false;

		if (!bIsRelative) {
			memcpy(pNewPointer, pTarget, CurrentInsLength);
			return CurrentInsLength;
		}
		else {
			size_t DDisplacementOffset				= 0;
			InstructionType IInstructionType		= T_GENERAL_UNKNOWN;
			OffsetTypes OffsetInstructionType		= TUNKNOWN;

			// instruction must be fixed.
			OffsetInstructionType = GetInstructionOffType(pTarget);
			if (OffsetInstructionType != TUNKNOWN) {
				// we can fix with our code
				DestAddress = GetAddressFromOffset(pTarget, OffsetInstructionType, 0, false, false);
				if (DestAddress) {
					IInstructionType = GetInstructionTypeFromOffsetType(OffsetInstructionType);
					NewDisplaceMent = BuildInstructionTypeDisplaceMent(DestAddress, pNewPointer, IInstructionType,
						&EncodedInsSize, &DDisplacementOffset, 0, &IsLong, &Failed);
					if (!Failed) {
						ReCompileJumpN = MustInstructionBeFixed(OffsetInstructionType, IsLong);
						EncodeDisplaceMentInstruction(pNewPointer, pTarget, NewDisplaceMent, IInstructionType, ReCompileJumpN, IsLong, EncodedInsSize,
							DDisplacementOffset);
					}
				}
			}
			else {
				DDisplacementOffset = GetDisplacementOffset(pTarget, DispValue, CurrentInsLength, &Failed);
				if (!Failed) {
					DestAddress = GetAddressFromOffset(pTarget, OffsetInstructionType, DDisplacementOffset, false, false);
					if (DestAddress) {
						NewDisplaceMent = BuildInstructionTypeDisplaceMent(DestAddress, pNewPointer, IInstructionType,
							&EncodedInsSize, nullptr, CurrentInsLength, &IsLong, &Failed);
						EncodeDisplaceMentInstruction(pNewPointer, pTarget, NewDisplaceMent, IInstructionType, ReCompileJumpN, IsLong, EncodedInsSize,
							DDisplacementOffset);
					}
				}
			}
		}
		return EncodedInsSize;
	}
	static size_t GetDisplacementOffset(void* pInstruction, int DisplaceMent, size_t InsLength, bool* pbFailed)
	{
		// search for the displacement value offset relative to beginning of the instruction.
		bool Failed						= true;
		size_t DispOffRet				= 0;

		if (pInstruction && InsLength) {
			for (size_t i = (InsLength - sizeof(int)); i > 0; i--) {
				if (*(int*)((ULONG_PTR)pInstruction + i) == DisplaceMent) {
					Failed = false;
					DispOffRet = i;
					break;
				}
			}
			if (Failed) {
				// search for 1 signed byte
				for (size_t i = (InsLength - sizeof(char)); i > 0; i--) {
					if (*(char*)((ULONG_PTR)pInstruction + i) == (char)DisplaceMent) {
						Failed = false;
						DispOffRet = i;
						break;
					}
				}
			}
		}

		if (pbFailed) {
			*pbFailed = Failed;
		}
		return DispOffRet;
	}
	static size_t PlaceOffsetJump(void* pDestination, void* pTarget, void* pMemory)
	{
		bool IsLongJumpReq					= false;
		int Displacement					= 0;
		size_t JumpSize						= 0;

		if (pDestination && pTarget && pMemory) {
			Displacement = BuildInstructionTypeDisplaceMent(pDestination, pTarget, TOFFSET_GENERAL_JUMP, nullptr, nullptr, 0, &IsLongJumpReq, nullptr);
			if (IsLongJumpReq) {
				memcpy(pMemory, OffsetJumpLong, OFFSET_JUMP_LONG_HOOK_SIZE);
				*(int*)((ULONG_PTR)pMemory + DISPLACEMENT_OFFSET) = Displacement;
				JumpSize = OFFSET_JUMP_LONG_HOOK_SIZE;
			}
			else {
				memcpy(pMemory, OffsetJumpShort, OFFSET_JUMP_SHORT_HOOK_SIZE);
				*(char*)((ULONG_PTR)pMemory + DISPLACEMENT_OFFSET) = (char)Displacement;
				JumpSize = OFFSET_JUMP_SHORT_HOOK_SIZE;
			}
		}
		return JumpSize;
	}
	static size_t PlaceAbsJump(void* pDestination, void* pMemory)
	{
		size_t AbsJmpSize			= 0;

		if (pDestination && pMemory) {
			memcpy(pMemory, AbsJumpRaw, ABS_HOOK_SIZE);
			*(void**)((ULONG_PTR)pMemory + ABS_JUMP_ADDRESS_OFFSET) = pDestination;
			AbsJmpSize = ABS_HOOK_SIZE;

			// different for 32 bit
			*(DWORD*)((ULONG_PTR)pMemory + sizeof(WORD)) = (DWORD)(((ULONG_PTR)pMemory + ABS_JUMP_ADDRESS_OFFSET));
		}
		return AbsJmpSize;
	}
	static WORD ConvertOpcode(void* pInstruction, InstructionType InsType, bool bToLong)
	{
		WORD Ret = 0;
		if (pInstruction) {
			if (InsType == TCONDITIONAL_GENERAL_JUMP) {
				if (bToLong) {
					*(BYTE*)(&Ret) = 0x0F;
					// last 4 bits
					*(BYTE*)((ULONG_PTR)&Ret + sizeof(BYTE)) = (0x80 | ((*(BYTE*)(pInstruction)) & 0x0F));
				} else {
					*(BYTE*)(&Ret) = (0x70 | ((*(BYTE*)((ULONG_PTR)pInstruction + sizeof(BYTE))) & 0x0F));
				}
			} else if (InsType == TOFFSET_GENERAL_JUMP) {
				if (bToLong) {
					*(BYTE*)(&Ret) = 0xE9;
				} else {
					*(BYTE*)(&Ret) = 0xEB;
				}
			}
		}
		return Ret;
	}

	static int BuildInstructionTypeDisplaceMent(void* pDestination, void* pTarget, InstructionType InsType, size_t* pNeededEncodeLength,
		size_t* pNewDisplaceMentOffset, size_t InsLength, bool* pbLongJump, bool* pbFailed)
	{
		DisplacementVar RealDisplacement			= 0;
		SIZE_T TSize								= 0;
		
		size_t NeededEncodeLength					= 0;
		size_t DisplacementOffset					= 0;
		int Flag									= EQUAL;

		bool Failed									= false;
		bool NotConfirmedInRange					= false;
		bool InRange								= false;
		bool LongJ									= false;

		if (pDestination && pTarget) {
			// check type
			if ((ULONG_PTR)pDestination > (ULONG_PTR)pTarget) {
				TSize = (SIZE_T)((ULONG_PTR)pDestination - (ULONG_PTR)pTarget);
				NotConfirmedInRange = !(TSize > MAX_RANGE_DELTA_P);
				Flag = POSITIVE;
			} else if ((ULONG_PTR)pDestination < (ULONG_PTR)pTarget) {
				TSize = (SIZE_T)((ULONG_PTR)pTarget - (ULONG_PTR)pDestination);
				NotConfirmedInRange = !(TSize > MAX_RANGE_DELTA_P);
				Flag = NEGATIVE;
			} else {
				NotConfirmedInRange = true;
				Flag = EQUAL;
			}

			InRange = true;

			if (InRange) {
				// calculate new offset
				RealDisplacement = (DisplacementVar)(((ULONG_PTR)pDestination - (ULONG_PTR)pTarget));

				if (InsType == TSPECIAL__GENERAL_JUMP_C || InsType == TCONDITIONAL_GENERAL_JUMP || InsType == TOFFSET_GENERAL_JUMP) {
					if (Flag == NEGATIVE) {
						if (TSize > MAX_COND_SHORT_JUMP_OFFSET_N) {
							if (InsType == TSPECIAL__GENERAL_JUMP_C)
								Failed = true;
							else
								LongJ = true;
						} else {
							LongJ = false;
						}
					} 
					else if (Flag == POSITIVE) {
						if (TSize > MAX_COND_SHORT_JUMP_OFFSET_P) {
							if (InsType == TSPECIAL__GENERAL_JUMP_C)
								Failed = true;
							else
								LongJ = true;
						} else {
							LongJ = false;
						}
					} 
					else {
						LongJ = false;
					}
					if (!Failed) {
						if (LongJ) {
							if (InsType == TCONDITIONAL_GENERAL_JUMP) {
								// for coditional jumps
								RealDisplacement -= ABS_CALL_AND_COND_OFFSET_LONG_JUMP;
								NeededEncodeLength = ABS_CALL_AND_COND_OFFSET_LONG_JUMP;
								DisplacementOffset = sizeof(WORD);
							} else {
								// a non conditional jump
								RealDisplacement -= OFFSET_JUMP_LONG_AND_OFFSET_CALL;
								NeededEncodeLength = OFFSET_JUMP_LONG_AND_OFFSET_CALL;
								DisplacementOffset = sizeof(BYTE);
							}
						} else {
							RealDisplacement -= OFFSET_JUMP_COND_SHORT;
							NeededEncodeLength = OFFSET_JUMP_COND_SHORT;
							DisplacementOffset = sizeof(BYTE);
						}
					}
				} 
				else if (InsType == TABSOLUTE_GENERAL_CALL || InsType == TABSOLUTE_GENERAL_JUMP) {
					RealDisplacement -= ABS_CALL_AND_COND_OFFSET_LONG_JUMP;
					NeededEncodeLength = ABS_CALL_AND_COND_OFFSET_LONG_JUMP;
					DisplacementOffset = sizeof(WORD);
				} 
				else if (InsType == TOFFSET_GENERAL_CALL) {
					RealDisplacement -= OFFSET_JUMP_LONG_AND_OFFSET_CALL;
					NeededEncodeLength = OFFSET_JUMP_LONG_AND_OFFSET_CALL;
					DisplacementOffset = sizeof(BYTE);
				} 
				else if (InsType == TABSOLUTE_GENERAL_JUMP_CUSTOM) {
					RealDisplacement -= (ABS_CALL_AND_COND_OFFSET_LONG_JUMP + sizeof(BYTE));
					NeededEncodeLength = (ABS_CALL_AND_COND_OFFSET_LONG_JUMP + sizeof(BYTE));
					DisplacementOffset = (sizeof(BYTE) + sizeof(WORD));
				} 
				else {
					RealDisplacement -= InsLength;
					NeededEncodeLength = InsLength;
					// I guess...
					DisplacementOffset = 0;
				}
			}
			else
				Failed = true;
		}
		else
			Failed = true;

		if (pbFailed) {
			*pbFailed = Failed;
		}
		if (pNeededEncodeLength) {
			*pNeededEncodeLength = NeededEncodeLength;
		}
		if (pNewDisplaceMentOffset) {
			*pNewDisplaceMentOffset = DisplacementOffset;
		}
		if (pbLongJump) {
			*pbLongJump = LongJ;
		}
		return RealDisplacement;
	}

	static InstructionType GetInstructionTypeFromOffsetType(OffsetTypes OffType)
	{
		InstructionType Ret;
		switch (OffType)
		{
		case TSPECIAL_JUMP_C:
			Ret = TSPECIAL__GENERAL_JUMP_C;
			break;
		case TCONDITIONAL_LONG_JUMP:
			Ret = TCONDITIONAL_GENERAL_JUMP;
			break;
		case TCONDITIONAL_SHORT_JUMP:
			Ret = TCONDITIONAL_GENERAL_JUMP;
			break;
		case TOFFSET_SHORT_JUMP:
			Ret = TOFFSET_GENERAL_JUMP;
			break;
		case TOFFSET_LONG_JUMP:
			Ret = TOFFSET_GENERAL_JUMP;
			break;
		case TOFFSET_CALL:
			Ret = TOFFSET_GENERAL_CALL;
			break;
		case TABSOLUTE_CALL:
			Ret = TABSOLUTE_GENERAL_CALL;
			break;
		case TABSOLUTE_JUMP:
			Ret = TABSOLUTE_GENERAL_JUMP;
			break;
		case TABSOLUTE_JUMP_CUSTOM:
			Ret = TABSOLUTE_GENERAL_JUMP_CUSTOM;
			break;
		default:
			Ret = T_GENERAL_UNKNOWN;
			break;
		}
		return Ret;
	}
	static OffsetTypes GetInstructionOffType(void* pInstruction)
	{
		// check instructions
		if (IsValidMem(pInstruction, false)) {
			if (*(BYTE*)(pInstruction) == 0xE8) {
				return TOFFSET_CALL;
			}
			if (*(BYTE*)(pInstruction) == 0xE9) {
				return TOFFSET_LONG_JUMP;
			}
			if (*(BYTE*)(pInstruction) == 0xEB) {
				return TOFFSET_SHORT_JUMP;
			}

			if (*(BYTE*)(pInstruction) >= 0x70 && *(BYTE*)(pInstruction) <= 0x7F) {
				return TCONDITIONAL_SHORT_JUMP;
			}
			if (*(BYTE*)(pInstruction) == 0x0F) {
				if (*(BYTE*)((ULONG_PTR)pInstruction + sizeof(BYTE)) >= 0x80 && *(BYTE*)((ULONG_PTR)pInstruction + sizeof(BYTE)) <= 0x8F)
					return TCONDITIONAL_LONG_JUMP;
			}

			if (*(BYTE*)(pInstruction) == 0xE3 || *(BYTE*)(pInstruction) == 0xE2
				|| *(BYTE*)(pInstruction) == 0xE1 || *(BYTE*)(pInstruction) == 0xE0) {
				// not very much important, 2 bytes size.
				return TSPECIAL_JUMP_C;
			}

			// absolute
			if (*(WORD*)(pInstruction) == 0x15FF) {
				return TABSOLUTE_CALL;
			}
			if (*(WORD*)(pInstruction) == 0x25FF) {
				return TABSOLUTE_JUMP;
			}
		}
		return TUNKNOWN;
	}

	static bool MustInstructionBeFixed(OffsetTypes Type, bool bLongDistance)
	{
		bool Ret;
		switch (Type)
		{
		case TCONDITIONAL_LONG_JUMP:
			Ret = (bLongDistance == false);
			break;
		case TCONDITIONAL_SHORT_JUMP:
			Ret = bLongDistance;
			break;
		case TOFFSET_SHORT_JUMP:
			Ret = bLongDistance;
			break;
		case TOFFSET_LONG_JUMP:
			Ret = (bLongDistance == false);
			break;
		case TOFFSET_CALL:
			Ret = false;
			break;
		case TABSOLUTE_CALL:
			Ret = false;
			break;
		case TABSOLUTE_JUMP:
			Ret = false;
			break;
		case TABSOLUTE_JUMP_CUSTOM:
			Ret = false;
			break;
		default:
			Ret = false;
			break;
		}
		return Ret;
	}
	static bool IsValidMem(void* pMem, bool bWriteAccessNeeded)
	{
		bool bRet = false;
		if (pMem) {
			if (!bWriteAccessNeeded)
				bRet = (IsBadReadPtr(pMem, sizeof(DWORD)) == FALSE);
			else
				bRet = (IsBadWritePtr(pMem, sizeof(DWORD)) == FALSE);
		}
		return bRet;
	}
	static bool EncodeDisplaceMentInstruction(void* pMemory, void* pOldOpCode, int offset, InstructionType InsType, bool bConvertOpcode,
		bool IsLong, size_t EncodeSize, size_t DispOffset)
	{
		bool Continue				= false;
		bool Failed					= false;
		WORD CnvOpcode				= 0;

		if (pMemory && EncodeSize && pOldOpCode) {
			if (bConvertOpcode) {
				// apparently the instruction was a (conditional) jump and it must be converted for a longer/smaller distance
				CnvOpcode = ConvertOpcode(pOldOpCode, InsType, IsLong);
				if (CnvOpcode) {
					// copy the first converted bytes
					memcpy(pMemory, &CnvOpcode, DispOffset);
					Continue = true;
				}
				else
					Failed = true;
			}
			else {
				Continue = true;
				memcpy(pMemory, pOldOpCode, DispOffset);
			}

			if (Continue) {
				// install the displacement
				memcpy((void*)((ULONG_PTR)pMemory + DispOffset), &offset, (EncodeSize - DispOffset));
			}
		}
		else
			Failed = true;
		return (Failed != true);
	}
	static bool IsHookAlreadyRegistered(int32_t HookID)
	{
		bool bRet = false;
		if (!ColdHook_Vars::RegisteredHooks.empty()) {
			auto pos = ColdHook_Vars::RegisteredHooks.find(HookID);
			bRet = (pos != ColdHook_Vars::RegisteredHooks.end());
		}
		return bRet;
	}

	static void LockOrUnlockOtherThreads(bool bLock)
	{
		if (bLock)
			ColdHook_Vars::Thread.lock();
		else
			ColdHook_Vars::Thread.unlock();
	}

	static void InternalUnHookRegData(bool ShutDown, Hook_Info* pData, int32_t* OutErrorCode)
	{
		int32_t ErrorC				= 0;

		if (pData)
		{
			if (pData->StatusHooked)
			{
				if (pData->IsDetourHook)
				{
					hvppIoctl.WriteProtectedMemory(pData->OrgData, pData->HFunction, pData->HookSize);
					if (pData->TrampolineAllocated)
					{
						if (ShutDown)
						{
							VirtualFree(pData->TrampolinePage, 0, MEM_RELEASE);

							pData->TrampolinePage = nullptr;
							pData->TrampolineAllocated = false;
						}
					}
					else
					{
						if (pData->TrampolinePage) 
						{
							// code cave...
							BYTE* pPos = (BYTE*)pData->TrampolinePage;
							for (size_t i = 0; i < MAX_CAVE_DATA; i++) 
							{
								hvppIoctl.WriteProtectedMemory(&pData->CodeCaveOData, &pPos[i], sizeof(BYTE));
							}
						}
					}
				}
				else 
				{
					hvppIoctl.WriteProtectedMemory(pData->COrgData, pData->HFunction, pData->HookSize);
					if (ShutDown) 
					{
						free(pData->COrgData);
						free(pData->CHookData);

						pData->COrgData = nullptr;
						pData->CHookData = nullptr;
					}
				}
				pData->StatusHooked = false;
			} 
			else
			{
				ErrorC = FAILED_HOOK_NOT_EXISTS;
			}
		}

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
	}
	static void InternalHookRegData(Hook_Info* pData, int32_t* OutErrorCode)
	{
		int32_t ErrorC				= 0;

		if (pData)
		{
			if (!pData->StatusHooked)
			{
				if (pData->IsDetourHook)
				{
					if (!pData->TrampolineAllocated)
					{
						if (pData->TrampolinePage)
						{
							hvppIoctl.WriteProtectedMemory(pData->CaveHookData, pData->TrampolinePage, MAX_CAVE_DATA);
							hvppIoctl.WriteProtectedMemory(pData->HookData, pData->HFunction, pData->HookSize);

							pData->StatusHooked = true;
						}
						else
						{
							hvppIoctl.WriteProtectedMemory(pData->HookData, pData->HFunction, pData->HookSize);
							pData->StatusHooked = true;
						}
					}
					else
					{
						hvppIoctl.WriteProtectedMemory(pData->HookData, pData->HFunction, pData->HookSize);
						pData->StatusHooked = true;
					}
				}
				else
				{
					hvppIoctl.WriteProtectedMemory(pData->COrgData, pData->HFunction, pData->HookSize);
					pData->StatusHooked = true;
				}
			}
			else
			{
				ErrorC = FAILED_HOOK_EXISTS;
			}
		}

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
	}
	static Hook_Info* InternalInitializeSTR(bool bDetourHook)
	{
		Hook_Info* OutputInfo			= nullptr;

		OutputInfo = (Hook_Info*)malloc(sizeof(Hook_Info));
		if (OutputInfo) {
			// initialize structure 
			memset(OutputInfo->OrgData, 0, MAX_HOOK_ARRAY);
			memset(OutputInfo->HookData, 0, MAX_HOOK_ARRAY);
			memset(OutputInfo->CaveHookData, 0, MAX_CAVE_DATA);

			OutputInfo->StatusHooked = false;
			OutputInfo->TrampolineAllocated = false;
			OutputInfo->IsDetourHook = bDetourHook;
			OutputInfo->OriginalF = nullptr;
			OutputInfo->HFunction = nullptr;
			OutputInfo->TrampolinePage = nullptr;
			OutputInfo->CHookData = nullptr;
			OutputInfo->COrgData = nullptr;
			OutputInfo->HookSize = 0;
			OutputInfo->CaveOriginalProtection = 0;
			OutputInfo->CodeCaveOData = 0;
		}
		return OutputInfo;
	}
	static void InternalEmuHook(void* pPlace, void* pDFunction, Hook_Info* OutputInfo, int32_t* pOutErrorCode)
	{
		int32_t ErrorCode = 0;
		if (IsValidMem(pPlace, false) && IsValidMem(pDFunction, false)) {
			OutputInfo->HookSize = PlaceOffsetJump(pDFunction, pPlace, OutputInfo->HookData);
		}
		else
			ErrorCode = FAILED_MEM_PROTECTION;

		if (pOutErrorCode) {
			*pOutErrorCode = ErrorCode;
		}
	}
	static void* InternalDetourHook(void* pPlace, void* pDFunction, Hook_Info* OutputInfo, int32_t* pOutErrorCode)
	{
		// special hook type to allow to call the original function back.
		void* BeckupF						= pPlace;
		void* CurFunction					= BeckupF;
		void* HookedF						= pDFunction;
		void* pTrampInstructionStart		= nullptr;
		void* pReturnFunction				= nullptr;
		void* pTrampInstructionEnd			= nullptr;
		void* pRedirStart					= nullptr;
		void* pReturnCode					= nullptr;
		void* Trampoline					= nullptr;

		size_t DisassembledLength			= 0;
		size_t VarHookSize					= 0;

		bool bPatchRedirAddress				= false;
		bool bAbsReturnJump					= false;
		bool HOffsetJump					= true;
		bool IsCodeCave						= false;

		int TrampolineResponse				= 0;
		int32_t ErrorCode					= 0;
		DWORD OldCaveP						= 0;

		if (IsValidMem(CurFunction, false) && IsValidMem(HookedF, false))
		{
			CurFunction = WalkThroughJumpIfPossible(BeckupF);
			if (CurFunction == nullptr)
				CurFunction = BeckupF;

			Trampoline = FindTrampoline(CurFunction, &TrampolineResponse, &OldCaveP);
			if (Trampoline)
			{
				switch (TrampolineResponse)
				{
				case ALLOCATED_64_2GB_CLOSE:
					pTrampInstructionStart = (void*)((ULONG_PTR)Trampoline + ABS_64_HOOK_SIZE);
					pRedirStart = Trampoline;
					HOffsetJump = true;
					bAbsReturnJump = true;
					bPatchRedirAddress = true;
					IsCodeCave = false;

					OutputInfo->TrampolineAllocated = true;
					OutputInfo->TrampolinePage = Trampoline;
					OutputInfo->CaveOriginalProtection = OldCaveP;
					OutputInfo->CodeCaveOData = 0;
					break;
				case RANDOM_ALLOCATED_32:
					pTrampInstructionStart = Trampoline;
					pRedirStart = HookedF;
					HOffsetJump = true;
					bAbsReturnJump = false;
					bPatchRedirAddress = false;
					IsCodeCave = false;

					OutputInfo->TrampolineAllocated = true;
					OutputInfo->TrampolinePage = Trampoline;
					OutputInfo->CaveOriginalProtection = OldCaveP;
					OutputInfo->CodeCaveOData = 0;
					break;
				case RANDOM_ALLOCATED_64:
					pTrampInstructionStart = Trampoline;
					pRedirStart = HookedF;
					HOffsetJump = false;
					bAbsReturnJump = true;
					bPatchRedirAddress = false;
					IsCodeCave = false;

					OutputInfo->TrampolineAllocated = true;
					OutputInfo->TrampolinePage = Trampoline;
					OutputInfo->CaveOriginalProtection = OldCaveP;
					OutputInfo->CodeCaveOData = 0;
					break;
				default:
					// code cave
					pTrampInstructionStart = Trampoline;
					pRedirStart = HookedF;
					HOffsetJump = true;
					bAbsReturnJump = false;
					bPatchRedirAddress = false;
					IsCodeCave = true;

					OutputInfo->TrampolineAllocated = false;
					OutputInfo->TrampolinePage = Trampoline;
					OutputInfo->CaveOriginalProtection = OldCaveP;
					OutputInfo->CodeCaveOData = *(BYTE*)(Trampoline);
					break;
				}

				// prepare hook data
				if (!HOffsetJump) {
					VarHookSize = PlaceAbsJump(pRedirStart, OutputInfo->HookData);
				}
				else {
					VarHookSize = PlaceOffsetJump(pRedirStart, CurFunction, OutputInfo->HookData);
				}

				// final hook size
				pReturnFunction = pTrampInstructionStart;
				OutputInfo->HookSize = VarHookSize;
				OutputInfo->OriginalF = pReturnFunction;

				// disassemble and copy the instruction
				pReturnCode = BeckupOriginalInstructions(CurFunction, pTrampInstructionStart, VarHookSize, &DisassembledLength);
				if (pReturnCode) {
					pTrampInstructionEnd = (void*)((ULONG_PTR)pTrampInstructionStart + DisassembledLength);

					// trampoline setup 
					if (bPatchRedirAddress) {
						PlaceAbsJump(HookedF, pRedirStart);
					}

					if (bAbsReturnJump) {
						PlaceAbsJump(pReturnCode, pTrampInstructionEnd);
					}
					else {
						PlaceOffsetJump(pReturnCode, pTrampInstructionEnd, pTrampInstructionEnd);
					}

					if (IsCodeCave) {
						memcpy(OutputInfo->CaveHookData, Trampoline, MAX_CAVE_DATA);
					}
				}
				else
					ErrorCode = FAILED_DISASSEMBLER;
			}
			else
				ErrorCode = FAILED_TRAMPOLINE_NOT_FOUND;
		}
		else
			ErrorCode = FAILED_MEM_PROTECTION;

		if (pOutErrorCode) {
			*pOutErrorCode = ErrorCode;
		}
		return CurFunction;
	}

	// Function wrap hooks
	int32_t InitFunctionHookByName(Hook_Info** OutputInfo, bool WrapFunction, bool CheckKBase, const char* ModulName, const char* FName, void* HookedF, int32_t* OutErrorCode)
	{
		// Vars
		void* CurFunction						= nullptr;
		const char* ModuleN						= ModulName;

		int32_t ErrorC							= 0;
		int32_t Ret								= 0;

		// Check arguments
		if (ModuleN && FName) {
			// Check if we can use the kernel base module 
			if (CheckKBase) {
				if (strlen("kernel32.dll") == strlen(ModuleN)) {
					if (strcmp(ModuleN, "kernel32.dll") == 0) {
						ModuleN = "kernelbase.dll";
					}
				}
			}
			// Get function pointer
			CurFunction = GetProcAddress(GetModuleHandleA(ModuleN), FName);
			if(!CurFunction)
				CurFunction = GetProcAddress(GetModuleHandleA(ModulName), FName);

			Ret = InitFunctionHookByAddress(OutputInfo, WrapFunction, CurFunction, HookedF, &ErrorC);

		} else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return Ret;
	}
	int32_t InitFunctionHookByAddress(Hook_Info** OutputInfo, bool WrapFunction, void* Target, void* HookedF, int32_t* OutErrorCode)
	{
		// Vars
		void* CurFunction						= Target;
		Hook_Info* OutputInfoVar				= nullptr;
		Hook_Info* OutputRet					= nullptr;

		int32_t ErrorC							= 0;
		int32_t Ret								= 0;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		// arguments 
		if (OutputInfo && CurFunction && HookedF) {
			// check system initialization
			if (ColdHook_Vars::Inited) {
				OutputInfoVar = InternalInitializeSTR(true);

				if (OutputInfoVar) {
					if (!WrapFunction) {
						InternalEmuHook(CurFunction, HookedF, OutputInfoVar, &ErrorC);
					}
					else {
						CurFunction = InternalDetourHook(CurFunction, HookedF, OutputInfoVar, &ErrorC);
					}

					if (!ErrorC) {
						// Hook

						// Store original data
						memcpy(OutputInfoVar->OrgData, CurFunction, OutputInfoVar->HookSize);

						// Place hook
						hvppIoctl.WriteProtectedMemory(OutputInfoVar->HookData, CurFunction, OutputInfoVar->HookSize);

						OutputInfoVar->StatusHooked = true;
						OutputInfoVar->HFunction = CurFunction;

						OutputRet = OutputInfoVar;
						++ColdHook_Vars::CurrentID;
						Ret = ColdHook_Vars::CurrentID;
					}
				}
				else {
					ErrorC = FAILED_ALLOCATION;
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}
			*OutputInfo = OutputRet;
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		// resume other threads execution 
		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return Ret;
	}

	// Memory custom hook
	int32_t InitHookCustomData(Hook_Info** OutputInfo, void* Target, void* CustomData, size_t CSize, int32_t* OutErrorCode)
	{
		// Vars
		Hook_Info* OutputInfoVar		= nullptr;
		Hook_Info* OutputRet			= nullptr;

		int32_t ErrorC					= 0;
		int32_t Ret						= 0;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		// arguments 
		if (OutputInfo && Target && CustomData) {
			if (IsValidMem(Target, false) && IsValidMem(CustomData, false)) {
				// check system initialization
				if (ColdHook_Vars::Inited) {
					OutputInfoVar = InternalInitializeSTR(false);

					if (OutputInfoVar) {
						// prepare buffers to save original and hook data
						OutputInfoVar->COrgData = malloc(CSize);
						if (OutputInfoVar->COrgData) {
							memset(OutputInfoVar->COrgData, 0, CSize);
							OutputInfoVar->CHookData = malloc(CSize);
							if (OutputInfoVar->CHookData) {
								memset(OutputInfoVar->CHookData, 0, CSize);

								// Hook
								OutputInfoVar->HookSize = CSize;
								OutputInfoVar->HFunction = Target;

								// Store original and hook data
								memcpy(OutputInfoVar->COrgData, Target, CSize);
								memcpy(OutputInfoVar->CHookData, CustomData, CSize);

								// Place hook
								hvppIoctl.WriteProtectedMemory(CustomData, Target, CSize);

								OutputInfoVar->StatusHooked = true;

								OutputRet = OutputInfoVar;
								++ColdHook_Vars::CurrentID;
								Ret = ColdHook_Vars::CurrentID;
							}
							else {
								free(OutputInfoVar->COrgData);
								ErrorC = FAILED_BUFFER_CREATION;
							}
						}
						else {
							ErrorC = FAILED_BUFFER_CREATION;
						}
					}
					else {
						ErrorC = FAILED_ALLOCATION;
					}
				}
				else {
					ErrorC = FAILED_NEEDS_INITIALIZATION;
				}
			}
			else {
				ErrorC = FAILED_MEM_PROTECTION;
			}

			*OutputInfo = OutputRet;
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		// resume other threads execution 
		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return Ret;
	}

	// UnHook
	bool UnHookRegisteredData(int32_t HookID, int32_t* OutErrorCode)
	{
		bool bRet				= false;
		int32_t ErrorC			= FAILED_NOT_EXISTS;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (HookID) {
			if (ColdHook_Vars::Inited) {
				if (!ColdHook_Vars::RegisteredHooks.empty()) {
					auto pos = ColdHook_Vars::RegisteredHooks.find(HookID);
					if (pos != ColdHook_Vars::RegisteredHooks.end()) {
						InternalUnHookRegData(false, pos->second, &ErrorC);
						bRet = (ErrorC == 0);
					}
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}
	bool HookAgainRegisteredData(int32_t HookID, int32_t* OutErrorCode)
	{
		bool bRet				= false;
		int32_t ErrorC			= FAILED_NOT_EXISTS;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (HookID) {
			if (ColdHook_Vars::Inited) {
				if (!ColdHook_Vars::RegisteredHooks.empty()) {
					auto pos = ColdHook_Vars::RegisteredHooks.find(HookID);
					if (pos != ColdHook_Vars::RegisteredHooks.end()) {
						InternalHookRegData(pos->second, &ErrorC);
						bRet = (ErrorC == 0);
					}
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}

	// Init And shut down
	bool ServiceGlobalInit(int32_t* OutErrorCode)
	{
		bool bRet				= false;
		int32_t ErrorC			= 0;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		// init the service 
		if (!ColdHook_Vars::Inited) {
			// init disassembler
			ZydisDecoderInit(&ColdHook_Vars::decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_ADDRESS_WIDTH_32);

			ColdHook_Vars::Inited = true;
			ColdHook_Vars::CurrentID = 0;
			bRet = ColdHook_Vars::Inited;
		}
		else {
			ErrorC = FAILED_ALREADY_INITIALIZED;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}
	bool ServiceGlobalShutDown(bool UnHook, int32_t* OutErrorCode)
	{
		bool bRet					= false;
		int32_t ErrorC				= 0;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (ColdHook_Vars::Inited) {
			if (!ColdHook_Vars::RegisteredHooks.empty()) {
				if (UnHook) {
					for (auto iter = ColdHook_Vars::RegisteredHooks.begin(); iter != ColdHook_Vars::RegisteredHooks.end(); ++iter) {
						if (iter->second) {
							InternalUnHookRegData(true, iter->second, &ErrorC);
							if (ErrorC != 0)
								if (ErrorC != FAILED_HOOK_NOT_EXISTS)
									break;
						}
					}
				}
				ColdHook_Vars::RegisteredHooks.clear();
			}
			ColdHook_Vars::Inited = (ErrorC == 0) ? false : true;
			bRet = (ColdHook_Vars::Inited == false);
		}
		else {
			ErrorC = FAILED_NEEDS_INITIALIZATION;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}

	// Informations
	bool RetrieveHookInfoByID(Hook_Info** OutputInfo, int32_t HookID, int32_t* OutErrorCode)
	{
		bool bRet						= false;

		Hook_Info* Ret					= nullptr;
		int32_t ErrorC					= FAILED_HOOK_NOT_EXISTS;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (OutputInfo && HookID) {
			if (ColdHook_Vars::Inited) {
				if (!ColdHook_Vars::RegisteredHooks.empty()) {
					auto pos = ColdHook_Vars::RegisteredHooks.find(HookID);
					if (pos != ColdHook_Vars::RegisteredHooks.end()) {
						ErrorC = 0;
						Ret = pos->second;
						bRet = true;
					}
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}

			*OutputInfo = Ret;
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}
	bool RetrieveHookIDByInfo(Hook_Info* InputInfo, int32_t* OutHookID, int32_t* OutErrorCode)
	{
		bool bRet						= false;

		int32_t HookID					= 0;
		int32_t ErrorC					= FAILED_HOOK_NOT_EXISTS;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (InputInfo && OutHookID) {
			if (ColdHook_Vars::Inited) {
				if (!ColdHook_Vars::RegisteredHooks.empty()) {
					for (auto iter = ColdHook_Vars::RegisteredHooks.begin(); iter != ColdHook_Vars::RegisteredHooks.end(); ++iter) {
						if (iter->second) {
							if (iter->second == InputInfo) {
								HookID = iter->first;
								ErrorC = 0;
								bRet = true;
							}
						}
					}
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}

			*OutHookID = HookID;
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}

	bool ServiceRegisterHookInformation(Hook_Info* InputInfo, int32_t HookID, int32_t* OutErrorCode)
	{
		bool bRet					= false;
		int32_t ErrorC				= 0;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (InputInfo && HookID) {
			if (ColdHook_Vars::Inited) {
				if (!IsHookAlreadyRegistered(HookID)) {
					ColdHook_Vars::RegisteredHooks.insert(std::make_pair(HookID, InputInfo));
					bRet = true;
				}
				else {
					ErrorC = FAILED_ALREADY_EXISTS;
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}
	bool ServiceUnRegisterHookInformation(int32_t HookID, int32_t* OutErrorCode)
	{
		bool bRet					= false;
		int32_t ErrorC				= 0;

		// suspend other threads execution
		LockOrUnlockOtherThreads(true);

		if (HookID) {
			if (ColdHook_Vars::Inited) {
				if (IsHookAlreadyRegistered(HookID)) {
					ColdHook_Vars::RegisteredHooks.erase(ColdHook_Vars::RegisteredHooks.find(HookID));
					bRet = true;
				}
				else {
					ErrorC = FAILED_NOT_EXISTS;
				}
			}
			else {
				ErrorC = FAILED_NEEDS_INITIALIZATION;
			}
		}
		else {
			ErrorC = FAILED_INVALID_PARAMETER;
		}

		LockOrUnlockOtherThreads(false);

		if (OutErrorCode) {
			*OutErrorCode = ErrorC;
		}
		return bRet;
	}

	// Error handler 
	const char* CHRetrieveErrorCodeString(int32_t InErrorCode)
	{
		const char* ErrorString;
		switch (InErrorCode)
		{
		case 0:
			ErrorString = "SUCCESS_NO_ERROR";
			break;
		case FAILED_NEEDS_INITIALIZATION:
			ErrorString = "FAILED_NEEDS_INITIALIZATION";
			break;
		case FAILED_ALREADY_INITIALIZED:
			ErrorString = "FAILED_ALREADY_INITIALIZED";
			break;
		case FAILED_HOOK_EXISTS:
			ErrorString = "FAILED_HOOK_EXISTS";
			break;
		case FAILED_HOOK_NOT_EXISTS:
			ErrorString = "FAILED_HOOK_NOT_EXISTS";
			break;
		case FAILED_BUFFER_CREATION:
			ErrorString = "FAILED_BUFFER_CREATION";
			break;
		case FAILED_INVALID_PARAMETER:
			ErrorString = "FAILED_INVALID_PARAMETER";
			break;
		case FAILED_ALREADY_EXISTS:
			ErrorString = "FAILED_ALREADY_EXISTS";
			break;
		case FAILED_NOT_EXISTS:
			ErrorString = "FAILED_NOT_EXISTS";
			break;
		case FAILED_FREE_MEMORY:
			ErrorString = "FAILED_FREE_MEMORY";
			break;
		case FAILED_UNHOOK:
			ErrorString = "FAILED_UNHOOK";
			break;
		case FAILED_HOOK:
			ErrorString = "FAILED_HOOK";
			break;
		case FAILED_NOT_ALLOWED:
			ErrorString = "FAILED_NOT_ALLOWED";
			break;
		case FAILED_NOT_HOOKED:
			ErrorString = "FAILED_NOT_HOOKED";
			break;
		case FAILED_ALLOCATION:
			ErrorString = "FAILED_ALLOCATION";
			break;
		case FAILED_NO_ACCESS:
			ErrorString = "FAILED_NO_ACCESS";
			break;
		case FAILED_DISASSEMBLER:
			ErrorString = "FAILED_DISASSEMBLER";
			break;
		case FAILED_MEM_PROTECTION:
			ErrorString = "FAILED_MEM_PROTECTION";
			break;
		case FAILED_MODULE_NOT_FOUND:
			ErrorString = "FAILED_MODULE_NOT_FOUND";
			break;
		case FAILED_FUNCTION_NOT_FOUND:
			ErrorString = "FAILED_FUNCTION_NOT_FOUND";
			break;
		case FAILED_OUT_RANGE:
			ErrorString = "FAILED_OUT_RANGE";
			break;
		case FAILED_TRAMPOLINE_NOT_FOUND:
			ErrorString = "FAILED_TRAMPOLINE_NOT_FOUND";
			break;
		case FAILED_HOOK_STILL_EXISTS_ACCESS_DENIED:
			ErrorString = "FAILED_HOOK_STILL_EXISTS_ACCESS_DENIED";
			break;
		case FAILED_CUSTOM_ORIGINAL_BUFFER_NOT_FOUND:
			ErrorString = "FAILED_CUSTOM_ORIGINAL_BUFFER_NOT_FOUND";
			break;
		default:
			ErrorString = "Unknown error";
			break;
		}
		return ErrorString;
	}
}