
#ifndef CAMERACAPTUREPREVIEWFORM_H_
#define CAMERACAPTUREPREVIEWFORM_H_

#include <FBase.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FApp.h>
#include <FMedia.h>
#include <FIo.h>
#include <FSystem.h>
#include <FUix.h>

#include "CameraCaptureResource.h"
#include "CameraCapturePreview.h"
#include "CameraCaptureIForm.h"

class CameraForm	:
	public CameraCaptureIForm,
	public Osp::Ui::IActionEventListener,
	public Osp::Media::ICameraEventListener,
	public Osp::Uix::ISensorEventListener,
	public Osp::Ui::IKeyEventListener
{
public:
	CameraForm(void);
	virtual ~CameraForm(void);
	result Construct( Osp::Ui::Controls::Frame *pFrame, Osp::Ui::Controls::Form *pMainForm, Osp::Ui::Orientation orientation, CameraStartType cameraStartType );
	result  OnInitializing (void);
	result  OnTerminating (void);

	result InitButtons( Osp::Ui::Orientation orientation );
	result Start( void );
	result Stop(void);
	result Cancel(void);
	bool IsStarted(void);
	bool IsSourceStarted(void);
	bool CleanUp(void);
	bool HandleLowBatteryCondition(void);

	result OnDraw(void);
	void OnActionPerformed(const Osp::Ui::Control& source, int actionId);

	// Inherited purely virtual functions from ICameraEventListener
	void OnCameraAutoFocused(bool completeCondition );
	void OnCameraPreviewed ( Osp::Base::ByteBuffer& previewedData , result r );
	void OnCameraCaptured ( Osp::Base::ByteBuffer& capturedData , result r );
	void OnCameraErrorOccurred(Osp::Media::CameraErrorReason r);
	// Inherited purely virtual functions from ISensorEventListener
	void OnDataReceived( Osp::Uix::SensorType sonsorType, Osp::Uix::SensorData &sensorData, result r);
	// Inherited purely virtual functions from IKeyEventListener
	void OnKeyPressed (const Control &source, Osp::Ui::KeyCode  keyCode);
	void OnKeyReleased (const Control &source, Osp::Ui::KeyCode  keyCode);
	void OnKeyLongPressed (const Control &source, Osp::Ui::KeyCode  keyCode);

	Osp::Ui::Orientation  GetOrientation (void);
	Osp::Ui::Controls::OverlayRegion* GetOverlayRegion(void);
	Osp::Graphics::Canvas* GetOverlayCanvas(void);
	Osp::Media::Camera*	GetCamera(void);
	result ShowButtons();
	
protected:
	void _GotoMainForm(void);
	int _SelectivePosition( int wideScreenPosition, int normalScreenPosition );
	result _SetButtonEnabled ( Osp::Ui::Controls::Button* pButton, bool enable );
	CameraStartType _GetCameraStartType (void);

private:
	result __InitCamera( void );
	result __InitOverlayRegion( Osp::Ui::Orientation orientation );
	result __InitSensor(void);
	result __TerminateCamera(void);
	void __TerminateSensor(void);

	result __DrawPreview( Osp::Ui::Controls::OverlayRegion& overlayRegion, Osp::Base::ByteBuffer& previewedData);
	result __DrawPrimitive(Osp::Graphics::Canvas& overlayCanvas );
	result __ViewImage(Osp::Base::String& imagePath);
	Osp::Base::ByteBuffer* __ConvertYCbCr420p2RGB565(Osp::Base::ByteBuffer * pB);
	Osp::Base::ByteBuffer* __ConvertYCbCr420p2ARGB8888(Osp::Base::ByteBuffer * pB);
	Osp::Base::ByteBuffer* __ConvertRGB565_2YCbCr420p(Osp::Base::ByteBuffer * pB);
	result __PreparePixelConverting(Osp::Graphics::PixelFormat outFormat );
	void __UnpreparePixelConverting(void);
	
	CameraPreview* __pCameraPreview;
	Osp::Ui::Controls::OverlayRegion*	__pOverlayRegion;
	Osp::Graphics::Canvas*		__pOverlayCanvas;
	Osp::Ui::Controls::Button *__pBackButton, *__pCaptureButton;
	int __previewWidth, __previewHeight;
	Osp::Graphics::PixelFormat __previewPixelFormat;
	bool __isStarted;

	CameraStartType __startType;
	Osp::Ui::Orientation __orientation;
	Osp::Ui::Controls::Frame* 	__pFrame;
	Osp::Ui::Controls::Form*	__pMainForm;
	Osp::Base::ByteBuffer* __pOutByteBuffer;
	Osp::Graphics::BufferInfo __bufferInfo;

	Osp::Uix::SensorManager* __pSensorManager;
	int __deviceOrientation;
	int __skipFrame ;

	static const int __deviceInitTime = 200;		// 200 msec
	static const Osp::Graphics::PixelFormat PREFERRED_PIXEL_FORMAT = Osp::Graphics::PIXEL_FORMAT_YCbCr420_PLANAR;
};

#endif // CAMERACAPTUREPREVIEWFORM_H_ 
