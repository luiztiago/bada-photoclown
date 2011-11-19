
#ifndef CAMERACAPTUREIFORM_H_
#define CAMERACAPTUREIFORM_H_

#include <FBase.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FApp.h>
#include <FMedia.h>

typedef enum
{
	CAMERA_START_NONE,
	CAMERA_START_NO_PREVIEW_WITHOUT_CALLBACK,
	CAMERA_START_NO_PREVIEW_WITH_CALLBACK,
	CAMERA_START_PREVIEW_WITHOUT_CALLBACK,
	CAMERA_START_PREVIEW_WITH_CALLBACK,
}CameraStartType;

typedef enum
{
	CAMERA_FORM,
	CAMERA_RECORDER_FORM,
}StartFormType;

class CameraCaptureIForm	:
	public Osp::Ui::Controls::Form
{
public:
	virtual result Construct( Osp::Ui::Controls::Frame *pFrame, Osp::Ui::Controls::Form *pMainForm, Osp::Ui::Orientation orientation, CameraStartType cameraStartType ) = 0;
	virtual result InitButtons( Osp::Ui::Orientation orientation ) = 0;
	virtual result ShowButtons() = 0;

	virtual result Start( void ) = 0;
	virtual result Stop(void) = 0;
	virtual result Cancel(void) = 0;
	virtual bool IsStarted(void) = 0;
	virtual bool IsSourceStarted(void) = 0;
	virtual bool CleanUp(void) = 0;
	virtual bool HandleLowBatteryCondition(void) = 0;
};

#endif //CAMERACAPTUREIFORM_H_ 
