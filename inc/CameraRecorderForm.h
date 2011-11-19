
#ifndef CAMERARECORDERFORM_H_
#define CAMERARECORDERFORM_H_

#include "CameraCapturePreviewForm.h"
#include "CameraRecorder.h"

class CameraRecorderForm:
	public CameraForm,
	public Osp::Media::IVideoRecorderEventListener,
	public Osp::Base::Runtime::ITimerEventListener
{
public:
	CameraRecorderForm(void);
	virtual ~CameraRecorderForm(void);
	result Construct( Osp::Ui::Controls::Frame *pFrame, Osp::Ui::Controls::Form *pMainForm, Osp::Ui::Orientation orientation, CameraStartType cameraStartType );
	result  OnInitializing (void);
	result  OnTerminating (void);

	result InitButtons( Osp::Ui::Orientation orientation );
	result StartRecord(void);
	result Stop(void);
	result Cancel(void);
	bool IsStarted(void);
	bool IsSourceStarted(void);
	bool CleanUp(void);
	bool HandleLowBatteryCondition(void);

	void OnActionPerformed(const Osp::Ui::Control& source, int actionId);

	// Inherited purely virtual functions from IVideoRecorderEventListener
	void OnVideoRecorderStopped(result r);
	void OnVideoRecorderCanceled(result r);
	void OnVideoRecorderPaused(result r);
	void OnVideoRecorderStarted(result r);
	void OnVideoRecorderEndReached( Osp::Media::RecordingEndCondition endCondition);
	void OnVideoRecorderClosed(result r);
	void OnVideoRecorderErrorOccurred( Osp::Media::RecorderErrorReason r);

	void  OnTimerExpired (Osp::Base::Runtime::Timer &timer);
	result ShowButtons();
private:
	result __InitCameraRecorder( void );
	result __updateTimeSize(bool reset);
	result __ViewVideo(Osp::Base::String& videoPath);

	Osp::Media::Camera* 	__pCamera;
	CameraRecorder* __pCameraRecorder;
	bool 		__isStarted;
	bool 		__isCanceled;
	bool 		__isFromError;
	Osp::Ui::Controls::Button *__pBackButton, *__pRecordButton, *__pStopButton;
	Osp::Base::String		__recordingFile;
	Osp::Base::Runtime::Timer __timer;

	static const int __inforTimerPeriod = 1000;		// 1sec
};

#endif // CAMERARECORDERFORM_H_ 
