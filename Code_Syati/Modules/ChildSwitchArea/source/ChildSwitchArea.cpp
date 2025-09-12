#include "ChildSwitchArea.h"

/*
 * Author: AwesomeTMC
 * Object: ChildSwitchArea
 * Description: An area that activates when the specified amount of child objects connected is in the area.
 * Note: Only 32 can be used in a level before it crashes.
 *
 *  SW_A (required): Activates when the specified amount of child objects connected is in the area.
 *
 *  name, type, default (-1)
 *  Obj_arg0, int, All: Amount to compare with.
 *  Obj_arg1, list, Equal: Sign to use
 *  - 0: no. of child objects = obj_arg0
 *  - 1: no. of child objects < obj_arg0
 *  - 2: no. of child objects > obj_arg0
 *  - 3: no. of child objects ≠ obj_arg0
 *  Obj_arg2, bool, false: Deactivate when no longer true?
 */
namespace pt
{
    ChildSwitchArea::ChildSwitchArea(const char *pName) : AreaObj(pName)
    {
        mActive = false;
        mChildAmount = 0;
    }

    void ChildSwitchArea::init(const JMapInfoIter &rIter)
    {
        AreaObj::init(rIter);
        MR::connectToSceneAreaObj(this);

        // Add child obj to group and initialize them
        mChildAmount = MR::getChildObjNum(rIter);
        pGroup = new LiveActorGroup("ChildSwitchGroup", mChildAmount);
        for (int i = 0; i < mChildAmount; i++)
        {
            // Nintendo seems to do this with the Extermination Switch as well.
            pGroup->registerActor((LiveActor *)NameObjFactory::initChildObj(rIter, i));
        }

        if (mObjArg0 == -1)
            mObjArg0 = mChildAmount;
    }

    void ChildSwitchArea::movement()
    {
        // Get number of child obj in area
        int amountInArea = 0;
        for (int i = 0; i < mChildAmount; i++)
        {
            LiveActor *actor = pGroup->getActor(i);
            if (isInVolume(actor->mTranslation) && !MR::isDead(actor))
                amountInArea++;
        }

        // Use user given sign to compare with.
        bool activate = false;
        switch (mObjArg1)
        {
        case 1:
            activate = (amountInArea < mObjArg0);
            break;
        case 2:
            activate = (amountInArea > mObjArg0);
            break;
        case 3:
            activate = (amountInArea != mObjArg0);
            break;
        default: // case 0
            activate = (amountInArea == mObjArg0);
            break;
        }

        if (activate && !mStageSwitchCtrl->isOnSwitchA())
            onSwitchA();
        else if (!activate && mStageSwitchCtrl->isOnSwitchA() && mObjArg2 >= 1)
            offSwitchA();
    }

    const char *ChildSwitchArea::getManagerName() const
    {
        return "ChildSwitchArea";
    }
}
