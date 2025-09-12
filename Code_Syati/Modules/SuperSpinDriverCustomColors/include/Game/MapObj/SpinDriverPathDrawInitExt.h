#include "Game/MapObj/SpinDriverPathDrawer.h"


class SpinDriverPathDrawInitExt : public SpinDriverPathDrawInit {
    public:
    SpinDriverPathDrawInitExt();
    virtual ~SpinDriverPathDrawInitExt();

    JUTTexture** mTextures;
};