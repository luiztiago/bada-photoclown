#include "CameraCapture.h"
#include "CameraCapturePreviewForm.h"
#include "CameraCaptureMainForm.h"
#include <FSysPowerManager.h>

using namespace Osp::Base;
using namespace Osp::Graphics;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Locales;
using namespace Osp::System;
using namespace Osp::App;
using namespace Osp::Ui::Controls;

bool __isBackGround;

CameraCapture::CameraCapture()
{
	__pFrame = null;
	__pMainForm = null;
	__isBackGround = false;
}


CameraCapture::~CameraCapture()
{
}


Application*
CameraCapture::CreateInstance(void)
{
	// You can create the instance through another constructor.
	return new CameraCapture();
}


bool
CameraCapture::OnAppInitializing(AppRegistry& appRegistry)
{
	result r = E_SUCCESS;

	IAppFrame* pAppFrame = GetAppFrame();
	if (NULL == pAppFrame)
	{
		AppLog("GetAppFrame has failed..");
		goto CATCH;
	}

	__pFrame = pAppFrame->GetFrame();
	if (!__pFrame)
	{
		AppLog("GetFrame has failed..");
		goto CATCH;
	}

	// initialize forms
	__pMainForm = new MainForm();
	if( !__pMainForm )
	{
		AppLog( ">>>>>> MainForm creation has failed.");
		return false;
	}

	//------------------------------
	// Construct form
	//------------------------------
	if( __pMainForm->Construct() != E_SUCCESS )
	{
		AppLog( ">>>>>> Construct has failed.");
		goto CATCH;
	}

	//------------------------------
	// Attach Form to Frame
	//------------------------------
	r = __pFrame->AddControl( *__pMainForm );
	if( IsFailed(r))
	{
		AppLog( ">>>>>> Adding MainForm has failed.");
		goto CATCH;
	}

	__pFrame->SetCurrentForm( *__pMainForm );

	// You should comment following statement if you do not listen to the screen on/off events.
	PowerManager::SetScreenEventListener(*this);

	return true;

CATCH:
	if ( __pMainForm )
		delete __pMainForm;

	return false;
}


bool
CameraCapture::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination)
{
	// TODO:
	// Deallocate or close any resources still alive.
	// Save the application's current states, if applicable.
	// If this method is successful, return true; otherwise, return false.

	return true;
}


void
CameraCapture::OnForeground(void)
{
	AppLog( ">>>>>> OnForeground is called.");
	__isBackGround = false;
	Osp::System::BatteryLevel eBatterLevel;
   	bool isCharging = false;
	Osp::System::Battery::GetCurrentLevel(eBatterLevel);
   	Osp::System::RuntimeInfo::GetValue(L"IsCharging", isCharging);

	if( (BATTERY_CRITICAL != eBatterLevel && BATTERY_EMPTY != eBatterLevel) || isCharging)
	{
		AppLog("--------------Normal BatterY Condition enum - %d",eBatterLevel);
		AppStartUp();
	}
	else
	{
		AppLog("--------------Critical BatterY Condition enum - %d",eBatterLevel);
		MessageBox msgBoxError;
		int msgBoxErrorResult = 0;
		if(__pFrame->GetCurrentForm() != __pMainForm)
		{
		msgBoxError.Construct(L"WARNING",L"Low Battery",MSGBOX_STYLE_NONE,1000);
		msgBoxError.ShowAndWait(msgBoxErrorResult);
		}
		AppHandleLowBattery();
	}

}


void
CameraCapture::OnBackground(void)
{
	AppLog( ">>>>>> OnBackground is called.");
	__isBackGround = true;
	AppCleanUp();
}


void
CameraCapture::OnLowMemory(void)
{
	// TODO:
	// Deallocate as many resources as possible.
}


void
CameraCapture::OnBatteryLevelChanged(BatteryLevel batteryLevel)
{
	// TODO:
	// It is recommended that the application save its data,
	// and terminate itself if the application consumes much battery.

   	bool isCharging = false;
   	Osp::System::RuntimeInfo::GetValue(L"IsCharging", isCharging);

	if( (BATTERY_CRITICAL == batteryLevel || BATTERY_EMPTY == batteryLevel ) && !isCharging && !__isBackGround)
	{
		AppLog("----------Handling Low Battery");
		AppHandleLowBattery();
	}
}


void
CameraCapture::OnScreenOn (void)
{
	// TODO:
	// Get the released resources or resume the operations that were paused or stopped in OnScreenOff().
	AppLog( ">>>>>> OnScreenOn is called.");
	AppStartUp();
}

void
CameraCapture::OnScreenOff (void)
{
	// TODO:
	//  Unless there is a strong reason to do otherwise, release resources (such as 3D, media, and sensors) to allow the device to enter the sleep mode to save the battery.
	// Invoking a lengthy asynchronous method within this listener method can be risky, because it is not guaranteed to invoke a callback before the device enters the sleep mode.
	// Similarly, do not perform lengthy operations in this listener method. Any operation must be a quick one.

	AppLog( ">>>>>> OnScreenOff is called.");
	AppCleanUp();
}

void
CameraCapture::AppCleanUp(void)
{
	if ( __pFrame->GetCurrentForm() == __pMainForm )
	{
		// do nothing.
	}
	else if ( __pMainForm && __pMainForm->GetStartForm()
			&& ( __pMainForm->GetStartForm()->IsStarted() || __pMainForm->GetStartForm()->IsSourceStarted() ))
	{
		__pMainForm->GetStartForm()->CleanUp();
	}
}
void
CameraCapture::AppStartUp(void)
{
	if ( __pFrame->GetCurrentForm() == __pMainForm )
	{
		// do nothing.
	}
	else if ( __pMainForm && __pMainForm->GetStartForm() && !__pMainForm->GetStartForm()->IsSourceStarted() )
	{
		__pMainForm->GetStartForm()->Start();
		__pMainForm->GetStartForm()->ShowButtons();
	}
}
void
CameraCapture::AppHandleLowBattery(void)
{
	if ( __pFrame->GetCurrentForm() == __pMainForm )
	{
		// do nothing.
		AppLog("--------------Critical BatterY Condition MainForm");
	}
	else if ( __pMainForm && __pMainForm->GetStartForm())
	{
		AppLog("--------------Critical BatterY Condition StartForm");
		__pMainForm->GetStartForm()->HandleLowBatteryCondition();
	}
}
