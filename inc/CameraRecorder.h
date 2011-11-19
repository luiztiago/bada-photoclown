
#ifndef CAMERARECORDER_H_
#define CAMERARECORDER_H_

#include <FBase.h>
#include <FGraphics.h>
#include <FMedia.h>

class CameraRecorder	:
	public Osp::Media::VideoRecorder
{
public:
	CameraRecorder(void);
	virtual ~CameraRecorder(void);

public:
	result StartRecord(void);
	result StopRecord(void);
};

#endif // CAMERARECORDER_H_ 
