#pragma once
#include <cinttypes>

#define TARGET_GAMEVERSION "Version 10.21.339.2173 [PUBLIC]"  // 

namespace offsets {
	namespace other {
		extern uint32_t objectArray;
	};

	namespace global {
		extern uint32_t oRetAddr;
		extern uint32_t oDrawCircleRetAddr;
		extern uint32_t oNetClient;
		extern uint32_t oChatClientPtr;
		extern uint32_t oObjManager;
		extern uint32_t oZoomClass;
		extern uint32_t oGameInfo;
		extern uint32_t oHudInstance;
		extern uint32_t oRenderer;
		extern uint32_t oUnderMouseObject;
		extern uint32_t oD3DRenderer;
		extern uint32_t oWorldToScreen;
		extern uint32_t oLocalPlayer;
		extern uint32_t oGameTime;
		extern uint32_t oGetFirstObject;
		extern uint32_t oGetNextObject;
		extern uint32_t oGameVersion;
		extern uint32_t oMenuGUI;
		extern uint32_t oHeroList;
		extern uint32_t oViewMatrix;
		/*extern uint32_t Player;
		extern uint32_t ChampionManager;
		extern uint32_t Riot__g_window;
		extern uint32_t GfxWinMsgProc;
		extern uint32_t ManagerTemplate_AIMinionClient_;
		extern uint32_t ManagerTemplate_AIHero_;
		extern uint32_t GameClient;*/
	};

	namespace cobject {
		extern uint32_t oObjIndex;
		extern uint32_t oObjTeam;
		extern uint32_t oObjName;
		extern uint32_t oObjNetworkID;
		extern uint32_t oObjPos;
		extern uint32_t oObjVisibility;
		extern uint32_t oObjSourceIndex;
		extern uint32_t oObjHealth;
		extern uint32_t oObjMaxHealth;
		extern uint32_t oObjMana;
		extern uint32_t oObjMaxMana;
		extern uint32_t oObjArmor;
		extern uint32_t oObjMagicRes;
		extern uint32_t oObjBonusMagicRes;
		extern uint32_t oObjBaseAtk;
		extern uint32_t oObjBonusAtk;
		extern uint32_t oObjMoveSpeed;
		extern uint32_t oObjAtkRange;
		extern uint32_t oObjBuffMgr;
		extern uint32_t oObjSpellBook;
		extern uint32_t oObjChampionName;
		extern uint32_t oObjLevel;
		extern uint32_t oObjAIManager;

		extern uint32_t oMissileSpellInfo;
		extern uint32_t oMissileSourceIndex;
		extern uint32_t oMissileTargetIndex;
		extern uint32_t oMissileStartPos;
		extern uint32_t oMissileEndPos;
	};

	namespace functions {
		extern uint32_t oCastSpell;
		extern uint32_t oDrawCircle;
		extern uint32_t oGetBasicAttack;
		extern uint32_t oGetAttackCastDelay;
		extern uint32_t oGetAttackDelay;
		extern uint32_t oGetPing;
		extern uint32_t oPingInstance;
		extern uint32_t oGetSpellState;
		extern uint32_t oIsTargetable;
		extern uint32_t oIsAlive;
		extern uint32_t oIsBaron;
		extern uint32_t oIsTurret;
		extern uint32_t oIsInhib;
		extern uint32_t oIsHero;
		extern uint32_t oIsMinion;
		extern uint32_t oIsDragon;
		extern uint32_t oIsMissile;
		extern uint32_t oIsNexus;
		extern uint32_t oIsNotWall;
		extern uint32_t oIsTroy;
		extern uint32_t oIssueOrder;
		extern uint32_t oPrintChat;
		extern uint32_t oSendChat;
		//extern uint32_t oGetTimerExpiry;
		/*extern uint32_t Riot__Renderer__MaterialRegistry__GetSingletonPtr;
		extern uint32_t translateString_UNSAFE_DONOTUSE;
		extern uint32_t CharacterDataStack__Push;
		extern uint32_t CharacterDataStack__Update;
		extern uint32_t GetOwnerObject;
		extern uint32_t CharacterData__GetCharacterPackage;*/
	};

	namespace buff {
		//buff
		extern uint32_t O_BUFFMGR_BUFFNAME;
		extern uint32_t O_BUFFMGR_STARTTIME;
		extern uint32_t O_BUFFMGR_ENDTIME;
		extern uint32_t O_BUFFMGR_flBUFFCOUNT;
		extern uint32_t O_BUFFMGR_iBUFFCOUNT;
		extern uint32_t oBuffType;
		extern uint32_t oBuffCountAlt;
		extern uint32_t oBuffCountAlt2;
	};

	namespace aimgr {
		extern uint32_t oAIMGR_TargetPos;
		extern uint32_t oAIMGR_IsMoving;
		extern uint32_t oAIMGR_IsDashing;
		extern uint32_t oAIMGR_NavBegin;
		extern uint32_t oAIMGR_NavEnd;
		extern uint32_t oAIMGR_CurrentPos;
		extern uint32_t oAIMGR_Velocity;
	};

	namespace stat {
		extern uint32_t AIBASE_STATOWNER;
		extern uint32_t STATOWNER__GETSTATTOTAL;
	};

	/*namespace ai_base {
		extern uint32_t CharacterDataStack;
		extern uint32_t SkinId;
	};*/

	/*namespace material_registry {
		extern uint32_t D3DDevice;
		extern uint32_t SwapChain;
	};

	namespace ai_minion {
		extern uint32_t IsLaneMinion;
	};*/
};
