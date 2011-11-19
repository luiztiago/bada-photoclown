#pragma once

#include <FBase.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FApp.h>
#include <FContent.h>
#include <FMedia.h>
#include <FIo.h>
#include <FSystem.h>


#include "CameraCaptureResource.h"
#include "CameraCaptureIForm.h"

#define BTN_HEIGHT 72
#define BTN_HEIGHT_MARGIN 20

#define BTN_WIDTH (FORM_WIDTH-BTN_WIDTH_MARGIN*2)
#define BTN_WIDTH_MARGIN 10

#define SMALL_BTN_WIDTH 140
#define SMALL_BTN_HEIGHT 60

#define FORM_X GetClientAreaBounds().x
#define FORM_Y GetClientAreaBounds().y
#define FORM_WIDTH GetClientAreaBounds().width
#define FORM_HEIGHT GetClientAreaBounds().height

#define Y_FROM_TOP(ORDER,HEIGHT,MARGIN) (FORM_Y +(HEIGHT*(ORDER))+(MARGIN*(ORDER+1)))
#define Y_FROM_BOTTOM(ORDER,HEIGHT,MARGIN) (FORM_HEIGHT -(HEIGHT*(ORDER+1))-(MARGIN*(ORDER+1)))

#define X_FROM_LEFT(ORDER,WIDTH,MARGIN) (FORM_X +(WIDTH*(ORDER))+(MARGIN*(ORDER+1)))
#define X_FROM_RIGHT(ORDER,WIDTH,MARGIN) (FORM_WIDTH -(WIDTH*(ORDER+1))-(MARGIN*(ORDER+1)))

class MainForm :
	public Osp::Ui::Controls::Form,
	public Osp::Ui::IActionEventListener
{
public:
	MainForm(void);
	virtual ~MainForm(void);
	result  OnInitializing (void);
	result  OnTerminating (void);
	result Construct();

	void OnActionPerformed(const Osp::Ui::Control& source, int actionId);
	CameraCaptureIForm* GetStartForm(void);

private:
    result __InitializeHeader(void);
	result __InitializeFooter(void);

	bool __StartCameraForm( Osp::Ui::Controls::Frame *pFrame, StartFormType formType );

	CameraCaptureIForm		*__pStartForm;
	Osp::Ui::Controls::Button		*__pBtnCameraStart, *__pBtnCameraRecorderStart;
	Osp::Ui::Controls::CheckButton	*__pCheckBtnCallback, *__pCheckBtnPreview;
	bool 	__isCallbackEnabled, __isPreviewScreenEnabled;
	Osp::Ui::Orientation		__targetOrientation;
};
