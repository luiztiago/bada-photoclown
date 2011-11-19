#include "CameraCaptureMainForm.h"
#include "CameraCapturePreviewForm.h"
#include "CameraRecorderForm.h"
#include <FSysBattery.h>

#define ID_BUTTON_CAMERA                      0x1001

#define ID_BUTTON_CALLBACK_CHECKED		0x1010
#define ID_BUTTON_CALLBACK_UNCHECKED		0x1011
#define ID_BUTTON_CALLBACK_SELECTED                      0x1012

#define ID_BUTTON_PREVIEW_CHECKED		0x1020
#define ID_BUTTON_PREVIEW_UNCHECKED		0x1021
#define ID_BUTTON_PREVIEW_SELECTED                      0x1022

using namespace Osp::Base;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Graphics;
using namespace Osp::System;

MainForm::MainForm(void)
{
        __pStartForm = null;
        __pBtnCameraStart = null;
        __pCheckBtnCallback = null;
        __pCheckBtnPreview = null;
        __isCallbackEnabled = false;
        __isPreviewScreenEnabled = false;
        __targetOrientation = ORIENTATION_LANDSCAPE;
}

MainForm::~MainForm(void)
{
}

result
MainForm::Construct()
{
        result r = E_SUCCESS;

        r = Form::Construct(FORM_STYLE_NORMAL|FORM_STYLE_INDICATOR|FORM_STYLE_HEADER|FORM_STYLE_FOOTER);
        if(IsFailed(r))
        {
        	AppLogException("Form construction has failed.");
        	return r;
        }
        return E_SUCCESS;
}

result
MainForm::OnInitializing(void)
{
        if( __InitializePreviewCheckButton()!= E_SUCCESS ) return E_OUT_OF_MEMORY;
        if( __InitializeCallbackCheckButton()!= E_SUCCESS ) return E_OUT_OF_MEMORY;
        if( __InitializeHeader()!= E_SUCCESS ) return E_OUT_OF_MEMORY;
        if( __InitializeFooter()!= E_SUCCESS ) return E_OUT_OF_MEMORY;

        return E_SUCCESS;
}

result
MainForm::OnTerminating(void)
{
        return E_SUCCESS;
}

CameraCaptureIForm*
MainForm::GetStartForm(void)
{
        return __pStartForm;
}

void
MainForm::OnActionPerformed(const Osp::Ui::Control& source, int actionId)
{
        Frame *pFrame = null;
    	MessageBox msgBoxError;
    	int msgBoxErrorResult = 0;
    	BatteryLevel batterylevel = BATTERY_HIGH;
    	bool isCharging = false;

        pFrame = (Frame *)GetParent();

        Battery::GetCurrentLevel(batterylevel);
    	Osp::System::RuntimeInfo::GetValue(L"IsCharging", isCharging);

        AppLog("MainForm::OnActionPerformed:battery level = %d\n",batterylevel);

        switch( actionId )
        {

        case ID_BUTTON_CAMERA:
				{
					if( (BATTERY_CRITICAL == batterylevel || BATTERY_EMPTY == batterylevel) && !isCharging )
					{
						AppLog("Low Battery\n");
						msgBoxError.Construct(L"WARNING",L"Low Battery.",MSGBOX_STYLE_NONE,2000);
						msgBoxError.ShowAndWait(msgBoxErrorResult);
					}
					else
					{
						__StartCameraForm( pFrame, CAMERA_FORM  );
					}
				}
                break;
        case ID_BUTTON_PREVIEW_CHECKED:
        	__isPreviewScreenEnabled = true;
        	break;
        case ID_BUTTON_PREVIEW_UNCHECKED:
        	__isPreviewScreenEnabled = false;
        	break;
        case ID_BUTTON_PREVIEW_SELECTED:
        	break;
        case ID_BUTTON_CALLBACK_CHECKED:
        	__isCallbackEnabled = true;
        	break;
        case ID_BUTTON_CALLBACK_UNCHECKED:
        	__isCallbackEnabled = false;
        	break;
        case ID_BUTTON_CALLBACK_SELECTED:
        	break;
        default:
        	break;
        }
}

result
MainForm::__InitializeHeader(void)
{
	result r = E_SUCCESS;
	Header* pHeader = null;

	pHeader = GetHeader();
	if (!pHeader)
	{
		AppLogException("Getting header failed.");
		goto CATCH;
	}

	r = pHeader->SetTitleText(L"Camera Capture");
	if (IsFailed(r))
	{
		AppLogException("Header set description text failed.");
		goto CATCH;
	}

CATCH:
    return r;
}

result
MainForm::__InitializeFooter(void)
{
	result r = E_SUCCESS;
    Footer* pFooter = null;
	FooterItem itemCamera;

	pFooter = GetFooter();
	if (!pFooter)
	{
		AppLogException("Getting footer failed.");
		goto CATCH;
	}

	r = pFooter->SetStyle(FOOTER_STYLE_BUTTON_TEXT);
	if ( IsFailed(r))
	{
		AppLogException("Footer SetStyle failed.");
		goto CATCH;
	}

	r = itemCamera.Construct(ID_BUTTON_CAMERA);
	if ( IsFailed(r))
	{
		AppLogException("Camera button Construct failed.");
		goto CATCH;
	}

	r = itemCamera.SetText(L"Camera");
	if ( IsFailed(r))
	{
		AppLogException("Camera button SetText failed.");
		goto CATCH;
	}

	r = pFooter->AddItem(itemCamera);
	if ( IsFailed(r))
	{
		AppLogException("Footer AddItem failed.");
		goto CATCH;
	}

	pFooter->AddActionEventListener(*this);

CATCH:
	return r;
}

bool
MainForm::__StartCameraForm( Frame *pFrame, StartFormType formType )
{
        result r = E_SUCCESS;
        CameraStartType cameraStartType = CAMERA_START_NONE;
        if ( __isPreviewScreenEnabled )
        {
                if ( __isCallbackEnabled )
                        cameraStartType = CAMERA_START_PREVIEW_WITH_CALLBACK;
        	else
                        cameraStartType = CAMERA_START_PREVIEW_WITHOUT_CALLBACK;
        }
        else
        {
        	if ( __isCallbackEnabled )
                        cameraStartType = CAMERA_START_NO_PREVIEW_WITH_CALLBACK;
        	else
                        cameraStartType = CAMERA_START_NO_PREVIEW_WITHOUT_CALLBACK;
        }

        if ( __pStartForm )
        {
        	r = pFrame->RemoveControl(*__pStartForm);
        	if( IsFailed(r) )
        	{
        		AppLogException( "Remove camera form has failed.");
        		goto CATCH;
        	}
        	__pStartForm= NULL;
        }

        if ( formType == CAMERA_FORM )
                __pStartForm = new CameraForm();
        else
				goto CATCH;

        // Camera Construct

        AppLog( ">>>>>>Orientation : %d, camera start type:%d", __targetOrientation, cameraStartType );
        if( __pStartForm->Construct( pFrame, this, __targetOrientation, cameraStartType ) == E_SUCCESS )
        {
        	//------------------------------
        	// Attach Form to Frame
        	//------------------------------
        	r = pFrame->AddControl( *__pStartForm );
        	if( IsFailed(r))
        	{
        		AppLogException( "Adding CameraForm has failed.");
		        r = pFrame->SetCurrentForm( *this );
		        if( r == E_SUCCESS )
		        {
		            	pFrame->Draw();
		            	pFrame->Show();
		        }
	        	goto CATCH_CAMERA_ERROR;
        	}
        }
        else
        {
        	AppLogException( "Construct has failed.");
        	goto CATCH_CAMERA_ERROR;
        }

        r = __pStartForm->InitButtons(__targetOrientation);
        if ( IsFailed(r) )
        {
        	goto CATCH_UI_ERROR;
        }

        //Assign the current form
        r = pFrame->SetCurrentForm( *__pStartForm );
        if( r == E_SUCCESS )
        {
            	r = pFrame->Draw();
                if ( IsFailed(r) )
                {
                	goto CATCH_UI_ERROR;
                }

            	r = pFrame->Show();
                if ( IsFailed(r) )
                {
                	goto CATCH_UI_ERROR;
                }
        }
        else
        {
        	goto CATCH_UI_ERROR;
        }

        return true;

CATCH_UI_ERROR:
		pFrame->RemoveControl(*__pStartForm);
		__pStartForm= NULL;
CATCH_CAMERA_ERROR:
		delete __pStartForm;
		__pStartForm= NULL;
CATCH:
		return false;
}

result
MainForm::__InitializePreviewCheckButton()
{
        result r = E_SUCCESS;

        __pCheckBtnPreview = new CheckButton;
        if( !__pCheckBtnPreview) return E_OUT_OF_MEMORY;

        r = __pCheckBtnPreview->Construct( Rectangle(X_FROM_LEFT(0,BTN_WIDTH,BTN_WIDTH_MARGIN)
        											,Y_FROM_TOP(1,BTN_HEIGHT,BTN_HEIGHT_MARGIN)
        											,BTN_WIDTH
        											,BTN_HEIGHT)
        											,CHECK_BUTTON_STYLE_MARK, BACKGROUND_STYLE_DEFAULT, false, String("Direct Preview"));
        if( IsFailed(r) )
        {
        	AppLogException( "Preview button constructing has failed." );
        	delete __pCheckBtnPreview;
        	return r;
        }

        r = AddControl(*__pCheckBtnPreview);
        if( IsFailed(r))
        {
        	AppLogException( "Adding Preview button has failed." );
        	delete __pCheckBtnPreview;
        	return r;
        }

        __pCheckBtnPreview->SetActionId(ID_BUTTON_PREVIEW_CHECKED, ID_BUTTON_PREVIEW_UNCHECKED, ID_BUTTON_PREVIEW_SELECTED);
        __pCheckBtnPreview->AddActionEventListener(*this);

        return r;
}


result
MainForm::__InitializeCallbackCheckButton()
{
        result r = E_SUCCESS;

        __pCheckBtnCallback = new CheckButton;
        if( !__pCheckBtnCallback) return E_OUT_OF_MEMORY;

        r = __pCheckBtnCallback->Construct( Rectangle(X_FROM_LEFT(0,BTN_WIDTH,BTN_WIDTH_MARGIN)
        											,Y_FROM_TOP(2,BTN_HEIGHT,BTN_HEIGHT_MARGIN)
        											,BTN_WIDTH,BTN_HEIGHT)
        											,CHECK_BUTTON_STYLE_MARK,BACKGROUND_STYLE_DEFAULT, false, String("Callback"));
        if( IsFailed(r) )
        {
        	AppLogException( "Callback button constructing has failed." );
        	delete __pCheckBtnCallback;
        	return r;
        }

        r = AddControl(*__pCheckBtnCallback);
        if( IsFailed(r))
        {
        	AppLogException( "Adding Callback button has failed." );
        	delete __pCheckBtnCallback;
        	return r;
        }

        __pCheckBtnCallback->SetActionId(ID_BUTTON_CALLBACK_CHECKED, ID_BUTTON_CALLBACK_UNCHECKED, ID_BUTTON_CALLBACK_SELECTED);
        __pCheckBtnCallback->AddActionEventListener(*this);

        return r;
}
