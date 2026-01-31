# Donut Blocks
Brings the Donut Blocks from Super Mario 3D Land/World into SMG2.
If given a path ID, the platform will follow said path.

## Obj_args
|   |Type|Name|Description|Default
|---|---|--- |---|---
|0|Int|Fall Delay|The time it takes for the platform to fall after the player steps on it.|25
|1|Int|Fall Duration|The time it takes for the platform to disappear after falling.|120
|2|Int|Respawn Delay|The time it takes for the platform to respawn after disappear.|100
|3|Float|Fall Speed|The speed the platform falls at.|5.0
|4|Float|Rail Speed|The speed the platform moves on a path at.|5.0
|5|Int|Rail Delay|The time the platform stays still for when it reaches the start/end of a path.|0

## Action Sound
This platform uses custom sounds You must add them yourself to ActionSound using any bcsv editor.
```csv
DonutBlock,OjBlockAppear,SE_OJ_BLOCK_APPEAR,,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0,0,-1,-1,0,-1,-1
```

## Effects
This platform uses custom particles. You must import them yourself using pygapa.
```json
[
    {
        "GroupName": "DonutBlock",
        "UniqueName": "Smoke",
        "EffectName": [
            "DeathSmoke00",
            "DeathSmoke01"
        ],
        "OffsetX": 0.0,
        "OffsetY": -20.0,
        "OffsetZ": 0.0,
        "ScaleValue": 0.7,
        "RateValue": 1.0,
        "LightAffectValue": 0.0,
        "DrawOrder": "3D",
        "Affect": [
            "T"
        ]
    }
]
```