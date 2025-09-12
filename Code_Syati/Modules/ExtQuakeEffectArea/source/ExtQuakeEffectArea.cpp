#include "ExtQuakeEffectArea.h"
#include "Game/AreaObj/AreaObj.h"
#include "Game/LiveActor/LiveActor.h"
#include "Game/Scene/SceneObjHolder.h"
#include "Game/Util/LiveActorUtil.h"

/*
* Author: Aurum
*/

namespace pt {
	namespace {
		/*
		* QuakeEffectGenerator never plays the earthquake sound as its sound object is never initialized. This is
		* likely an oversight from when the SMG2 developers upgraded SMG1's sound system since sounds are loaded
		* slightly different in SMG1.
		*/
		void initQuakeEffectGeneratorSound(LiveActor *pActor) {
			MR::invalidateClipping(pActor);
			pActor->initSound(1, "QuakeEffectGenerator", false, TVec3f(0.0f, 0.0f, 0.0f));
		}

		kmCall(0x8026360C, initQuakeEffectGeneratorSound); // redirection hook
	}

	NameObj* createQuakeEffectArea(const char *pName) {
		if (!MR::isExistSceneObj(SCENE_OBJ_QUAKE_EFFECT_GENERATOR)) {
			MR::createSceneObj(SCENE_OBJ_QUAKE_EFFECT_GENERATOR);
		}

		return new AreaObj(pName);
	}
}
