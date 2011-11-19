#pragma once

#include <FBase.h>
#include <FGraphics.h>
#include <FMedia.h>
#include <FSystem.h>

class CameraPreview	:
	public Osp::Media::Camera
{
public:
	CameraPreview(void);
	virtual ~CameraPreview(void);

public:
	result StartPreview( CameraStartType startType, const Osp::Graphics::BufferInfo* pBufferInfo );
	result StopPreview(void);
	result StartCapture(void);
	result Initialize(void);
	result Terminate(void);
};
