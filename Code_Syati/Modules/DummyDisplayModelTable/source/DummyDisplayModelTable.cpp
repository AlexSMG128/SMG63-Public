#include "syati.h"
#include "DummyDisplayModelTable.h"

DummyDisplayModelExt::DummyDisplayModelExt(LiveActor *pHost, const DummyDisplayModelInfo *pInfo, int i, s32 l1, s32 l2) :
DummyDisplayModel(pHost, pInfo, i, l1, l2) {
    s32 mRotateType = 0;
    s32 mSetupType = 0;
    s32 mNoLightCtrl = 0;
    s32 mBehaviorType = 0;
}

void* gDummyDisplayModelTable = pt::loadArcAndFile("/SystemData/DummyDisplayModelTable.arc", "/DummyDisplayModelTable.bcsv", 0);
namespace pt {
    
    /*
    * New Dummy items
    *
    * New variants can be easily added and created here. This extends the existing set of dummy models. This replaces
    * all calls to MR::tryCreateDummyModel, but uses the original function to initialize the base game's items.
    */

    DummyDisplayModelExt* tryCreateNewDummyModel(LiveActor *pHost, const JMapInfoIter &rIter, s32 defaultId, int v4) {
        s32 modelId = -1;
        s32 modelParam = 0;

        if (MR::isValidInfo(rIter)) {
            MR::getJMapInfoArg6NoInit(rIter, &modelParam);
            MR::getJMapInfoArg7NoInit(rIter, &modelId);
        }

        if (modelId == -1 && defaultId != 1)
            modelId = defaultId;

        if (modelId == -1)
            return 0;

        JMapInfo table;
        table.attach(gDummyDisplayModelTable); 
    
        s32 line = calcTargetLine(table, modelId);

        s32 setupType;
        MR::getCsvDataS32(&setupType, &table, "SetupType", line);

        #ifdef BLUECOINSYSTEM
        if (setupType == 6) {
            if (BlueCoinUtil::isBlueCoinGotCurrentFile(modelParam)) {
                line = calcTargetLine(table, line+1);
                MR::getCsvDataS32(&setupType, &table, "SetupType", line);
            }
        }
        #endif

        DummyDisplayModelInfo* info = new DummyDisplayModelInfo;
        createInfoFromTable(info, table, line);

        DummyDisplayModelExt* pNewModel = new DummyDisplayModelExt(pHost, info, v4, modelId, modelParam);

        MR::getCsvDataS32(&pNewModel->mRotateType, &table, "RotateType", line);
        MR::getCsvDataS32(&pNewModel->mNoLightCtrl, &table, "NoLightCtrl", line);
        MR::getCsvDataS32(&pNewModel->mBehaviorType, &table, "BehaviorType", line);
        pNewModel->mSetupType = setupType;

        f32 scale;
        MR::getCsvDataF32(&scale, &table, "ScaleValue", line);
        MR::setScale(pNewModel, scale);

        pNewModel->initWithoutIter();
        return pNewModel;
    }

    // Hooks
    kmCall(0x80295BB0, tryCreateNewDummyModel);
    kmCall(0x80295C28, tryCreateNewDummyModel);
    kmCall(0x80295CAC, tryCreateNewDummyModel);
    kmCall(0x80295D04, tryCreateNewDummyModel);
    kmCall(0x80295D88, tryCreateNewDummyModel);

    // Skip repeated reading of Obj_arg7 field
    kmWrite32(0x801D0314, 0x7CA32B78); // mr r3, r5
    kmWrite32(0x801D0318, 0x60000000); // nop

    s32 createInfoFromTable(DummyDisplayModelInfo* pInfo, JMapInfo table, s32 targetLine) {
        MR::getCsvDataStrOrNULL(&pInfo->mModelName, &table, "ModelName", targetLine);
            if (MR::isEqualString(pInfo->mModelName, "0"))
                pInfo->mModelName = 0;

        MR::getCsvDataStrOrNULL(&pInfo->mInitName, &table, "InitName", targetLine);
            if (MR::isEqualString(pInfo->mInitName, "0"))
                pInfo->mInitName = 0;
        
        MR::getCsvDataVec(&pInfo->mOffset, &table, "Offset", targetLine);

        MR::getCsvDataS32(&pInfo->_14, &table, "Param00Int", targetLine);

        MR::getCsvDataStrOrNULL(&pInfo->mAnimName, &table, "AnimName", targetLine);
            if (MR::isEqualString(pInfo->mAnimName, "0"))
                pInfo->mAnimName = 0;

        MR::getCsvDataBool(&pInfo->mHasColorChange, &table, "ColorChange", targetLine);

        return targetLine;
    }

    s32 calcTargetLine(JMapInfo table, s32 modelId) {
        s32 tableIndex = 0;
        s32 targetLine = 0;

        for (s32 i = 0; i < MR::getCsvDataElementNum(&table); i++) {
            MR::getCsvDataS32(&tableIndex, &table, "Index", i);

            if (modelId == tableIndex) {
                targetLine = i;
                break;
            }
        }

        return targetLine;
    }
    // Read and apply RotateType
    kmWrite32(0x80295A10, 0x801E00B4); // lwz r0, 0xB4(r30)
    kmWrite32(0x80295A14, 0x2C000001); // cmpwi r0, 1
    kmWrite32(0x80295A1C, 0x2C000002); // cmpwi r0, 2
    kmWrite32(0x80295A24, 0x48000144); // b 0x144

    kmWrite32(0x802955F0, 0x801B00B4); // lwz r0, 0xB4(r27)
    kmWrite32(0x802955F8, 0x2C000001); // cmpwi r0, 1

    // Read and apply SetupType
    kmWrite32(0x80295704, 0x801D00B8); // lwz r0, 0xB8(r29)
    kmWrite32(0x80295708, 0x2C000001); // cmpwi r0, 1   (Shadow Sphere)
    kmWrite32(0x80295710, 0x2C000002); // cmpwi r0, 2   (PowerStar SetupColor)
    kmWrite32(0x80295718, 0x2C000003); // cmpwi r0, 3   (GrandStar SetupColor)
    kmWrite32(0x80295720, 0x2C000004); // cmpwi r0, 4   (SpinDriver Start Color Anims)
    kmWrite32(0x80295728, 0x2C000005); // cmpwi r0, 5   (Kinopio Create LodCtrl)
    
    // Read and apply CreateLightCtrl
    kmWrite32(0x802956C8, 0x801D00BC); // lwz r0, 0xBC(r29)
    kmWrite32(0x802956CC, 0x2C000001); // cmpwi r0, 1
    kmWrite32(0x802956D4, 0x48000018); // b 0x18

    s32 setControlType(DummyDisplayModelExt* pModel) {
        if (pModel->mRotateType == 2)
            return 1;

        if (pModel->mSetupType == 5)
            return 2;

        return 0;
    }

    kmCall(0x802958E4, setControlType); // bl setControlType
    kmWrite32(0x802958E8, 0x2C030001); // cmpwi r3, 1
    kmWrite32(0x802958F0, 0x2C030002); // cmpwi r3, 2
    kmWrite32(0x802958F4, 0x41820068); // beq 0x68
    kmWrite32(0x802958F8, 0x4800006C); // b 0x6C

    kmWrite32(0x8029595C, 0x807F00AC); // lwz r3, 0xAC(r31) 

    s32 getDummyDisplayModelBehaviorTypeFromTable(const JMapInfoIter& rIter, s32 l) {
        if (MR::isValidInfo(rIter)) {
            s32 modelId = 0;
            MR::getJMapInfoArg7NoInit(rIter, &modelId);

            JMapInfo table;
            table.attach(gDummyDisplayModelTable);

            s32 index = 0;
            s32 targetLine = 0;

            for (s32 i = 0; i < MR::getCsvDataElementNum(&table); i++) {
                MR::getCsvDataS32(&index, &table, "Index", i);

                if (modelId == index) {
                    targetLine = i;
                    break;
                }
            }
            
            s32 behaviorType = 0;
            MR::getCsvDataS32(&behaviorType, &table, "BehaviorType", targetLine);
            return behaviorType;
        }
        return l;
    }
    kmBranch(0x80295DC0, getDummyDisplayModelBehaviorTypeFromTable); // b getDummyDisplayModelIdFromTable

    kmWrite32(0x80295E30, 0x806300C0); // lwz r3, 0xC0(r3)


    void Setup_PowerStarOverrideColor(DummyDisplayModelExt* pModel, NameObj* pHostActor, s32 l) {
        if (pModel->mColorFrame == 0)
            ((PowerStar*)pModel)->setupColor(pHostActor, -1);
        else {
            f32 frame = (f32)pModel->mColorFrame;
            MR::startBtpAndSetFrameAndStop(pModel, "PowerStarColor", frame);
            MR::startBrkAndSetFrameAndStop(pModel, "PowerStarColor", frame);
            MR::startBtkAndSetFrameAndStop(pModel, "PowerStarColor", frame);

            bool isCollected = false;
            int ret = MR::isStarCollected(pHostActor, -1, &isCollected);
            MR::startBvaAndSetFrameAndStop(pModel, "PowerStarColor", (f32)isCollected);
        }
    }
    kmCall(0x8029575C, Setup_PowerStarOverrideColor);
}