#include "frame.h"

STRUCT()
{
	V3 pos, rot;
	FRAME frameSize;
} Frame;

Frame g_frame;

void InitFrame(V3 pos, V3 rot, FRAME frameSize)
{
	g_frame.pos = pos;
	g_frame.rot = rot;
	g_frame.frameSize[0] = frameSize[0];
	g_frame.frameSize[1] = frameSize[1];
}

void UninitFrame(void)
{

}

void UpdateFrame(void)
{

}

void DrawFrame(void)
{

}

void SetPositionFrame(V3 pos)
{

}

void SetRotationFrame(V3 rot)
{

}

void SetSizeFrame(FRAME frameSize)
{

}