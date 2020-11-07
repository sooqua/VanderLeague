#include "Offsets.h"

namespace offsets {
	namespace other {
		uint32_t objectArray = 0x14;
	}

	namespace global {
		uint32_t oRetAddr = 0x6E502; // 				// find addr : ret // opcode : C3
		uint32_t oDrawCircleRetAddr = 0x6E4FF; // 		// find addr : add esp, 28 ret // opcode : 83 C4 1C C3

		uint32_t oNetClient = 0x351F884; // 			8B 0D ? ? ? ? 85 C9 74 07 8B 01 6A 01 FF 50 08 8B // dword_[offset]
		uint32_t oChatClientPtr = 0x1C79590; //			8B 0D ? ? ? ? 6A 00 50 E8 ? ? ? ? 33 // dword_[offset]
		uint32_t oObjManager = 0x1C7BB18; //			8B 0D ? ? ? ? 89 74 24 14 // dword_[offset]
		uint32_t oZoomClass = 0x350FF54; //				A3 ? ? ? ? 83 FA 10 72 32 // dword_[offset]
		uint32_t oGameInfo = 0x35118B0; // 				A1 ? ? ? ? 83 78 08 02 0F 94 C0 // dword_[offset]
		uint32_t oHudInstance = 0x1C7BB54; //			8B 0D ? ? ? ? 6A 00 8B 49 34 E8 ? ? ? ? B0 // dword_[offset]
		uint32_t oRenderer = 0x3540C74; // 				8B 15 ? ? ? ? 83 EC 08 F3 // dword_[offset]
		uint32_t oUnderMouseObject = 0x1C1B0F0; // 		8B 0D ? ? ? ? 89 0D // dword_[offset] // from 34E65C0 to 1C072C0 ?
		uint32_t oD3DRenderer = 0x35439B8; // 			A1 ? ? ? ? 89 54 24 18 // dword_[offset]
		uint32_t oWorldToScreen = 0x986720; // 			83 EC 10 56 E8 ? ? ? ? 8B 08 // fn
		uint32_t oLocalPlayer = 0x3518034; //			A1 ? ? ? ? 85 C0 74 07 05 ? ? ? ? EB 02 33 C0 56 // dword_[offset]
		uint32_t oGameTime = 0x3510778; // 				F3 0F 11 05 ? ? ? ? 8B 49 // dword_[offset]
		uint32_t oGetFirstObject = 0x288160; //			8B 41 14 8B 51 18 // fn
		uint32_t oGetNextObject = 0x289140; //			E8 ? ? ? ? 8B F0 85 F6 75 E4 // sub_[offset]
		uint32_t oGameVersion = 0x56C740; //			8B 44 24 04 BA ? ? ? ? 2B D0 // fn
		uint32_t oMenuGUI = 0x3518004; // 				8B 0D ? ? ? ? 6A 00 E8 ? ? ? ? C7 // dword_[offset]
		uint32_t oHeroList = 0x1C79A48;
		uint32_t oMinionList = 0x28C8100;
		uint32_t oTurretList = 0x350F720;
		uint32_t oInhibitorList = 0x35191C0;
		uint32_t oViewMatrix = 0x353DF38;
		/*uint32_t Player = 0x34E0280;
		uint32_t ChampionManager = 0x034D93A4;
		uint32_t Riot__g_window = 0x34E026C;
		uint32_t GfxWinMsgProc = 0x34FF420;
		uint32_t ManagerTemplate_AIMinionClient_ = 0x1C3D790;
		uint32_t ManagerTemplate_AIHero_ = 0x01C3B63C;
		uint32_t GameClient = 0x34D93B4;*/
	};

	namespace cobject {
		uint32_t oObjIndex = 0x20;
		uint32_t oObjTeam = 0x4C;
		uint32_t oObjName = 0x6C;
		uint32_t oObjNetworkID = 0xCC;
		uint32_t oObjPos = 0x220;
		uint32_t oObjVisibility = 0x270;
		uint32_t oObjSourceIndex = 0x290;
		uint32_t oObjHealth = 0xDC4;
		uint32_t oObjMaxHealth = 0xDD4;
		uint32_t oObjMana = 0x298;
		uint32_t oObjMaxMana = 0x2A8;
		uint32_t oObjArmor = 0x12B0;
		uint32_t oObjMagicRes = 0x12B8;
		uint32_t oObjBonusMagicRes = 0x12BC;
		uint32_t oObjBaseAtk = 0x1288;
		uint32_t oObjBonusAtk = 0x1208;
		uint32_t oObjMoveSpeed = 0x12C8;
		uint32_t oObjAtkRange = 0x12D0;
		uint32_t oObjBuffMgr = 0x2174;
		uint32_t oObjSpellBook = 0x2720;
		uint32_t oObjChampionName = 0x314C;
		uint32_t oObjLevel = 0x36BC;
		uint32_t oObjAIManager = 0x3030;

		uint32_t oMissileSpellInfo = 0x230;
		uint32_t oMissileSourceIndex = 0x290;
		uint32_t oMissileTargetIndex = 0x2E8;
		uint32_t oMissileStartPos = 0x2A8;
		uint32_t oMissileEndPos = 0x2B4;
	}

	namespace functions {
		uint32_t oCastSpell = 0x50F1D0; // 				xref: ERROR: Client Tried to cast a spell from an invalid slot: %d.\n
		uint32_t oDrawCircle = 0x503470; //			 	E8 ? ? ? ? 83 C4 1C 80 7F // sub_[offset]
		uint32_t oGetBasicAttack = 0x166970; // 	 	53 8B D9 B8 ? ? ? ? 8B 93 // fn
		uint32_t oGetAttackCastDelay = 0x2A5020; // 	83 EC 0C 53 8B 5C 24 14 8B CB 56 57 8B 03 FF 90 // fn
		uint32_t oGetAttackDelay = 0x2A5120; // 		E8 ? ? ? ? 8B 44 24 1C 83 C4 0C 8B CE // sub_[offset]
		uint32_t oGetPing = 0x335730; // 				E8 ? ? ? ? 8B 4F 2C 85 C9 0F // sub_[offset]
		uint32_t oPingInstance = 0x351F884; //			8B 0D ? ? ? ? 85 C9 74 07 8B 01 6A 01 FF 50 08 8B // dword_[offset]
		uint32_t oGetSpellState = 0x5004F0; // 			E8 ? ? ? ? 8B F8 8B CB 89 // sub_[offset]
		uint32_t oIsTargetable = 0x1D4070; //			56 8B F1 E8 ? ? ? ? 84 C0 74 2E // fn
		uint32_t oIsAlive = 0x19F2A0; // 				56 8B F1 8B 06 8B 80 ? ? ? ? FF D0 84 C0 74 19 // fn
		uint32_t oIsBaron = 0x173B20; //				E8 ? ? ? ? 84 C0 74 0A BB // sub_[offset]
		uint32_t oIsTurret = 0x1ADD20; // 				E8 ? ? ? ? 83 C4 04 84 C0 74 09 5F // sub_[offset]
		uint32_t oIsInhib = 0x1AD930; // 				E8 ? ? ? ? 83 C4 04 84 C0 74 38 // sub_[offset]
		uint32_t oIsHero = 0x1ADAB0; // 				E8 ? ? ? ? 83 C4 04 84 C0 74 2B 57 // sub_[offset]
		uint32_t oIsMinion = 0x1ADAF0; // 				E8 ? ? ? ? 83 C4 04 84 C0 74 16 8B 0F // sub_[offset]
		uint32_t oIsDragon = 0x171C70; //  				E8 ? ? ? ? 33 DB 84 C0 0F 95 C3 8D 1C 5D ? ? ? ? E9 // sub_[offset]
		uint32_t oIsMissile = 0x1ADB10; //		   		E8 ? ? ? ? 83 C4 04 84 C0 74 60 // sub_[offset]
		uint32_t oIsNexus = 0x1ADA30; //				E8 ? ? ? ? 83 C4 04 84 C0 0F 95 C0 C3 CC 8A // sub_[offset]
		uint32_t oIsNotWall = 0x9036A0; // 				E8 ? ? ? ? 33 C9 83 C4 0C 84 // sub_[offset]
		uint32_t oIsTroy = 0x1AD9B0;  // 				E8 ? ? ? ? 8B E8 83 C4 04 85 ED 0F 84 ? ? ? ? 6A 08 // go to sub_XXX then sub_[offset]
		uint32_t oIssueOrder = 0x173BD0; // 			81 EC ? ? ? ? 56 57 8B F9 C7 // fn
		uint32_t oPrintChat = 0x57EA20; //				E8 ? ? ? ? 33 C0 5F C2 // sub_[offset]
		uint32_t oSendChat = 0x5FB9D0; //				A1 ? ? ? ? 56 6A FF // fn // E8 ? ? ? ? 8D 4C 24 04 C6 // sub_[offset]
		//uint32_t oGetTimerExpiry = 0x16C420; // 		E8 ? ? ? ? 51 D9 1C 24 E8 ? ? ? ? 8B // sub_[offset]
		/*uint32_t Riot__Renderer__MaterialRegistry__GetSingletonPtr = 0x0099A6C0;
		uint32_t translateString_UNSAFE_DONOTUSE = 0x00A7E720;
		uint32_t CharacterDataStack__Push = 0x0013FD00;
		uint32_t CharacterDataStack__Update = 0x00127B70;
		uint32_t GetOwnerObject = 0x001738E0;
		uint32_t CharacterData__GetCharacterPackage = 0x00233260;*/
	};

	namespace buff {
		uint32_t O_BUFFMGR_BUFFNAME = 0x08;
		uint32_t O_BUFFMGR_STARTTIME = 0xC;
		uint32_t O_BUFFMGR_ENDTIME = 0x10;
		uint32_t O_BUFFMGR_flBUFFCOUNT = 0x130;
		uint32_t O_BUFFMGR_iBUFFCOUNT = 0x74;
		uint32_t oBuffType = 0x4;
		uint32_t oBuffCountAlt = 0x24;
		uint32_t oBuffCountAlt2 = 0x20;
	};

	namespace aimgr {
		uint32_t oAIMGR_TargetPos = 0x10;
		uint32_t oAIMGR_IsMoving = 0x198;
		uint32_t oAIMGR_IsDashing = 0x1E8;
		uint32_t oAIMGR_NavBegin = 0x1BC;
		uint32_t oAIMGR_NavEnd = 0x1FC;
		uint32_t oAIMGR_CurrentPos = 0x2B4;
		uint32_t oAIMGR_Velocity = 0x2C8;
	};

	namespace stat {
		uint32_t AIBASE_STATOWNER = 0x22C;
		uint32_t STATOWNER__GETSTATTOTAL = 0x1BE170;
	};

	/*namespace ai_base {
		uint32_t CharacterDataStack = 0x2F58;
		uint32_t SkinId = 0xED4;
	};

	namespace material_registry {
		uint32_t D3DDevice = 0x200;
		uint32_t SwapChain = 0x10C;
	};

	namespace ai_minion {
		uint32_t IsLaneMinion = 0x37A0;
	};*/
};