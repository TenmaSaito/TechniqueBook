#ifndef _FRAME_H_
#define _FRAME_H_

#include "main.h"

using V3 = D3DXVECTOR3;
using FRAME = float[2];

void InitFrame(V3 pos, V3 rot, FRAME frameSize);
void UninitFrame(void);
void UpdateFrame(void);
void DrawFrame(void);

void SetPositionFrame(V3 pos);
void SetRotationFrame(V3 rot);
void SetSizeFrame(FRAME frameSize);

#endif // !_FRAME_H_

