#ifndef __CAMERACAPTURE_H__
#define __CAMERACAPTURE_H__


#include <FBase.h>
#include <FGraphics.h>
#include <Fui.h>
#include <FApp.h>
#include "CameraCaptureMainForm.h"
#include <FSystem.h>

class CameraCapture :
	public Osp::App::Application,
	public Osp::System::IScreenEventListener
{
public:
	// The application must have a factory method that creates an instance of the application.
	static Osp::App::Application* CreateInstance(void);


public:
	CameraCapture();
	~CameraCapture();

public:

	//Called when the application is initializing.
	bool OnAppInitializing(Osp::App::AppRegistry& appRegistry);

	//Called when the application is terminating.
	bool OnAppTerminating(Osp::App::AppRegistry& appRegistry, bool forcedTermination = false);

	// Called when the application's frame moves to the top of the screen.
	void OnForeground(void);

	//Called when this application's frame is moved from top of the screen to the background.
	void OnBackground(void);

	// Called when the system memory is not sufficient to run the application any further.
	void OnLowMemory(void);

	//Called when the battery level changes.
	void OnBatteryLevelChanged(Osp::System::BatteryLevel batteryLevel);

	 //	Called when the screen turns on.
	void OnScreenOn (void);

	 //	Called when the screen turns off.
	void OnScreenOff (void);

private:
	MainForm* __pMainForm;
	Osp::Ui::Controls::Frame*	__pFrame;
	void AppCleanUp(void);
	void AppStartUp(void);
	void AppHandleLowBattery(void);
};

#endif
