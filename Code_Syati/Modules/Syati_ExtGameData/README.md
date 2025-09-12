# Syati_ExtGameData
A module that allows the creation of new binary data chunks in GameData.bin.

## Creating a new chunk
To create a new chunk, create a new class that inherits [`BinaryDataChunkBase`](https://github.com/SMGCommunity/Syati/blob/main/include/Game/System/BinaryDataChunkHolder.h).<br>
This new class will manage the reading and writing of new data sections to the user save file.

```c++
class TestChunk : public BinaryDataChunkBase {
public:
    // Ctor should not contain any parameters.
    TestChunk();

    virtual ~TestChunk();

    // Returns a second identifier for this chunk.
    // Note that this value should be static, and should not be calculated based on data values.
    virtual u32 makeHeaderHashCode() const;

    // Returns the signature of this chunk.
    // Should be a four character code.
	virtual u32 getSignature() const;

    // Used for writing data to the save file.
    // The `pPosition` argument is a pointer to the
    // start of the data in the save file.
    // Unsure of what the 2nd argument is used for.
    // Return value is the size of the data written.
    // Note that you don't need to write the signature and hash yourself.
	virtual u32 serialize(u8 *pPosition, u32) const;

    // Used for reading data from the save file.
    // The `pPosition` argument is a pointer to the
    // start of the data in the save file.
    // The `size` argument is the size of this chunk.
    // Return value appears to be a return code. 0 = success.
	virtual u32 deserialize(const u8 *pPosition, u32 size) const;

    // Used for initializing this chunk's member variables.
	virtual void initializeData();
}
```

## Registering the new chunk
To register this new chunk, you will need to add an entry to your `ModuleInfo.json`'s `ModuleData` entries:

```json
"NewSaveChunk": [
    {
      "ChunkMemberType": "TestChunk",
      "ChunkMemberName": "mTestChunk",
      "ChunkCreationFunc": "createBinaryChunk<TestChunk>",
      "Include": "TestChunk.h"
    }
]
```

Furthermore, you can add `Syati_ExtGameData_API` to your `ModuleDependancies` (typo intentional, please fix this hackio :D) and include `ExtGameDataUtil.h` to access the current extended `GameDataHolder` instance. With this, you can make a simple utility function to easly access the custom chunk from anywhere:
```c++
TestChunk* getCurrentTestChunk() {
    ExtGameDataHolder *pHolder = ExtGameDataUtil::getCurrentGameDataHolder();

    if (pHolder) {
        return pHolder->mTestChunk;
    }

    return NULL;
}
```