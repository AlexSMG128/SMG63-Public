#include "syati.h"
#include "Game/NPC/TalkMessageCtrl.h"
#include "Game/NPC/TalkNodeCtrl.h"
#include "Game/Screen/GameSceneLayoutHolder.h"

#include "JSystem/JUtility/JUTException.h"
#include "JSystem/JUtility/JUTConsole.h"
#include "JSystem/JUtility/JUTConsoleManager.h"

/*
* Error Message Fallback
* PictureFont Redirection
* Custom Flag Textures
* Custom HipDropSwitch Colors
* Print File Name if Missing
* Yes/No Dialogue Extensions
* Appear Custom Coin Types on Dark Comets
* Custom Green Star Scenario Names
* Death Area Extensions
* Repeat Timer Switch Sound Effects Control
*/
namespace pt {
	/*
	* Error Message Fallback
	*
	* Missing text will usualy crash the game. This small patch will display the text label whose message data could not be
	* located instead of nothing. This tells developers what text is missing.
	*/
	static wchar_t cErrorMessageBuffer[128]; // Buffer for missing text label. Last wchar should always be NULL.

	const wchar_t* getErrorMessage(const char *pLabel) {
		asm("mr %0, r31" : "=r" (pLabel));

		size_t len = strlen(pLabel) + 1;

		if (len > 127) {
			len = 127;
		}

		mbstowcs(cErrorMessageBuffer, pLabel, len);

		return cErrorMessageBuffer;
	}

	kmCall(0x800413F0, getErrorMessage); // MR::getGameMessageDirect will return the error message instead of NULL

	/*
	* Custom Flag Textures
	*
	* Requires a BTI added to the target Flag.arc ending with a number.
	* This number is what Obj_arg0 will select.
	* 
	* Setting Obj_arg0 to 4 would mean that...
	* (FlagName(4)).bti would be selected in (FlagName).arc
	*/

	JUTTexture* CustomFlagTextures(LiveActor* pActor, const char* pStr, const JMapInfoIter& rIter) {
		s32 flagTex = 0;
		char outArcStr[24];
		char outTexStr[24];
		MR::getJMapInfoArg0NoInit(rIter, &flagTex);

		snprintf(outArcStr, 24, "%s.arc", pStr);

		if (flagTex < 1)
			snprintf(outTexStr, 24, "%s.bti", pStr);
		else
			snprintf(outTexStr, 24, "%s%d.bti", pStr, flagTex);
			
		return MR::loadTexFromArc(pActor, outArcStr, outTexStr);
	}

	kmWrite32(0x80254880, 0x60000000); // nop
	kmWrite32(0x80254884, 0x7FA5EB78); // mr r5, r29
	kmCall(0x8025488C, CustomFlagTextures);


	void customHipDropSwitchColors(LiveActor* actor, const JMapInfoIter& iter) {
		MR::needStageSwitchWriteA(actor, iter);
	
		f32 frame = 0;
		MR::getJMapInfoArg1NoInit(iter, &frame);
		MR::startBtpAndSetFrameAndStop(actor, "ButtonColor", frame);
	}
	
	kmCall(0x802AF524, customHipDropSwitchColors);
	
	/*
	* Debugging feature: displaying the names of missing files on the "File isn't exist" error.
	*
	* When the game attempts to load a file into memory, it runs MR::isFileExist to check for the file, and if the file it's checking
	* for doesn't exist, it calls OSFatal, crashing the game. It also prints "File isn't exist" to the log.
	*
	* Here, the MR::isFileExist call is replaced with a call to this new function, that prints the file name with the error, if the checked file is missing.
	*
	* This is useful for debugging certain things!
	*/

	void printFileNameIfMissing(const char* fileName) {
		if (!MR::isFileExist(fileName, 0))
			OSPanic("FileRipper.cpp", 118, "File \"%s\" isn't exist.", fileName);
	}

	#if defined TWN || defined KOR
		kmCall(0x804B2050, printFileNameIfMissing);
	#else
		kmCall(0x804B1FE0, printFileNameIfMissing);
	#endif

	extern "C" {
		void __kAutoMap_80059F90(char*, char*);
	}

	void YesNoDialogueExtensions(int* pTalkDirector, const TalkMessageCtrl* pCtrl) {
		u32* pNodeBranch = (u32*)pCtrl->mTalkNodeCtrl->getNextNodeBranch();
		u16 selectTxt = *((u16*)pNodeBranch+4);

		char bufName[32];
		
		if (selectTxt < 18) {
			const char* pStr = pCtrl->getBranchID();
			MR::copyString(bufName, pStr, strlen(pStr)+1);
		}
		else {
			char bufNew[10];
			snprintf(bufNew, 10, "New%d", selectTxt - 18);
			MR::copyString(bufName, bufNew, 32);
		}

		char bufYes[256];
		char bufNo[256];

		snprintf(bufYes, 256, "Select_%s_Yes", bufName);
		snprintf(bufNo, 256, "Select_%s_No", bufName);

		if (pCtrl->isSelectYesNo()) {
			MR::resetYesNoSelectorSE();
		}
		else {
			const char* pSeStr = "SE_SY_TALK_SELECT";
			MR::setYesNoSelectorSE("SE_SY_TALK_FOCUS_ITEM", pSeStr, pSeStr);
		}
		LayoutActor* pLayout = (LayoutActor*)MR::getGameSceneLayoutHolder()->mYesNoLayout;
		MR::requestMovementOn(pLayout);
		__kAutoMap_80059F90(bufYes, bufNo);
	}

	kmCall(0x80379F84, YesNoDialogueExtensions);

	const wchar_t* CustomGreenStarNames(GalaxyStatusAccessor accessor, const char* pStageName, s32 starid) {
		char textName[256];
	    snprintf(textName, 256, "ScenarioName_%s_GreenStar%d", pStageName, starid-accessor.getPowerStarNum()/2);
	
	    TalkMessageInfo info;
	    MessageSystem::getGameMessageDirect(&info, textName);
	
	    if (info.mMessage) {
	        return info.mMessage;
		}
	
	    snprintf(textName, 256, "ScenarioName_GreenStar%d", starid-accessor.getPowerStarNum()/2);
	    return MR::getGameMessageDirect(textName);
	}
	
	kmWrite32(0x8004159C, 0x7FC4F378); // mr r4, r30
	kmWrite32(0x800415A0, 0x7FE5FB78); // mr r5, r31
	kmCall(0x800415A4, CustomGreenStarNames);
	kmWrite32(0x800415A8, 0x48000050); // b 0x50


	/*
	* Mini Patch: Swimming Death Area
	* 
	* This patch is really useless but I thought it would be nice to include.
	* For example, this could be used to make instant death water/lava.
	*/

	void DeathAreaExtensions(DeathArea* area) {
		if (area->mObjArg0 < 0) {
			MR::getGameSceneLayoutHolder()->mMarioSubMeter->mAirMeter->mLayoutActorFlag.mIsHidden = area->isInVolume(*MR::getPlayerPos());

			bool checkForSwimming;

			if (area->mObjArg1 == -1)
				checkForSwimming = false;
			else
				checkForSwimming = true;

			if (area->isInVolume(*MR::getPlayerPos()) && checkForSwimming ? MR::isPlayerSwimming() : true) 
				MarioAccess::forceKill(checkForSwimming ? 3 : 0, 0);
		}
	}

	kmCall(0x8007401C, DeathAreaExtensions);
	kmWrite32(0x8007402C, 0x60000000);


	bool isInDeathFix(const char* name, const TVec3f& pos) {
		if (MR::isInAreaObj(name, pos))
			if (MR::getAreaObj(name, pos)->mObjArg1 == -1 || MR::isInWater(pos))
				return true;

		return false;
	}

	kmBranch(0x8004AC1C, isInDeathFix);


    // Repeat Timer Switch Sound Effect Control

    kmWrite32(0x80342BA4, 0x3860002C); // li r3, 0x2C
    
    void RepeatTimerSwitchGetNewArg(LiveActor* pActor, const JMapInfoIter& rIter) {
        ((s32*)pActor)[0xA] = -1; // stw r0, 0x28(r3)
        MR::connectToSceneMapObjMovement(pActor);
        MR::getJMapInfoArg2NoInit(rIter, &((s32*)pActor)[0xA]); // addi r4, r30, 0x28
    }

    kmCall(0x80266B5C, RepeatTimerSwitchGetNewArg);

    asm void RepeatTimerSwitchPlaySound() {
        lwz r3, 0x28(r28)
        cmpwi r3, -1
        bne dontplaysound
        b playsound

        playsound:
        lwz r3, 0x24(r28)
        li r5, -1
        li r6, -1
        b end

        dontplaysound:
        li r3, 0
        b end

        end:
        blr
    }

    kmCall(0x80266C8C, RepeatTimerSwitchPlaySound);
    kmWrite32(0x80266C94, 0x2C030000); // cmpwi r3, 0
    kmWrite32(0x80266C98, 0x4182005C); // beq 0x5C

    kmCall(0x80266CC0, RepeatTimerSwitchPlaySound);
    kmWrite32(0x80266CC8, 0x2C030000); // cmpwi r3, 0
    kmWrite32(0x80266CCC, 0x41820028); // beq 0x28

    kmCall(0x80266CE0, RepeatTimerSwitchPlaySound);
    kmWrite32(0x80266CE8, 0x2C030000); // cmpwi r3, 0
    kmWrite32(0x80266CEC, 0x41820008); // beq 0x8

	// Allow Skeeters to be killed by a fire ball

	bool menboFireBallCheck (u32 msg) {
		return msg == ACTMES_FIREBALL_ATTACK || MR::isMsgJetTurtleAttack(msg);
	}

	kmCall(0x801C5008, menboFireBallCheck);

	// Allow Piranha Plants to be blown up by a Bob-omb

	bool packunPetitExplosionCheck (u32 msg) {
		return msg == ACTMES_ENEMY_ATTACK_EXPLOSION || MR::isMsgToEnemyAttackBlow(msg);
	}

	kmCall(0x801E3A04, packunPetitExplosionCheck);

	void funcTest(JUTConsole* pConsole, const char* pStr) {
		const char* region = 0;

		#ifdef USA
			region = "USA";
		#elif PAL
			region = "PAL";
		#elif JPN
			region = "JPN";
		#elif TWN
			region = "TWN";
		#elif KOR
			region = "KOR";
		#endif

		pConsole->print("-------------------------------- SYATI\n");
		pConsole->print_f("Build Date: %s\nBuild Time: %s\n", __DATE__, __TIME__);
		pConsole->print_f("Region: %s\n", region);
		pConsole->print("--------------------------------\n");
	}
	kmCall(0x80510578, funcTest);

	void DisableAllSpin(MarioActor* pMarioActor) {
		if (!pMarioActor->mMario)
			return;

		bool b = *((bool*)pMarioActor->mMario+0xA78);
		
		if (b)
			return;

		pMarioActor->updateControllerSwing();
		pMarioActor->updateControllerSwingLeft();
	}
	kmCall(0x803C20F8, DisableAllSpin);
	kmWrite32(0x803C20FC, 0x48000008);
}