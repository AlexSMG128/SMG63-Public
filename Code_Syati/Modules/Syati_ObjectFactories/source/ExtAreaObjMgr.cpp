#include "Game/AreaObj/AreaObjContainer.h"
#include "ModuleData_ExtAreaObjMgr.h"
void ExtAreaObjManagers(AreaObjMgr* pManager, AreaObjContainer* pContainer, const JMapInfoIter& rIter, s32 l) {
    pManager->init(rIter);
    pContainer->mManagers.assign(pManager, pContainer->mNumManagers++);

    if (l == 94) {
        for (s32 i = 1; i < cModuleCreateAreaObjMgrTableCount; i++) {
            ManagerEntry* entry = &cModuleCreateAreaObjMgrTable[i];
            pManager = entry->mCreationFunc(entry->mMaxAreas, entry->pManagerName);
            pManager->init(rIter);
            pContainer->mManagers.assign(pManager, pContainer->mNumManagers++);
        }
    }
}

kmWrite32(0x80071318, 0x3860017C + cModuleCreateAreaObjMgrTableCount*4); // li r3, 0x17C + cModuleCreateAreaObjMgrTableCount*4
kmWrite32(0x80071320, 0x3800005F + cModuleCreateAreaObjMgrTableCount); // li r0, 0x5F + cModuleCreateAreaObjMgrTableCount

kmWrite32(0x8007136C, 0x7F24CB78); // mr r4, r25
kmWrite32(0x80071370, 0x38A10008); // addi r5, r1, 8
kmWrite32(0x80071374, 0x7F66DB78); // mr r6, r27
kmCall(0x80071378, ExtAreaObjManagers); // bl ExtAreaObjManagers
kmWrite32(0x8007137C, 0x48000018); // b 0x18

kmWrite32(0x80071394, 0x3B7B0001); // addi r27, r27, 1
kmWrite32(0x80071398, 0x2C1B005F); // cmpwi r27, 0x5F
kmWrite32(0x8007139C, 0x3BFF000C); // addi r31, r31, 0xC