#pragma once

#include "syati-light.h"
#include "Game/LiveActor/LiveActor.h"
#include "Game/LiveActor/PartsModel.h"
#include "Game/Map/StartCountdownLayout.h"
#include "Game/Player/JetTurtleShadow.h"
#include "Game/Util/ActorCameraUtil.h"
#include "Game/Util/FixedPosition.h"
#include "Game/Player/RaceDataReader.h"
#include "Game/Animation/XanimePlayer.h"

extern void* sInstance__Q213NrvMarioActor19MarioActorNrvNoRush;

/* ==== SMG2 OpenGhost ====
* This Module re-adds GhostPlayer to the game!
* 
* This module does NOT depend on GalaxyGST directly, but you need it to record the Ghost Data (.gst)
* 
* Port by Super Hackio
* Github Repository: 
* 
* 
* ObjArg0 = GstFile ID. Example: Setting it to 4 will load GhostPlayerData04.gst / GhostPlayerData04Luigi.gst. GhostPlayerData00 is the default.
* 
* SW_APPEAR = Activation. Activate the switch to start the race sequence
*/



struct GhostSoundData
{
	const char* pJapanese; //_0
	const char* pSfxName; //_4
	const char* pVoiceName; //_8
};

class GhostPlayer : public LiveActor
{
public:
	GhostPlayer(const char* pName);

	virtual void init(const JMapInfoIter& rIter);

	virtual void draw() const;

	virtual void appear();

	virtual void control();
	virtual void calcAndSetBaseMtx();

	virtual void attackSensor(HitSensor* pSender, HitSensor* pReceiver);

	virtual bool receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver);

	virtual bool receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver);


	void initAnimation();
	bool isRequestSkipDemo() const;
	bool receiveGhostPacket();
	void setAnimation(const char* pName);
	void setAnimationWeight(const f32 fWeight);
	void warpPosition(const char* pName);

	bool isPlayerInPowerStarGet(); //NEW to this port

	static GhostSoundData* playSound(const char*); //ALREADY IN THE GAME

	void exeWait();
	void exePreStartDemo0();
	void exePreStartDemo1();
	void exePreStartDemo2();
	void exeLostDemo();
	void exeWinDemo();


	s32 _8C;
	s32 _90;
	ActorCameraInfo* mCameraInfo; //_94
	s32 _98;
	XanimePlayer* mAnimePlayer; //_9C
	Mtx _A0; //_A0
	f32 _D0; //_D0
	f32 _D4; //_D4
	f32 _D8; //_D8
	f32 _DC; //_DC
	TVec3f mHomePosition; //_E0
	u8 _EC[0x0C]; //_EC... couldn't find this...
	TVec3f _F8; //_F8
	TVec3f _104; //_104
	bool _110; //_110
	bool _111; //_111
	bool _112; //_112
	bool _113; //_113
	bool _114; //_114
	bool _115; //_115 //possibly "isUseJetTurtle"?
	LiveActor* _118; //_118
	JetTurtleShadow* mJetTurtleShadow; //11C
	FixedPosition* mHandRFixedPos; //_120
	s16 _124; //_124
	s16 _126; //_126
	RaceDataReader* mGstFileData; //_128
	StartCountdownLayout* mRaceManagerLayout; //_12C
	CameraTargetMtx* mCameraTargetMtx; //_130
	FixedPosition* mOriginFixedPos;
	PartsModel* mRollingRock; // NEW
	bool mEnableRollingRock; // NEW
};

namespace NrvGhostPlayer
{
	NERVE(HostTypeNrvWait);
	NERVE(HostTypeNrvWinDemo);
	NERVE(HostTypeNrvLostDemo);
	NERVE(HostTypeNrvPreStartDemo0);
	NERVE(HostTypeNrvPreStartDemo1);
	NERVE(HostTypeNrvPreStartDemo2);
	NERVE(HostTypeNrvStartDemo);
	NERVE(HostTypeNrvRun);
}

#define NrvGhostPlayerWait &NrvGhostPlayer::HostTypeNrvWait::sInstance
#define NrvGhostPlayerWinDemo &NrvGhostPlayer::HostTypeNrvWinDemo::sInstance
#define NrvGhostPlayerLostDemo &NrvGhostPlayer::HostTypeNrvLostDemo::sInstance
#define NrvGhostPlayerPreStartDemo0 &NrvGhostPlayer::HostTypeNrvPreStartDemo0::sInstance
#define NrvGhostPlayerPreStartDemo1 &NrvGhostPlayer::HostTypeNrvPreStartDemo1::sInstance
#define NrvGhostPlayerPreStartDemo2 &NrvGhostPlayer::HostTypeNrvPreStartDemo2::sInstance
#define NrvGhostPlayerStartDemo &NrvGhostPlayer::HostTypeNrvStartDemo::sInstance
#define NrvGhostPlayerRun &NrvGhostPlayer::HostTypeNrvRun::sInstance

void raceDataReaderTryPlayActionSoundName(RaceDataReader* pReader, const char* pName);
void raceDataReaderTryPlayActionSoundHash(RaceDataReader* pReader, u32 hash);

bool raceDataReaderIsSound(RaceDataReader* pReader);
void raceDataReaderSetSound(RaceDataReader* pReader, bool toggle);

namespace MR
{
	bool testSubPadReleaseZ(s32);

	bool isGalaxyGhostCometAppearInCurrentStage();
}