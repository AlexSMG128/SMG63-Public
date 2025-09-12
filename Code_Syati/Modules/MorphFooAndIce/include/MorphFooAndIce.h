#pragma once

#include "syati.h"

#define MORPHFOO
#define MORPHICE

namespace pt {
	class MorphItemNeoIce : public MorphItemObjNeo {
	public:
		MorphItemNeoIce(const char *pName);

		virtual u32 getPowerUp();
		virtual void emitAppearEffects();
	};

	class MorphItemNeoFoo : public MorphItemObjNeo {
	public:
		MorphItemNeoFoo(const char *pName);

		virtual u32 getPowerUp();
		virtual void emitAppearEffects();
	};
};
