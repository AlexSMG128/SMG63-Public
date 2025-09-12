#include "SuperSpinDriverCustomColors.h"
#include "Game/MapObj/SpinDriverPathDrawInitExt.h"
#include "Game/Util/ActorAnimUtil.h"

/*
* Authors: Originally made by Aurum
* Custom Colors overhaul by Evanbowl and Lord Giganticus
*/

namespace pt {
	namespace {

		const char* gColorsStr[] = {"Red.bti", "Blue.bti", "Rainbow.bti", "Purple.bti", "Black.bti", "White.bti", "Yellow.bti"};
		const size_t numColors = sizeof(gColorsStr)/4;
	
		void initSuperSpinDriverCustomColor(SuperSpinDriver *pActor) {
			s32 color = pActor->mColor;
	
			if (color < 3) {
				pActor->initColor();
	
				if (color == 1) {
					pActor->mSpinDriverPathDrawer->mColor = 0;
					MR::startBtpAndSetFrameAndStop(pActor, "SuperSpinDriver", color);
					MR::startBrk(pActor, "Green");	
				}
			} 
			else {
				MR::startBtpAndSetFrameAndStop(pActor, "SuperSpinDriver", color);
				MR::startBrk(pActor, "Red");
	
				pActor->mSpinDriverPathDrawer->mColor = color;
			}
		}
	
		kmCall(0x8031E29C, initSuperSpinDriverCustomColor); // bl initSuperSpinDriverCustomColor
	
		f32 setSpinDriverPathCustomColor(SpinDriverPathDrawer* pDrawer) {
			if (pDrawer->mColor >= 3) {
				SpinDriverPathDrawInitExt* pDrawInit = (SpinDriverPathDrawInitExt*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_SPIN_DRIVER_PATH_DRAW_INIT);
				pDrawInit->mTextures[pDrawer->mColor - 3]->load(GX_TEXMAP0);
			}
			return pDrawer->calcDrawCode(); // Restore original call
		}
	
		kmCall(0x8030EF28, setSpinDriverPathCustomColor); // bl setSpinDriverPathCustomColor
		kmWrite32(0x8030EF04, 0x48000020); // b 0x20
	
		kmWrite32(0x8031E2A4, 0x60000000); // nop
	}
}

SpinDriverPathDrawInitExt::SpinDriverPathDrawInitExt() : SpinDriverPathDrawInit() {
	mTextures = new JUTTexture*[pt::numColors];

	for (s32 i = 0; i < pt::numColors; i++) {
		mTextures[i] = new JUTTexture(MR::loadTexFromArc("SpinDriverPath.arc", pt::gColorsStr[i], 0), 0);
	}
}

SpinDriverPathDrawInitExt::~SpinDriverPathDrawInitExt() {}

SpinDriverPathDrawInitExt* createSpinDriverPathDrawInitExt() {
	return new SpinDriverPathDrawInitExt();
}

kmCall(0x8045A29C, createSpinDriverPathDrawInitExt); // bl createSpinDriverPathDrawInitExt
kmWrite32(0x8045A2A0, 0x4800067C); // b 0x67C


void CustomSpinDriverDisplayModelColor(DummyDisplayModel* pDisplayModel) {
	MR::startBtpAndSetFrameAndStop(pDisplayModel, "SuperSpinDriver", pDisplayModel->mColorFrame);
	MR::startBrk(pDisplayModel, "Yellow");
}

kmCall(0x802957B8, CustomSpinDriverDisplayModelColor); // bl CustomSpinDriverDisplayModelColor