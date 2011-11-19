#include "CameraRecorder.h"

CameraRecorder::CameraRecorder(void)
{
}

CameraRecorder::~CameraRecorder(void)
{
}

result CameraRecorder::StartRecord(void)
{
	result r = E_SUCCESS;
	AppLog("StartRecord, state:%d", GetState() );

	r = VideoRecorder::Record();

	return r;
}

result CameraRecorder::StopRecord(void)
{
	result r = E_SUCCESS;
	AppLog("StopRecord, state:%d", GetState() );

	r = VideoRecorder::Stop();

	return r;
}

