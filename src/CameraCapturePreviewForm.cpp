#include "CameraCaptureMainForm.h"
#include "CameraCapturePreview.h"
#include "CameraCapturePreviewForm.h"

using namespace Osp::Base;
using namespace Osp::Base::Collection;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Uix;
using namespace Osp::Graphics;
using namespace Osp::Io;
using namespace Osp::Media;
using namespace Osp::App;

#define ACCERERATION_INTERVAL 100
#define ACCELERATION_THRESHOLD 0.2
#define IS_UPLIGHT(X) ((-ACCELERATION_THRESHOLD<1.0-X)&&(1.0-X<ACCELERATION_THRESHOLD) ? true:false)
#define IS_UPSIDE_DOWN(X) ((-ACCELERATION_THRESHOLD<1.0+X)&&(1.0+X<ACCELERATION_THRESHOLD) ? true:false)

#define DEVICE_PORTRAIT 0
#define DEVICE_PORTRAIT_REVERSE 1
#define DEVICE_LANDSCAPE 2
#define DEVICE_LANDSCAPE_REVERSE 3



CameraForm::CameraForm(void)
{
	__pCameraPreview = null;
	__pOverlayCanvas = null;
	__pOverlayRegion = null;
	__pBackButton = null;
	__pCaptureButton = null;
	__startType = CAMERA_START_NONE;
	__orientation = ORIENTATION_NONE;
	__isStarted = false;
	__pFrame = null;
	__pMainForm = null;
	__previewWidth = 0;
	__previewHeight = 0;
	__previewPixelFormat = PIXEL_FORMAT_YCbCr420_PLANAR;
	__pOutByteBuffer = null;
	__pSensorManager = null;
	__deviceOrientation = DEVICE_LANDSCAPE;
	__skipFrame = 0;
}

CameraForm::~CameraForm(void)
{
	if ( __pCameraPreview )
	{
		delete __pCameraPreview;
		__pCameraPreview = null;
	}

	if ( __pOverlayRegion )
	{
		delete __pOverlayRegion;
		__pOverlayRegion = null;
	}

	if ( __pOverlayCanvas )
	{
	delete __pOverlayCanvas;
	__pOverlayCanvas = null;
	}
}


result
CameraForm::Construct(  Frame *pFrame, Form *pMainForm, Orientation orientation, CameraStartType cameraStartType )
{
	result r = E_SUCCESS;

	r = Form::Construct(FORM_STYLE_NORMAL);
	if(IsFailed(r))
	{
		AppLogException( "Constructing form failed.");
		return r;
	}

	__pFrame = pFrame;
	__pMainForm = pMainForm;
	__startType = cameraStartType;
	__orientation = orientation;

	return E_SUCCESS;
}


result
CameraForm::OnInitializing(void)
{
        result r = E_SUCCESS;

        AppLog("Orientation : %d", __orientation);
        r = __InitCamera();
        if ( r != E_SUCCESS )
        {
                return r;
        }

        SetOrientation(__orientation);
        r = __InitOverlayRegion(__orientation);
        if ( r != E_SUCCESS )
        {
                return r;
        }

        r = __InitSensor();
        if ( r != E_SUCCESS )
        {
                return r;
        }

        r = CameraForm::Start();
        if ( r != E_SUCCESS )
        {
                return r;
        }    

        AddKeyEventListener(*this);

        return E_SUCCESS;  
}

result
CameraForm::OnTerminating(void)
{
	__TerminateCamera();
	__TerminateSensor();

	return E_SUCCESS;
}

void
CameraForm::OnActionPerformed(const Control& source, int actionId)
{
	result r = E_SUCCESS;

	if( actionId == IDC_CAMERA_CAPTURE )
	{
		AppLog("__deviceOrientation:%d", __deviceOrientation);

		switch ( __deviceOrientation)
		{
			case DEVICE_PORTRAIT:
				r=__pCameraPreview->SetExifOrientation(CAMERA_EXIF_ORIENTATION_RIGHT_TOP );
				break;
			case DEVICE_PORTRAIT_REVERSE:
				r=__pCameraPreview->SetExifOrientation(CAMERA_EXIF_ORIENTATION_LEFT_BOTTOM );
				break;
			case DEVICE_LANDSCAPE:
				r=__pCameraPreview->SetExifOrientation(CAMERA_EXIF_ORIENTATION_TOP_LEFT);
				break;
			case DEVICE_LANDSCAPE_REVERSE:
				r=__pCameraPreview->SetExifOrientation(CAMERA_EXIF_ORIENTATION_BOTTOM_RIGHT);
				break;
			default:
				break;
		}

		_SetButtonEnabled(__pBackButton, false);
		_SetButtonEnabled(__pCaptureButton, false);

		r = __pCameraPreview->StartCapture();
		if ( r != E_SUCCESS )
		{
			_SetButtonEnabled(__pBackButton, true);
			_SetButtonEnabled(__pCaptureButton, true);
		}
	}
	else if( actionId == IDC_GOTO_MAIN )
	{
		r = Cancel();
		_GotoMainForm();
	}
}

Orientation
CameraForm::GetOrientation (void)
{
	return __orientation;
}

OverlayRegion*
CameraForm::GetOverlayRegion(void)
{
	return __pOverlayRegion;
}

Canvas*
CameraForm::GetOverlayCanvas(void)
{
	return __pOverlayCanvas;
}

Camera*
CameraForm::GetCamera(void)
{
	return __pCameraPreview;
}

bool
CameraForm::IsStarted(void)
{
	return __isStarted;
}

bool
CameraForm::IsSourceStarted(void)
{
	return IsStarted();
}

result
CameraForm::OnDraw()
{
	result r = E_SUCCESS;

	r = Form::OnDraw();
	if ( IsFailed(r))
	{
		AppLogException("OnDraw failed..");
		return r;
	}

	return r;
}

result
CameraForm::Start( void )
{
	result r = E_SUCCESS;
	Osp::Graphics::BufferInfo bufferInfo;

	r = __pCameraPreview->Initialize();
	if ( IsFailed(r))
	{
		AppLogException("Initializing the camera failed..");
		goto CATCH;
	}

	// Store the preview information
	__previewWidth = __pCameraPreview->GetPreviewResolution().width;
	__previewHeight = __pCameraPreview->GetPreviewResolution().height;
	__previewPixelFormat = __pCameraPreview->GetPreviewFormat();

	switch( __startType )
	{
	case CAMERA_START_NO_PREVIEW_WITHOUT_CALLBACK:
	case CAMERA_START_NO_PREVIEW_WITH_CALLBACK:
		r = __pCameraPreview->StartPreview( __startType, null );
		if ( IsFailed(r))
		{
			AppLogException("StartPreview with callback failed..");
			goto CATCH;
		}
		break;

	case CAMERA_START_PREVIEW_WITH_CALLBACK:
	case CAMERA_START_PREVIEW_WITHOUT_CALLBACK:
		r = __pOverlayRegion->GetBackgroundBufferInfo(bufferInfo);
		if ( IsFailed(r))
		{
			AppLogException("Panel's GetBackgroundBufferInfo failed..");
			goto CATCH;
		}

		r = __pCameraPreview->StartPreview( __startType, &bufferInfo );
		if ( IsFailed(r))
		{
			AppLogException("StartPreview without callback failed..");
			goto CATCH;
		}
		break;

	default:
		r = E_INVALID_ARG;
		goto CATCH;
		break;
	}

    if (__pSensorManager->IsAvailable(SENSOR_TYPE_ACCELERATION))
    {
        r = __pSensorManager->AddSensorListener(*this, SENSOR_TYPE_ACCELERATION, ACCERERATION_INTERVAL, true);
        if (IsFailed(r))
        {
            AppLogException("Add acceleration lister failed.");
			goto CATCH;
        }

    }
    else
    {
        AppLogException("Acceleration sensor is not available.");
        r = E_RESOURCE_UNAVAILABLE;
		goto CATCH;
    }

	__isStarted = true;
    return E_SUCCESS;

CATCH:
	return r;
}

result
CameraForm::Stop()
{
	result r = E_SUCCESS;

	r = __pCameraPreview->StopPreview();
	if ( IsFailed(r))
	{
		AppLogException("Preview stop failed.");
		goto CATCH;
	}

	r = __pSensorManager->RemoveSensorListener(*this, SENSOR_TYPE_ACCELERATION);
	if ( IsFailed(r))
	{
		AppLogException("Remove sensor listener failed.");
		goto CATCH;
	}

	__isStarted = false;

CATCH:
	return r;
}

result
CameraForm::Cancel()
{
	result r = CameraForm::Stop();
	if ( r == E_SUCCESS )
		__pCameraPreview->Terminate();

	_SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pCaptureButton, true);

	return r;
}

bool
CameraForm::CleanUp(void)
{
	this->Cancel();
	return true;
}

bool
CameraForm::HandleLowBatteryCondition(void)
{
	AppLog("CameraForm::HandleLowBatteryCondition\n");
	AppLog("Battery Level is Low: Exit Camera and go to MainForm\n");
	this->_GotoMainForm();
	return true;
}

result
CameraForm::InitButtons( Orientation orientation )
{
    result r = E_SUCCESS;
    Rectangle backButtonRect, captureButtonRect;
    AppLog("orientation : %d", orientation);

	backButtonRect = Rectangle(X_FROM_RIGHT(0,SMALL_BTN_WIDTH,BTN_WIDTH_MARGIN)
								,Y_FROM_TOP(0,SMALL_BTN_HEIGHT,BTN_HEIGHT_MARGIN)
								,SMALL_BTN_WIDTH
								,SMALL_BTN_HEIGHT);

	captureButtonRect = Rectangle(X_FROM_RIGHT(0,SMALL_BTN_WIDTH,BTN_WIDTH_MARGIN)
									,Y_FROM_TOP(1,SMALL_BTN_HEIGHT,BTN_HEIGHT_MARGIN)
									,SMALL_BTN_WIDTH
									,SMALL_BTN_HEIGHT);

    __pBackButton = new Button();
    r = __pBackButton->Construct(backButtonRect,L"Back");
    if(IsFailed(r))
    {
    	AppLogException( "__pBackButton construct failed.");
        goto CATCH;
    }

    __pCaptureButton = new Button();
    r = __pCaptureButton->Construct(captureButtonRect,L"Capture");
    if(IsFailed(r))
    {
    	AppLogException( "__pCaptureButton construct failed.");
        goto CATCH;
    }

    __pBackButton->SetActionId(IDC_GOTO_MAIN);
    __pBackButton->AddActionEventListener(*this);
    r = AddControl(*__pBackButton);
    if(IsFailed(r))
    {
    	AppLogException( "Adding __pBackButton failed.");
        goto CATCH;
    }

    __pCaptureButton->SetActionId(IDC_CAMERA_CAPTURE);
    __pCaptureButton->AddActionEventListener(*this);
    r = AddControl(*__pCaptureButton);
    if(IsFailed(r))
    {
    	AppLogException( "Adding __pCaptureButton failed.");
        goto CATCH;
    }

    __pCaptureButton->Draw();
    __pBackButton->Draw();

    return r;

CATCH:
    RemoveControl(*__pBackButton);
    RemoveControl(*__pCaptureButton);
    return r;

}

void
CameraForm::_GotoMainForm()
{
	result r = E_SUCCESS;

	r = __pCameraPreview->Terminate();
	if( IsFailed(r))
	{
		AppLogException( "Terminate failed..");
//		return;
	}

	//Switch to FormBase
	r = __pFrame->SetCurrentForm( *__pMainForm );
	if( IsFailed(r))
	{
		AppLogException( "SetCurrentForm MainForm failed..");
//		return;
	}

	//Redraw
	__pFrame->Draw();
	__pFrame->Show();
}

int
CameraForm::_SelectivePosition( int wideScreenPosition, int normalScreenPosition )
{
    if ( __orientation == ORIENTATION_PORTRAIT ||
    		__orientation == ORIENTATION_PORTRAIT_REVERSE ||
    				__orientation == ORIENTATION_NONE )
    {
    	return     	normalScreenPosition;
    }
    else
    {
    	return wideScreenPosition;
    }
}

result
CameraForm::_SetButtonEnabled ( Osp::Ui::Controls::Button* pButton, bool enable )
{
	result r = E_SUCCESS;

	if ( pButton )
	{
		r = pButton->SetEnabled(enable);
		if ( r != E_SUCCESS )
			return r;

		r = pButton->Draw();
		if ( r != E_SUCCESS )
			return r;

		r = pButton->Show();
		if ( r != E_SUCCESS )
			return r;
	}
	return r;
}

CameraStartType
CameraForm::_GetCameraStartType (void)
{
	return __startType;
}

result
CameraForm::__ViewImage(Osp::Base::String& imagePath)
{
	result r = E_SUCCESS;

	AppControl* pAc = AppManager::FindAppControlN(APPCONTROL_PROVIDER_IMAGE,APPCONTROL_OPERATION_VIEW);
	if(pAc != null)
	{
		ArrayList* pDataList = null;
		pDataList = new ArrayList();
		pDataList->Construct();
		String* pData = null;

		pData = new String(L"type:image");
		pDataList->Add(*pData);

		pData = new String(L"path:");
		(*pData).Append(imagePath);
		pDataList->Add(*pData);

		r = pAc->Start(pDataList, null);

		delete pAc;
		pAc = null;

		pDataList->RemoveAll(true);
		delete pDataList;

	}
	else
	{
		r  = E_OBJ_NOT_FOUND;
	}

	return r;
}

result
CameraForm::__InitCamera( void )
{
	result r = E_SUCCESS;
	IList* pPreviewResolutionList = null;
	IList* pCaptureResolutionList = null;
	IListT<PixelFormat>* pCaptureFormatList = null;
	IListT<PixelFormat>* pPreviewFormatList = null;

	__pCameraPreview = new CameraPreview();
	if(null == __pCameraPreview)
	{
		AppLogException("Creating CameraPreview failed..");
		return E_OUT_OF_MEMORY;
	}

	r = __pCameraPreview->Construct(*this);
	if (IsFailed(r))
	{
		AppLogException("Construct of camera failed.");
		goto CATCH;
	}

 	r = __pCameraPreview->PowerOn();
 	if (IsFailed(r))
 	{
 		AppLogException("Power on of camera failed. There is no camera on your system. Please check your camera at first..");
		goto CATCH;
 	}

	pPreviewResolutionList = __pCameraPreview->GetSupportedPreviewResolutionListN();
	if ( pPreviewResolutionList != null )
	{
		Dimension* pPreviewResolution = (Dimension*)pPreviewResolutionList->GetAt(pPreviewResolutionList->GetCount ()-1);		// supported max resolution
		r = __pCameraPreview->SetPreviewResolution(*pPreviewResolution);
		if (IsFailed(r))
		{
			AppLogException("Setting preview resolution failed.");
			goto CATCH;
		}
		pPreviewResolutionList->RemoveAll(true);
		delete pPreviewResolutionList;
		pPreviewResolutionList = null;
	}

	pCaptureResolutionList = __pCameraPreview->GetSupportedCaptureResolutionListN();
	if ( pCaptureResolutionList != null )
	{
		Dimension* pCaptureResolution = (Dimension*)pCaptureResolutionList->GetAt(pCaptureResolutionList->GetCount ()-1);		// supported max resolution
		r = __pCameraPreview->SetCaptureResolution(*pCaptureResolution);
		if (IsFailed(r))
		{
			AppLogException("Setting capturing resolution failed.");
			goto CATCH;
		}
		pCaptureResolutionList->RemoveAll(true);
		delete pCaptureResolutionList;
		pCaptureResolutionList = null;
	}

	pCaptureFormatList = __pCameraPreview->GetSupportedCaptureFormatListN();
	if ( pCaptureFormatList != null )
	{
		if ( pCaptureFormatList->Contains(PIXEL_FORMAT_JPEG) )
		{
			r = __pCameraPreview->SetCaptureFormat(PIXEL_FORMAT_JPEG);
			if (IsFailed(r))
			{
				AppLogException("Setting capturing format failed - PIXEL_FORMAT_JPEG .");
				goto CATCH;
			}
		}
		else if ( pCaptureFormatList->Contains(PIXEL_FORMAT_RGB565) )
		{
			r = __pCameraPreview->SetCaptureFormat(PIXEL_FORMAT_RGB565);
			if (IsFailed(r))
			{
				AppLogException("Setting capturing format failed - PIXEL_FORMAT_RGB565.");
				goto CATCH;
			}
		}
		else
		{
			PixelFormat pixelFormat;
			pCaptureFormatList->GetAt(0, pixelFormat);
			r = __pCameraPreview->SetCaptureFormat(pixelFormat);
		}

		pCaptureFormatList->RemoveAll();
		delete pCaptureFormatList;
		pCaptureFormatList = null;
	}

	pPreviewFormatList = __pCameraPreview->GetSupportedPreviewFormatListN();
	if ( pPreviewFormatList != null )
	{
		if ( pPreviewFormatList->Contains(PIXEL_FORMAT_YCbCr420_PLANAR) )
		{
			r = __pCameraPreview->SetPreviewFormat(PIXEL_FORMAT_YCbCr420_PLANAR);
			if (IsFailed(r))
			{
				AppLogException("Setting preview format failed - PIXEL_FORMAT_YCbCr420_PLANAR .");
				goto CATCH;
			}
		}
		else if ( pPreviewFormatList->Contains(PIXEL_FORMAT_RGB565) )
		{
			r = __pCameraPreview->SetPreviewFormat(PIXEL_FORMAT_RGB565);
			if (IsFailed(r))
			{
				AppLogException("Setting preview format failed - PIXEL_FORMAT_RGB565.");
				goto CATCH;
			}
		}
		else
		{
			PixelFormat pixelFormat;
			pPreviewFormatList->GetAt(0, pixelFormat);
			r = __pCameraPreview->SetPreviewFormat( pixelFormat );
			if (IsFailed(r))
			{
				AppLogException("Setting preview format failed.");
				goto CATCH;
			}			
		}

		pPreviewFormatList->RemoveAll();
		delete pPreviewFormatList;
		pPreviewFormatList = null;
	}	

	r = __PreparePixelConverting(PREFERRED_PIXEL_FORMAT);
	if (IsFailed(r))
	{
		AppLogException("Setting preferred pixel setting failed.");
		goto CATCH;
	}

	return E_SUCCESS;

CATCH:
	if ( pPreviewResolutionList )
	{
		pPreviewResolutionList->RemoveAll(true);
		delete pPreviewResolutionList;
	}

	if ( pCaptureResolutionList )
	{
		pCaptureResolutionList->RemoveAll(true);
		delete pCaptureResolutionList;
	}

	if ( pCaptureFormatList )
	{
		pCaptureFormatList->RemoveAll();
		delete pCaptureFormatList;
	}

	if ( pPreviewFormatList )
	{
		pPreviewFormatList->RemoveAll();
		delete pPreviewFormatList;
	}

	if ( __pCameraPreview )
	{
		delete __pCameraPreview;
		__pCameraPreview = null;
	}

	return r;
}

result
CameraForm::__InitOverlayRegion( Orientation orientation )
{
    result r = E_SUCCESS;

    Rectangle __rect(FORM_X, FORM_Y, FORM_WIDTH, FORM_HEIGHT);
	AppLog("Bounds is (%d,%d,%d,%d)",__rect.x, __rect.y, __rect.width, __rect.height);

    bool modified = false;
    bool isValidRect = OverlayRegion::EvaluateBounds(OVERLAY_REGION_EVALUATION_OPTION_LESS_THAN, __rect, modified);
	if (false == isValidRect)
	{
		AppLog("Failed to EvaluateBounds() : [%s] has arisen.", GetErrorMessage(GetLastResult()));
		goto CATCH;
	}
    if (modified)
    {
    	AppLog("Bounds is modified to (%d,%d,%d,%d)", __rect.x, __rect.y, __rect.width, __rect.height);
    }
    __pOverlayRegion= GetOverlayRegionN(__rect, OVERLAY_REGION_TYPE_PRIMARY_CAMERA);

//  Get the OverlayCanvas
    __pOverlayCanvas = GetCanvasN(__rect);
//	For transparency
    __pOverlayCanvas->SetBackgroundColor(Color(0,0,0,0));
    __pOverlayCanvas->SetForegroundColor(Color::COLOR_WHITE);

    r = __pOverlayCanvas->Clear();
    if(IsFailed(r))
    {
    	AppLogException( "Clearing the Canvas failed.");
        goto CATCH;
    }
    return r;

CATCH:
    return r;
}

result
CameraForm::__InitSensor(void)
{
	result r = E_SUCCESS;

	__pSensorManager = new SensorManager();
	r = __pSensorManager->Construct();

	return r;
}

result
CameraForm::__TerminateCamera(void)
{
	if ( IsStarted())
		CameraForm::Stop();

	if ( __pCameraPreview )
		__pCameraPreview->Terminate();

	__UnpreparePixelConverting();

	return E_SUCCESS;
}

void
CameraForm::__TerminateSensor(void)
{
	if ( __pSensorManager )
	{
		delete __pSensorManager;
		__pSensorManager = null;
	}
}

result
CameraForm::__DrawPreview(Osp::Ui::Controls::OverlayRegion& overlayRegion, Osp::Base::ByteBuffer& previewedData)
{
	result r = E_SUCCESS;
	ByteBuffer* pOutPreviewBuffer;
	OverlayRegionBufferPixelFormat bufferPixelFormat = OVERLAY_REGION_BUFFER_PIXEL_FORMAT_YCbCr420_PLANAR;
	Osp::Graphics::PixelFormat outPixelFormat = PIXEL_FORMAT_YCbCr420_PLANAR;

	AppLog("Preview format : %d, Converting pixel format : %d", __previewPixelFormat, PREFERRED_PIXEL_FORMAT);

	if ( __previewPixelFormat != PREFERRED_PIXEL_FORMAT)
	{
		if ( __previewPixelFormat == PIXEL_FORMAT_RGB565
				&& PREFERRED_PIXEL_FORMAT == PIXEL_FORMAT_YCbCr420_PLANAR)
		{
			pOutPreviewBuffer = __ConvertRGB565_2YCbCr420p(&previewedData);
			outPixelFormat = PIXEL_FORMAT_YCbCr420_PLANAR;
		}
		else if ( __previewPixelFormat == PIXEL_FORMAT_YCbCr420_PLANAR
				&& PREFERRED_PIXEL_FORMAT == PIXEL_FORMAT_RGB565)
		{
			pOutPreviewBuffer = __ConvertYCbCr420p2RGB565(&previewedData);
			outPixelFormat = PIXEL_FORMAT_RGB565;
		}
		else if ( __previewPixelFormat == PIXEL_FORMAT_YCbCr420_PLANAR
				&& PREFERRED_PIXEL_FORMAT == PIXEL_FORMAT_ARGB8888)
		{
			pOutPreviewBuffer = __ConvertYCbCr420p2ARGB8888(&previewedData);
			outPixelFormat = PIXEL_FORMAT_ARGB8888;
		}
		else
		{
			pOutPreviewBuffer = &previewedData;
			outPixelFormat = __previewPixelFormat;
		}

		if ( !pOutPreviewBuffer )
			return E_SYSTEM;
	}
	else
	{
		pOutPreviewBuffer = &previewedData;
		outPixelFormat = __previewPixelFormat;
	}

	switch (outPixelFormat)
	{
/**
* OverlayRegion's SetInputBuffer API doesn't support the OVERLAY_REGION_BUFFER_PIXEL_FORMAT_ARGB8888 and OVERLAY_REGION_BUFFER_PIXEL_FORMAT_RGB565 format since API version 1.2.
* Camera's RGB565 preview data can be converted to YCbCr420Planar format by using __ConvertRGB565_2YCbCr420p method in this sample.
*
*	case PIXEL_FORMAT_RGB565:
*		bufferPixelFormat = OVERLAY_REGION_BUFFER_PIXEL_FORMAT_RGB565;
*		break;
*	case PIXEL_FORMAT_ARGB8888:
*		bufferPixelFormat = OVERLAY_REGION_BUFFER_PIXEL_FORMAT_ARGB8888;
*		break;
*/
	case PIXEL_FORMAT_YCbCr420_PLANAR:
		bufferPixelFormat = OVERLAY_REGION_BUFFER_PIXEL_FORMAT_YCbCr420_PLANAR;
		break;
	default:
		AppLog( "Pixel format not supported.");
		return E_UNSUPPORTED_FORMAT;
	}
	r = overlayRegion.SetInputBuffer(*pOutPreviewBuffer, Dimension(__previewWidth,__previewHeight), bufferPixelFormat);

	return r;
}


result
CameraForm::__DrawPrimitive(Osp::Graphics::Canvas& overlayCanvas )
{
	result r = E_SUCCESS;

	// Draw a line and rectangle overlapping to previewed image.
	// The native draw APIs should use container's canvas.

	r = overlayCanvas.DrawLine(Point(0,0), Point(200,200));											// Draw line overlapping to preview image
	if (IsFailed(r))
	{
		AppLogException( " DrawLine was failed.");
		return r;
	}

	r = overlayCanvas.FillRectangle(Color(0xff0000, false), Rectangle(130,130,50,50));         // Fill rectangle overlapping to preview image
	if (IsFailed(r))
	{
		AppLogException( " FillRectangle was failed.");
		return r;
	}

	return r;
}

Osp::Base::ByteBuffer*
CameraForm::__ConvertYCbCr420p2RGB565(Osp::Base::ByteBuffer * pB)
{
	int w = __previewWidth;
	int h = __previewHeight;
	int hw = w/2;
	int hh = h/2;

	unsigned char* pOriY= (unsigned char*)(const_cast<byte*>(pB->GetPointer()));
	unsigned char* pOriCb= pOriY + w*h;
	unsigned char* pOriCr = pOriCb + hw*hh;

	unsigned char* pOut1st = (unsigned char*)(const_cast<byte*>(__pOutByteBuffer->GetPointer()));

	for(int j=0;j<h;j+=2)
	{
		unsigned char* pY0,*pY1, *pCb, *pCr;
		pY0 = pOriY + j*w;
		pY1 = pOriY + (j+1)*w;
		pCb = pOriCb + hw*(j/2);
		pCr = pOriCr + hw*(j/2);

		unsigned char* buf = pOut1st + (w*2)*j;
		unsigned short* pB0= (unsigned short*)buf;

		buf = pOut1st + (w*2)*(j+1);
		unsigned short* pB1= (unsigned short*)buf;
		for( int i=0;i <w;i+=2)
		{
			int Y[4],Cb,Cr;
			Cb = pCb[i/2] - 128;
			Cr = pCr[i/2] - 128;

			Y[0]=pY0[i];
			Y[1]=pY0[i+1];
			Y[2]=pY1[i];
			Y[3]=pY1[i+1];

			int A,B,C;
			A= Cr + (Cr>>2) + (Cr>>3) + (Cr>>5);
			B= (Cb>>2) + (Cb>>4) + (Cb>>5) + (Cr>>1) + (Cr>>3)
				+ (Cr>>4) + (Cr>>5);
			C= Cb + (Cb>>1) + (Cb>>2) + (Cb>>6);

#define CL_R(A)		(((A)>>3)&0x1F)
#define CL_G(A)		(((A)>>2)&0x3F)
#define CL_B(A)		(((A)>>3)&0x1F)
#define RGB565(R, G, B) ( (R<<11) | (G<<5) | (B) )	//FOR BITMAP RGB565
			pB0[i]   = RGB565(CL_R(Y[0]+A), CL_G(Y[0]-B), CL_B(Y[0]+C));
			pB0[i+1] = RGB565(CL_R(Y[1]+A), CL_G(Y[1]-B), CL_B(Y[1]+C));
			pB1[i]   = RGB565(CL_R(Y[2]+A), CL_G(Y[2]-B), CL_B(Y[2]+C));
			pB1[i+1] = RGB565(CL_R(Y[3]+A), CL_G(Y[3]-B), CL_B(Y[3]+C));
		}
	}

	return __pOutByteBuffer;
}

Osp::Base::ByteBuffer*
CameraForm::__ConvertYCbCr420p2ARGB8888(Osp::Base::ByteBuffer * pB)
{
	int w = __previewWidth;
	int h = __previewHeight;
	int hw = w/2;
	int hh = h/2;

	unsigned char* pOriY= (unsigned char*)const_cast<byte *>(pB->GetPointer());
	unsigned char* pOriCb= pOriY + w*h;
	unsigned char* pOriCr = pOriCb + hw*hh;

	unsigned char* pOut1st = const_cast<byte*>(__pOutByteBuffer->GetPointer());

	for(int j=0;j<h;j+=2)
	{
		unsigned char* pY0,*pY1, *pCb, *pCr;
		pY0 = pOriY + j*w;
		pY1 = pOriY + (j+1)*w;
		pCb = pOriCb + hw*(j/2);
		pCr = pOriCr + hw*(j/2);

		unsigned char* buf = pOut1st + (w*4)*j;
		unsigned int* pB0= (unsigned int*)buf;

		buf = pOut1st + (w*4)*(j+1);
		unsigned int* pB1= (unsigned int*)buf;
		for( int i=0;i <w;i+=2)
		{
			int Y[4],Cb,Cr;
			Cb = pCb[i/2] - 128;
			Cr = pCr[i/2] - 128;

			Y[0]=pY0[i];
			Y[1]=pY0[i+1];
			Y[2]=pY1[i];
			Y[3]=pY1[i+1];

			int A,B,C;
			A= Cr + (Cr>>2) + (Cr>>3) + (Cr>>5);
			B= (Cb>>2) + (Cb>>4) + (Cb>>5) + (Cr>>1) + (Cr>>3)
				+ (Cr>>4) + (Cr>>5);
			C= Cb + (Cb>>1) + (Cb>>2) + (Cb>>6);

#define CL(A) 	((A>255)? 255:( (A<0)? 0:A ))
//#define RGBA(R, G, B, A) ( (A <<24) | (B<<16) | (G<<8) | (R) )//FOR OPENGLES
#define RGBA(R, G, B, A) ( (A <<24) | (R<<16) | (G<<8) | (B) )//FOR BITMAP ARGB8888

			pB0[i]   = RGBA(CL(Y[0]+A), CL(Y[0]-B), CL(Y[0]+C), 0XFF);
			pB0[i+1] = RGBA(CL(Y[1]+A), CL(Y[1]-B), CL(Y[1]+C), 0XFF);
			pB1[i]   = RGBA(CL(Y[2]+A), CL(Y[2]-B), CL(Y[2]+C), 0XFF);
			pB1[i+1] = RGBA(CL(Y[3]+A), CL(Y[3]-B), CL(Y[3]+C), 0XFF);
		}
	}

	return __pOutByteBuffer;
}

Osp::Base::ByteBuffer*
CameraForm::__ConvertRGB565_2YCbCr420p(Osp::Base::ByteBuffer * pB)
{
	int w = __previewWidth;
	int h = __previewHeight;
	int hw = w/2;
	int hh = h/2;

	unsigned char* pOriB = (unsigned char*)(const_cast<byte*>(pB->GetPointer()));
	unsigned char* pOutY= (unsigned char*)(const_cast<byte *>(__pOutByteBuffer->GetPointer()));

	if ( !pOutY || !pOriB )
		return null;

	unsigned char* pOutCb= pOutY + w*h;
	unsigned char* pOutCr = pOutCb + hw*hh;

	for (int j=0;j<h;j+=2)
	{
		unsigned short* pB0;
		unsigned short* pB1;

		unsigned char *pY0,*pY1, *pCb, *pCr;

		pY0 = pOutY + j*w;
		pY1 = pOutY + (j+1)*w;

		pCb = pOutCb + hw*(j/2);
		pCr = pOutCr + hw*(j/2);

		pB0 = (unsigned short*)(pOriB + (w*2)*j);
		pB1 = (unsigned short*)(pOriB + (w*2)*(j+1));

		for (int i=0; i<w; i+=2)
		{
			int Y[2][2],Cb,Cr;

			unsigned char R, G, B;
			unsigned short* pRgb[2][2];

			pRgb[0][0] = pB0 + i;
			pRgb[0][1] = pB0 + i + 1;
			pRgb[1][0] = pB1 + i;
			pRgb[1][1] = pB1 + i + 1;

#define CL_R_(A)		(((A>>11) &0b11111 ) << 3 )
#define CL_G_(A)		(((A>>5) &0b00000111111) << 2 )
#define CL_B_(A)		((A&0b0000000000011111) << 3 )

			for ( int row=0; row<2; row++ )
			{
				for ( int col=0; col<2; col++ )
				{
					R = (unsigned char)CL_R_(*(pRgb[row][col]));
					G = (unsigned char)CL_G_(*(pRgb[row][col]));
					B = (unsigned char)CL_B_(*(pRgb[row][col]));

					Y[row][col] = (( 66*(int)R + 129*(int)G + 25*(int)B + 128 ) >> 8) + 16;
				}
			}

			Cb = ((-38*(int)R -74*(int)G + 112*(int)B + 128 ) >> 8 ) + 128;
			Cr = ((112*(int)R -94*(int)G -18*(int)B + 128 ) >> 8 ) + 128;

			// Boundary check and assign
			pY0[i] = Y[0][0] < 235 ? Y[0][0] : 235;
			pY0[i+1] = Y[0][1] < 235 ? Y[0][1] : 235;
			pY1[i] = Y[1][0] < 235 ? Y[1][0] : 235;
			pY1[i+1] = Y[1][1] < 235 ? Y[1][1] : 235;
			*pCb++ = Cb < 240 ? Cb : 240;
			*pCr++ = Cr < 240 ? Cr : 240;
		}
	}

	return __pOutByteBuffer;
}

result
CameraForm::__PreparePixelConverting( Osp::Graphics::PixelFormat outFormat )
{
	result r = E_SUCCESS;
	Dimension previewDim;
	// Prepare the resources for pixel converting.

	if( !__pCameraPreview)
	{
		AppLog( ">>>>>> Camera object not found.");
		r = E_OBJ_NOT_FOUND;
		goto CATCH;
	}
	previewDim = __pCameraPreview->GetPreviewResolution();

	__UnpreparePixelConverting();

	switch ( outFormat )
	{
		case PIXEL_FORMAT_RGB565:
			__pOutByteBuffer = new ByteBuffer();
			r = __pOutByteBuffer->Construct(previewDim.width* previewDim.height * 2 );
			break;

		case PIXEL_FORMAT_ARGB8888:
			__pOutByteBuffer = new ByteBuffer();
			r = __pOutByteBuffer->Construct(previewDim.width* previewDim.height * 4 );
			break;

		case PIXEL_FORMAT_YCbCr420_PLANAR:
			__pOutByteBuffer = new ByteBuffer();
			r = __pOutByteBuffer->Construct(previewDim.width* previewDim.height * 6/4);
			break;

		default:
			break;
	}

	if(IsFailed(r))
	{
		AppLogException( "ByteBuffer Construct failed.");
		goto CATCH;
	}

	return r;

CATCH:
	__UnpreparePixelConverting();
	return r;
}


void
CameraForm::__UnpreparePixelConverting(void)
{
	if ( __pOutByteBuffer )
	{
		delete 	__pOutByteBuffer;
		__pOutByteBuffer = null;
	}
}

void
CameraForm::OnCameraCaptured ( Osp::Base::ByteBuffer& capturedData, result r )
{
	result ir = E_SUCCESS;

	String imagePath = L"/Home/Sample";
	MessageBox msgBoxError;
	int msgBoxErrorResult = 0;
	int width = 0;
	int height = 0;
	int bitsPerPixel = 0;
	int bytesPerPixel = 0;

	if ( r == E_SUCCESS )
	{
		if ( __pCameraPreview->GetCaptureFormat() == PIXEL_FORMAT_JPEG )
		{
			File file;

			imagePath.Append(L".jpg");

			if ( File::IsFileExist(imagePath))
			{
				r = File::Remove(imagePath);
				if (IsFailed(ir))
				{
					AppLogException( "File removing failed .");
					goto CATCH_FILE_ERROR;
				}
			}

			ir = file.Construct(imagePath, L"w", true);
			if (IsFailed(ir))
			{
				AppLogException( "File construct failed .");
				goto CATCH_FILE_ERROR;
			}

			ir = file.Write(capturedData);
			if (IsFailed(ir))
			{
				AppLogException( "File writing failed .");
				goto CATCH_FILE_ERROR;
			}
		}
		else if ( __pCameraPreview->GetCaptureFormat() == PIXEL_FORMAT_RGB565 )
		{

			BufferInfo info;

			Bitmap* pBitmap = new Bitmap();
			ir = pBitmap->Construct(capturedData,Dimension(__pCameraPreview->GetCaptureResolution().width, __pCameraPreview->GetCaptureResolution().height), BITMAP_PIXEL_FORMAT_RGB565);

			pBitmap->Lock(info);

			width = info.width;
			height = info.height;
			bitsPerPixel = info.bitsPerPixel;

			if(bitsPerPixel == 32)
			bytesPerPixel = 4;
			else if(bitsPerPixel == 24)
			bytesPerPixel =3;

			for(int x=0;x<width;x++) {
				for (int y=0;y<height;y++) {
					int* color = ((int *)((byte *) info.pPixels + y * info.pitch + x *bytesPerPixel));
					byte* blue = (byte*) color;
					byte* green = blue+ 1;
					byte* red = green+ 1;
					byte gray = ((*blue) * 0.11) + ((*green) * 0.59) + ((*red) *0.3);
					*red = gray;
					*green = gray;
					*blue =gray;
			   }
			}

			pBitmap->Unlock();


			if (IsFailed(ir))
			{
				AppLogException( "Bitmap construction failed .");
				delete pBitmap;
				goto CATCH_ENCODING_ERROR;
			}

			imagePath.Append(L".bmp");
			Image* pImage = new Image();
			ir = pImage->Construct();

			if (IsFailed(ir))
			{
				AppLogException( "Image construction failed .");
				delete pBitmap;
				delete pImage;
				goto CATCH_ENCODING_ERROR;
			}

			AppLog( "Image construct succeeded.");
			ir = pImage->EncodeToFile(*pBitmap, IMG_FORMAT_JPG, imagePath, true);
			if ( IsFailed(ir) )
			{
				AppLogException( "Image Encode to file failed.");
				delete pBitmap;
				delete pImage;
				goto CATCH_FILE_ERROR;
			}

			delete pImage;
			delete pBitmap;
		}
		else
		{
			//Add the codes to save the other format's capture data.
		}

		ir = __ViewImage(imagePath);
		if ( IsFailed(ir) )
		{
			AppLogException("ImageViewer AppContorl not found. .");
			goto CATCH_IMAGE_VIEWER_ERROR;
		}
	}

	return;

CATCH_ENCODING_ERROR:
	// Error
	msgBoxError.Construct(L"WARNING",L"Encoding to File error",MSGBOX_STYLE_OK,0);
	msgBoxError.ShowAndWait(msgBoxErrorResult);

	ir = CameraForm::Start();
	if ( ir != E_SUCCESS )
	{
		ir = __pCameraPreview->Terminate();
	}

	_SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pCaptureButton, true);

	return;

CATCH_FILE_ERROR	:
	// File write error
	msgBoxError.Construct(L"WARNING",L"File writing error or storage is full",MSGBOX_STYLE_OK,0);
	msgBoxError.ShowAndWait(msgBoxErrorResult);

	ir = CameraForm::Start();
	if ( ir != E_SUCCESS )
	{
		ir = __pCameraPreview->Terminate();
	}

	_SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pCaptureButton, true);

	return;

CATCH_IMAGE_VIEWER_ERROR	:
	// Image Viewer not found
	msgBoxError.Construct(L"WARNING",L"Image Viewer not found",MSGBOX_STYLE_OK,0);
	msgBoxError.ShowAndWait(msgBoxErrorResult);

	ir = CameraForm::Start();
	if ( ir != E_SUCCESS )
	{
		ir = __pCameraPreview->Terminate();
	}

	_SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pCaptureButton, true);
	return;
}

void
CameraForm::OnCameraAutoFocused(bool completeCondition )
{
	result r = E_SUCCESS;

	r = __pCameraPreview->Capture();
	if (IsFailed(r))
	{
		AppLogException("Camera Capture failed..");
	}
}

void
CameraForm::OnCameraPreviewed ( Osp::Base::ByteBuffer& previewedData , result r)
{
	result ir = E_SUCCESS;
	AppLog( "Enter. __startType:%d, r=%s", __startType, GetErrorMessage(r));

	if(__skipFrame < 2)
	{
		__skipFrame++;
		return;
	}
	if ( r == E_SUCCESS )
	{
		if ( __startType == CAMERA_START_NO_PREVIEW_WITH_CALLBACK )
		{
			ir = __DrawPreview(*__pOverlayRegion, previewedData);
			if (IsFailed(ir))
			{
				AppLogException( "Draw previewing  failed.");
				return;
			}
		}

Image* pImage = new Image();
		pImage->Construct();
	    Rectangle rect(FORM_X, FORM_Y, FORM_WIDTH, FORM_HEIGHT);

		Bitmap* pBitmap = pImage->DecodeN(L"/Res/bada.jpg", BITMAP_PIXEL_FORMAT_RGB565, 50, 50 );
		r = __pOverlayCanvas -> DrawBitmap(rect, *pBitmap);
		if (IsFailed(r)){
			AppLog("nao carregou imagem");
		}


/*
		if ( __startType != CAMERA_START_NO_PREVIEW_WITHOUT_CALLBACK )
		{
			r = __DrawPrimitive(*__pOverlayCanvas);
			if (IsFailed(ir))
			{
				AppLogException( "Draw primitive  failed.");
				return;
			}

			r = __pOverlayCanvas->Show();
			if (IsFailed(ir))
			{
				AppLogException( "The Canvas show failed.");
				return;
			}
		}
*/
		if ( __startType == CAMERA_START_NO_PREVIEW_WITH_CALLBACK )
		{
			ir = __pOverlayRegion->Show();
			if (IsFailed(ir))
			{
				AppLogException( "OverlayRegion's show failed.");
				return;
			}
		}
	}
}

void
CameraForm::OnCameraErrorOccurred( CameraErrorReason r )
{
	MessageBox msgBoxError;
	int msgBoxErrorResult = 0;

	AppLog( "OnCameraErrorOccurred has been called.");

	switch(r)
	{
	case CAMERA_ERROR_OUT_OF_MEMORY:
		msgBoxError.Construct(L"WARNING",L"Memory full",MSGBOX_STYLE_OK,0);
		break;
	case CAMERA_ERROR_DEVICE_FAILED:
		msgBoxError.Construct(L"WARNING",L"Camera Device Failed",MSGBOX_STYLE_OK,0);
		break;
	case CAMERA_ERROR_DEVICE_INTERRUPTED:
		msgBoxError.Construct(L"WARNING",L"Camera Device Interrupted",MSGBOX_STYLE_OK,0);
		break;
	default:
		msgBoxError.Construct(L"WARNING",L"An Error Occurred",MSGBOX_STYLE_OK,0);
		break;				
	}
	// An error occurred

	msgBoxError.ShowAndWait(msgBoxErrorResult);

	CameraForm::Cancel();
    CameraForm::Start();

    _SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pCaptureButton, true);
}

void
CameraForm::OnDataReceived( SensorType sensorType, SensorData &sensorData, result r)
{
	if ( r == E_SUCCESS && sensorType == SENSOR_TYPE_ACCELERATION )
	{
		int tempDeviceOrientation = __deviceOrientation;
		float valueX, valueY, valueZ;
		r = sensorData.GetValue((SensorDataKey)ACCELERATION_DATA_KEY_X, valueX );
		r = sensorData.GetValue((SensorDataKey)ACCELERATION_DATA_KEY_Y, valueY );
		r = sensorData.GetValue((SensorDataKey)ACCELERATION_DATA_KEY_Z, valueZ );

		if ( IS_UPLIGHT(valueY))
			tempDeviceOrientation = DEVICE_PORTRAIT;
		else if ( IS_UPSIDE_DOWN(valueY))
			tempDeviceOrientation = DEVICE_PORTRAIT_REVERSE;
		else if ( IS_UPLIGHT(valueX))
			tempDeviceOrientation = DEVICE_LANDSCAPE;
		else if ( IS_UPSIDE_DOWN(valueX))
			tempDeviceOrientation = DEVICE_LANDSCAPE_REVERSE;

		if ( tempDeviceOrientation != __deviceOrientation )
		{
			AppLog("Orientation changed. ACCELERATION(%f, %f, %f), Device-orientation:%d", valueX,valueY,valueZ,tempDeviceOrientation);
			__deviceOrientation = tempDeviceOrientation;
		}
	}
}

void
CameraForm::OnKeyReleased (const Control &source, Osp::Ui::KeyCode  keyCode)
{

}

void
CameraForm::OnKeyLongPressed (const Control &source, Osp::Ui::KeyCode  keyCode)
{

}

void
CameraForm::OnKeyPressed (const Control &source, Osp::Ui::KeyCode  keyCode)
{
	result r = E_SUCCESS;

	if( __pCameraPreview )
	{
		int zoomLevel = __pCameraPreview->GetZoomLevel();
		int maxZoomLevel = __pCameraPreview->GetMaxZoomLevel();

		if(keyCode == Osp::Ui::KEY_SIDE_UP)
		{
			if ( zoomLevel < maxZoomLevel)
			{
				r = __pCameraPreview->ZoomIn();
				AppLog("ZoomIn:%s", GetErrorMessage(r));
			}
		}
		else if(keyCode == Osp::Ui::KEY_SIDE_DOWN)
		{
			if ( zoomLevel > 0)
			{
				r = __pCameraPreview->ZoomOut();
				AppLog("ZoomOut:%s", GetErrorMessage(r));
			}
		}
	}

	Osp::Ui::Control::ConsumeInputEvent();
	if(IsFailed(r))
	{
		AppLog("Consuming input event failed:%s", GetErrorMessage(r));
	}
}

result
CameraForm::ShowButtons()
{
	_SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pCaptureButton, true);

	return E_SUCCESS;
}

