#include "CameraCapture.h"
#include "CameraCapturePreview.h"

using namespace Osp::Media;

CameraPreview::CameraPreview(void)
{
}

CameraPreview::~CameraPreview(void)
{
	Terminate();
}

result
CameraPreview::StartPreview( CameraStartType startType, const Osp::Graphics::BufferInfo* pBufferInfo )
{
	result r = E_SUCCESS;

	Osp::Media::CameraState state = GetState();
	AppLog("state:%d", state);

	if( state != CAMERA_STATE_INITIALIZED && state != CAMERA_STATE_CAPTURED )
	{
		r = E_INVALID_STATE;
		AppLogException("Invalid State, state:%d", state);
		return r;
	}

	if ( startType == CAMERA_START_NO_PREVIEW_WITHOUT_CALLBACK )
	{
		r = Camera::StartPreview( null, false );
		if(IsFailed(r))
		{
			AppLogException("Camera StartPreview without preview screen and without callback failed.");
			return r;
		}
	}
	else if ( startType == CAMERA_START_NO_PREVIEW_WITH_CALLBACK )
	{
		r = Camera::StartPreview( null, true );
		if(IsFailed(r))
		{
			AppLogException("Camera StartPreview without preview screen and with callback failed.");
			return r;
		}
	}
	else if ( startType == CAMERA_START_PREVIEW_WITHOUT_CALLBACK )
	{
		r = Camera::StartPreview(pBufferInfo, false);
		if(IsFailed(r))
		{
			AppLogException("Camera StartPreview with preview screen and without callback failed.");
			return r;
		}
	}
	else if ( startType == CAMERA_START_PREVIEW_WITH_CALLBACK )
	{
		r = Camera::StartPreview(pBufferInfo, true);
		if(IsFailed(r))
		{
			AppLogException("Camera StartPreview with preview screen and callback failed.");
			return r;
		}
	}
	else
	{
		AppLog("Camera StartPreview has invalid arg.");
		return E_INVALID_ARG;
	}

	r = Osp::System::PowerManager::KeepScreenOnState  ( true, false);
	if(IsFailed(r))
	{
		AppLogException("KeepScreenOnStaet failed.");
		return r;
	}

	return r;
}

result
CameraPreview::StopPreview(void)
{
	result r = E_SUCCESS;

	Osp::Media::CameraState state = GetState();
	AppLog("state:%d", state);

	if ( state == CAMERA_STATE_PREVIEW 	)
	{
		r = Camera::StopPreview();
		if(IsFailed(r))
		{
			AppLogException("Camera StopPreview failed.");
			return r;
		}
	}

	r = Osp::System::PowerManager::KeepScreenOnState  ( false, true);
	AppLog("KeepScreenOnStaet result:%s", GetErrorMessage(r));

	return r;
}

result
CameraPreview::StartCapture(void)
{
	result r = E_SUCCESS;
	bool bFocusSupport = false;	

	Osp::Media::CameraState state = GetState();
	AppLog("state:%d", state);

	if( state != CAMERA_STATE_PREVIEW )
	{
		r = E_INVALID_STATE;
		AppLogException("Invalid state");
		return r;
	}

	r = MediaCapability::GetValue(CAMERA_PRIMARY_SUPPORT_FOCUS, bFocusSupport);
	if ( r == E_SUCCESS && bFocusSupport )
	{
		AppLog("SetAutoFocus is called..");
		r = SetAutoFocus(true);
		if (IsFailed(r))
		{
			AppLogException("SetAutoFocus failed..");
			return r;
		}
	}
	else
	{
		AppLog("Focus is not supported. Capture is called directly..");
		r = Capture();
		if (IsFailed(r))
		{
			AppLogException("Capture failed..");
			return r;
		}
	}

	return r;
}

result
CameraPreview::Initialize(void)
{
	result r = E_SUCCESS;
	AppLog("Enter.");

	if(! IsPoweredOn())
	{
		r = PowerOn();
		if(IsFailed(r))
		{
			AppLogException("Power on of camera failed.");
			return r;
		}
	}

	return r;
}

result
CameraPreview::Terminate(void)
{
	result r = E_SUCCESS;

	Osp::Media::CameraState state = GetState();
	AppLog("state:%d", state);

	if( IsPoweredOn())
	{
		this->StopPreview();

		r = PowerOff();
		if(IsFailed(r))
		{
			AppLogException("Power off of camera failed.");
			return r;
		}
	}

	return r;
}
