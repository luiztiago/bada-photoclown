#include "CameraCaptureMainForm.h"
#include "CameraRecorderForm.h"

#define __DEBUG
#define SAMPLE_FILE L"/Home/SampleVideo"

using namespace Osp::Base;
using namespace Osp::Base::Collection;
using namespace Osp::Ui::Controls;
using namespace Osp::Ui;
using namespace Osp::Graphics;
using namespace Osp::Media;
using namespace Osp::App;

extern bool __isBackGround;
static String SampleVideoFile;

CameraRecorderForm::CameraRecorderForm(void)
{
	__pCamera = null;
	__pCameraRecorder = null;
	__isStarted = false;
	__isCanceled = false;
	__isFromError = false;
	__pBackButton = null;
	__pRecordButton = null;
	__pStopButton = null;
}

CameraRecorderForm::~CameraRecorderForm(void)
{
	if ( __pCameraRecorder )
	{
		delete __pCameraRecorder;
		__pCameraRecorder = null;
	}
}


result
CameraRecorderForm::Construct(  Frame *pFrame, Form *pMainForm, Orientation orientation, CameraStartType cameraStartType )
{
	result r = E_SUCCESS;

	r = CameraForm::Construct (  pFrame, pMainForm, orientation, cameraStartType );
	if(IsFailed(r))
	{
		AppLogException( "Constructing form failed.");
		return r;
	}

	r = __timer.Construct(*this);
	if (IsFailed(r))
	{
		AppLogException( "Timer construct failed.");
		return r;
	}

	return r;
}


result
CameraRecorderForm::OnInitializing(void)
{
	result r = E_SUCCESS;

	r = CameraForm::OnInitializing();
	if ( IsFailed(r) )
	{
	    return r;
	}

	__pCamera = GetCamera();

	r = __InitCameraRecorder();
	if ( IsFailed(r) )
	{
		return r;
	}

	return E_SUCCESS;
}

result
CameraRecorderForm::OnTerminating(void)
{
	result r = E_SUCCESS;
	Stop();
	r = CameraForm::OnTerminating();
	return E_SUCCESS;
}

void
CameraRecorderForm::OnActionPerformed(const Control& source, int actionId)
{
	result r = E_SUCCESS;

	if( actionId == IDC_VREC_RECORD )
	{
		_SetButtonEnabled(__pBackButton, false);
		_SetButtonEnabled(__pRecordButton, false);
		r = StartRecord();
		if(r != E_SUCCESS)
		{
			_SetButtonEnabled(__pBackButton, true);
			_SetButtonEnabled(__pRecordButton, true);
		}
	}
	else if( actionId == IDC_VREC_STOP )
	{
		r = Stop();
	}
	else if( actionId == IDC_GOTO_MAIN )
	{
		r = Cancel();
		_GotoMainForm();
	}
}

result
CameraRecorderForm::StartRecord( void )
{
	result r = E_SUCCESS;
	MessageBox msgBoxError;
	int msgBoxErrorResult = 0;

    AppLog("Start Enter.");

	if ( !CameraForm::IsStarted() )
	{
		r = CameraForm::Start();
		if ( IsFailed(r) )
		{
			__isFromError =  true;
			AppLogException("Start CameraForm failed");
		    goto CATCH_CAMERA_ERROR;
		}
	}

	if ( __pCameraRecorder->GetState() != RECORDER_STATE_INITIALIZED
			&& __pCameraRecorder->GetState() != RECORDER_STATE_CLOSED )
	{
		goto CATCH_STATE_PASS;
	}

	if ( !IsStarted() )
	{
		r = __pCameraRecorder->CreateVideoFile( SampleVideoFile, true);
		if ( IsFailed(r))
	 	{
			__isFromError =  true;
			AppLogException("CreateVideoFile failed");
			goto CATCH_RECORDING_ERROR;
	 	}
		__recordingFile = SampleVideoFile;

		r = __pCameraRecorder->StartRecord();
		if ( IsFailed(r) )
		{
			__isFromError =  true;
			AppLogException("Record failed");
			__pCameraRecorder->Close();
			goto CATCH_RECORDING_ERROR;
		}


		r = __updateTimeSize( true );
		if (IsFailed(r))
		{
			AppLogException( " Reset time and size failed");
			goto CATCH_TIMER_ERROR;
		}
	}

	return E_SUCCESS;

CATCH_CAMERA_ERROR:
	msgBoxError.Construct(L"WARNING",L"Camera starting failed.",MSGBOX_STYLE_OK,0);
	msgBoxError.ShowAndWait(msgBoxErrorResult);
	__recordingFile.Clear();
	__isFromError =  false;
	return r;

CATCH_RECORDING_ERROR:
	msgBoxError.Construct(L"WARNING",L"Recording failed.",MSGBOX_STYLE_OK,0);
	msgBoxError.ShowAndWait(msgBoxErrorResult);
	__recordingFile.Clear();
	__isFromError =  false;
	return r;

CATCH_TIMER_ERROR:
	__recordingFile.Clear();
	__isFromError =  false;
	return r;

CATCH_STATE_PASS:
	return E_SUCCESS;
}

result
CameraRecorderForm::Stop()
{
	result r = E_SUCCESS;
    AppLog("Stop Enter.");

	if ( IsStarted() )
	{
		r = __pCameraRecorder->StopRecord();
		if ( IsFailed(r) )
		{
			AppLogException("Recording Stop failed");
			goto CATCH;
		}

		r = __timer.Cancel();
		if (IsFailed(r))
		{
			AppLogException( " Timer cancel failed.");
			goto CATCH;
		}

		if(true == __isBackGround)
		{
			AppLog("OnVideoRecorderStopped: App is in BackGround: Clear the recorded file\n");
			__recordingFile.Clear();
		}

	}

	return E_SUCCESS;

CATCH:
	return r;
}

result
CameraRecorderForm::Cancel()
{
	result r = E_SUCCESS;
    AppLog("Cancel Enter.");

	if ( IsStarted() )
	{
		r = __pCameraRecorder->Cancel();
		if ( IsFailed(r) )
		{
			AppLogException("Recording Cancel failed");
		}

		r = __timer.Cancel();
		if (IsFailed(r))
		{
			AppLogException( " Timer cancel failed.");
			goto CATCH;
		}
	}

	if ( CameraForm::IsStarted() )
	{
		r = CameraForm::Cancel();
		if ( IsFailed(r) )
		{
			AppLogException("Cancel CameraForm failed");
			goto CATCH;
		}
	}

	return E_SUCCESS;

CATCH:
	return r;
}

bool
CameraRecorderForm::CleanUp(void)
{
	result r;
	r = this->Stop();
	if ( IsFailed(r) )
	{
		AppLogException("Stop recording failed");
		goto CATCH;
	}

	if( CameraForm::IsStarted() )
	{
		r = CameraForm::Cancel();
		if ( IsFailed(r) )
		{
			AppLogException("Cancel camera failed");
			goto CATCH;
		}
	}
	return true;
CATCH:
		return false;
}

bool
CameraRecorderForm::HandleLowBatteryCondition(void)
{
	result r = E_SUCCESS;
	AppLog("CameraRecorderForm::HandleLowBatteryCondition\n");
	AppLog("Battery Level is Low: Exit Recorder and go to MainForm\n");
	__recordingFile.Clear();
	r = Cancel();
	_GotoMainForm();
	return true;
}

bool
CameraRecorderForm::IsStarted(void)
{
	return __isStarted;
}

bool
CameraRecorderForm::IsSourceStarted(void)
{
	return CameraForm::IsStarted();
}

result
CameraRecorderForm::InitButtons( Orientation orientation )
{
    result r = E_SUCCESS;
    Rectangle backButtonRect, recordButtonRect, stopButtonRect;
    AppLog("orientation : %d", orientation);

	backButtonRect = Rectangle(X_FROM_RIGHT(0,SMALL_BTN_WIDTH,BTN_WIDTH_MARGIN)
									,Y_FROM_TOP(0,SMALL_BTN_HEIGHT,BTN_HEIGHT_MARGIN)
									,SMALL_BTN_WIDTH
									,SMALL_BTN_HEIGHT);

	recordButtonRect = Rectangle(X_FROM_RIGHT(0,SMALL_BTN_WIDTH,BTN_WIDTH_MARGIN)
									,Y_FROM_TOP(1,SMALL_BTN_HEIGHT,BTN_HEIGHT_MARGIN)
									,SMALL_BTN_WIDTH
									,SMALL_BTN_HEIGHT);
	stopButtonRect = Rectangle(X_FROM_RIGHT(0,SMALL_BTN_WIDTH,BTN_WIDTH_MARGIN)
									,Y_FROM_TOP(2,SMALL_BTN_HEIGHT,BTN_HEIGHT_MARGIN)
									,SMALL_BTN_WIDTH
									,SMALL_BTN_HEIGHT);

    __pBackButton = new Button();
    r = __pBackButton->Construct(backButtonRect,L"Back");
    if(IsFailed(r))
    {
    	AppLogException( "__pBackButton construct failed.");
        goto CATCH;
    }
    __pRecordButton = new Button();
    r = __pRecordButton->Construct(recordButtonRect,L"Record");
    if(IsFailed(r))
    {
    	AppLogException( "__pRecordButton construct failed.");
        goto CATCH;
    }
    __pStopButton = new Button();
    r = __pStopButton->Construct(stopButtonRect,L"Stop");
    if(IsFailed(r))
    {
    	AppLogException( "__pStopButton construct failed.");
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

    __pRecordButton->SetActionId(IDC_VREC_RECORD);
    __pRecordButton->AddActionEventListener(*this);
    r = AddControl(*__pRecordButton);
    if(IsFailed(r))
    {
    	AppLogException( "Adding __pRecordButton failed.");
        goto CATCH;
    }

    __pStopButton->SetActionId(IDC_VREC_STOP);
    __pStopButton->AddActionEventListener(*this);
    r = AddControl(*__pStopButton);
    if(IsFailed(r))
    {
    	AppLogException( "Adding __pStopButton failed.");
        goto CATCH;
    }

	_SetButtonEnabled(__pRecordButton, true);
	_SetButtonEnabled(__pStopButton, false);

    return r;

CATCH:
	RemoveControl(*__pBackButton);
	RemoveControl(*__pRecordButton);
	RemoveControl(*__pStopButton);
    return r;
}

result
CameraRecorderForm::ShowButtons()
{
	_SetButtonEnabled(__pRecordButton, true);
	_SetButtonEnabled(__pBackButton, true);
	_SetButtonEnabled(__pStopButton,  false);
	return E_SUCCESS;
}

result
CameraRecorderForm::__InitCameraRecorder( void )
{
	result r = E_SUCCESS;
	IListT<CodecType>* pAudioCodecList = null;
	IListT<CodecType>* pVideoCodecList = null;
	IListT<MediaContainerType>* pMediaContainerList = null;
	IList* pRecordingResolutionList = null;
	CodecType audioCodec = CODEC_NONE;
	CodecType videoCodec = CODEC_NONE;
	MediaContainerType mediaContainer = MEDIA_CONTAINER_NONE;
	Dimension cameraPreviewDim;
	String fileExt;

	__pCameraRecorder = new CameraRecorder();
	if(null == __pCameraRecorder)
	{
		AppLogException("Creating CameraRecorder failed..");
		return E_OUT_OF_MEMORY;
	}

	r = __pCameraRecorder->Construct(*this, *__pCamera);
	if (IsFailed(r))
	{
		AppLogException("Construct of camera recorder failed.");
		goto CATCH;
	}

	pAudioCodecList = __pCameraRecorder->GetSupportedAudioCodecListN();
	if (pAudioCodecList == null )
	{
		AppLogException("There is no supported audio codec.");
		goto CATCH;
	}
#ifdef __DEBUG
	{
		int count = pAudioCodecList->GetCount ();
		for ( int i = 0; i < count; i++ )
		{
			pAudioCodecList->GetAt(i,audioCodec);
			AppLog("Codec[%dst] is 0x%x", i, (int)audioCodec);
		}
	}
#endif

	pVideoCodecList = __pCameraRecorder->GetSupportedVideoCodecListN();
	if (pAudioCodecList == null )
	{
		AppLogException("There is no supported audio codec.");
		goto CATCH;
	}
#ifdef __DEBUG
	{
		int count = pVideoCodecList->GetCount ();
		for ( int i = 0; i < count; i++ )
		{
			pVideoCodecList->GetAt(i, videoCodec);
			AppLog("Codec[%dst] is 0x%x", i, (int)videoCodec);
		}
	}
#endif

	pMediaContainerList = __pCameraRecorder->GetSupportedContainerListN();
	if (pMediaContainerList == null )
	{
		AppLogException("There is no supported container.");
		goto CATCH;
	}
#ifdef __DEBUG
	{
		int count = pMediaContainerList->GetCount ();
		for ( int i = 0; i < count; i++ )
		{
			pMediaContainerList->GetAt(i, mediaContainer);
			AppLog("Container[%dst] is 0x%x", i, (int)mediaContainer);
		}
	}
#endif

	//Getting default codec and container
	r = __pCameraRecorder->GetFormat( audioCodec, videoCodec, mediaContainer);
    if (IsFailed(r))
	{
    	AppLogException("__pCameraRecorder->GetFormat has failed.");
		goto CATCH;
	}

	if (pAudioCodecList->Contains(CODEC_AAC)
		&&pVideoCodecList->Contains(CODEC_H264)
		&&pMediaContainerList->Contains(MEDIA_CONTAINER_MP4))
	{
		r = __pCameraRecorder->SetFormat( CODEC_AAC, CODEC_H264, MEDIA_CONTAINER_MP4);
		mediaContainer = MEDIA_CONTAINER_MP4;
		audioCodec = CODEC_AAC;
		videoCodec = CODEC_H264;
	}
	else if (pAudioCodecList->Contains(CODEC_AMR_NB)
		&&pVideoCodecList->Contains(CODEC_H263)
		&&pMediaContainerList->Contains(MEDIA_CONTAINER_3GP))
	{
		r = __pCameraRecorder->SetFormat( CODEC_AMR_NB, CODEC_H263, MEDIA_CONTAINER_3GP);
		mediaContainer = MEDIA_CONTAINER_3GP;
		audioCodec = CODEC_AMR_NB;
		videoCodec = CODEC_H263;
	}
	else
	{
		// system default setting
		r = E_SUCCESS;
	}

	if (IsFailed(r))
	{
		AppLogException("Setting codecs and container failed.");
		goto CATCH;
	}

	SampleVideoFile = String(SAMPLE_FILE);
	SampleVideoFile.Append(L".");
	switch (mediaContainer)
	{
		case MEDIA_CONTAINER_3GP:
			fileExt = L"3GP";
			break;
		case MEDIA_CONTAINER_MP4:
			fileExt = L"MP4";
			break;
		default:
			fileExt = L"unknown";
			break;
	}
	SampleVideoFile.Append(fileExt);

	/*  --------------------------------------------------------------------------------------------------------------------
	 *  This is important. VideoRecorder's recording resolution SHOULD follow the Camera's preview resolution.
	 */
	pRecordingResolutionList = __pCameraRecorder->GetSupportedRecordingResolutionListN();
	if (pRecordingResolutionList == null )
	{
		AppLogException("There is no supported recording resolution.");
		goto CATCH;
	}

	cameraPreviewDim = GetCamera()->GetPreviewResolution();

#ifdef __DEBUG
	{
		int count = pRecordingResolutionList->GetCount ();
		for ( int i = 0; i < count; i++ )
		{
			Dimension* pRecordingResolution = (Dimension*)pRecordingResolutionList->GetAt(i);
			AppLog("Resolution[%dst] is (%d, %d)", i, pRecordingResolution->width, pRecordingResolution->height);
		}
	}
#endif

	r = __pCameraRecorder->SetRecordingResolution(cameraPreviewDim);
	if (IsFailed(r))
	{
		// Camera's preview resolution should be same with one of recorder's supported resolutions.
		// Camera should be stopped for changing the preview resolution.
		AppLog("Currently camera's preview resolution(%d,%d) is not supported for the video recorder.", cameraPreviewDim.width, cameraPreviewDim.height);

		int recorderSupportedResolutionCount = pRecordingResolutionList->GetCount();
		if ( recorderSupportedResolutionCount <= 0 )
			goto CATCH;
		int i = recorderSupportedResolutionCount-1;

		r = CameraForm::Stop();
		if ( IsFailed(r) )
			goto CATCH;

		for ( ; i >= 0; i-- )
		{
			Object* pObj = null;
			Dimension* pDim = null;
			pObj = pRecordingResolutionList->GetAt( i );
			pDim = static_cast<Dimension*>(pObj);
			if ( pDim == null )
				goto CATCH;

			AppLog("Try to set camera preview resolution with video recorder's resolution(%d,%d).", pDim->width, pDim->height);
			r = GetCamera()->SetPreviewResolution(*pDim);
			if ( r == E_SUCCESS )
			{
				r = __pCameraRecorder->SetRecordingResolution(*pDim);
				if ( IsFailed(r) )
					goto CATCH;

				AppLog("Complete to set camera and video recorder's resolution to (%d,%d).", pDim->width, pDim->height);
				break;
			}
		}

		if ( i < 0 )
			goto CATCH;

		r = CameraForm::Start();
		if ( IsFailed(r) )
			goto CATCH;

	}


	/*
	 *  ------------------------------------------------------------------------------------------------
	 */

	r = __pCameraRecorder->SetMaxRecordingSize(1024*5);		//1024*5 Kb
	if (IsFailed(r))
	{
		AppLogException("Setting max recording size failed.");
		goto CATCH;
	}

	r = __pCameraRecorder->SetMaxRecordingTime(120*1000);	// 120 sec
	if (IsFailed(r))
	{
		AppLogException("Setting max recording time failed.");
		goto CATCH;
	}

	pAudioCodecList->RemoveAll();
	delete pAudioCodecList;
	pAudioCodecList = null;

	pVideoCodecList->RemoveAll();
	delete pVideoCodecList;
	pVideoCodecList = null;

	pMediaContainerList->RemoveAll();
	delete pMediaContainerList;
	pMediaContainerList = null;

	pRecordingResolutionList->RemoveAll(true);
	delete pRecordingResolutionList;
	pRecordingResolutionList = null;

	return E_SUCCESS;

CATCH:
	if ( pAudioCodecList )
	{
		pAudioCodecList->RemoveAll();
		delete pAudioCodecList;
	}

	if ( pVideoCodecList )
	{
		pVideoCodecList->RemoveAll();
		delete pVideoCodecList;
	}

	if ( pMediaContainerList )
	{
		pMediaContainerList->RemoveAll();
		delete pMediaContainerList;
	}

	if ( pRecordingResolutionList )
	{
		pRecordingResolutionList->RemoveAll(true);
		delete pRecordingResolutionList;
	}

	if ( __pCameraRecorder )
	{
		delete __pCameraRecorder;
		__pCameraRecorder = null;
	}
	return r;
}

result
CameraRecorderForm::__updateTimeSize( bool reset )
{
	result r = E_SUCCESS;
	int textXinWide = 520, textXinNarrow = 280, textYinWide = 410, textYinNarrow = 200, textWidth = 280, textHeight = 80;
	Canvas* pOverlayCanvas = GetOverlayCanvas();
	long recSizeInByte = 0, recTimeInMsec = 0;
	int recSizeInKb = 0, maxRecSizeInKb = 0, recTimeInSec = 0, maxRecTimeInSec = 0;

	maxRecSizeInKb = __pCameraRecorder->GetMaxRecordingSize();
	maxRecTimeInSec = __pCameraRecorder->GetMaxRecordingTime()/1000;
	if ( !reset )
	{
		recSizeInByte = __pCameraRecorder->GetRecordingSize();
		recTimeInMsec = __pCameraRecorder->GetRecordingTime();
	}

	r = pOverlayCanvas->Clear(Rectangle(_SelectivePosition(textXinWide,textXinNarrow)
										,_SelectivePosition(textYinWide,textYinNarrow)
										,textWidth,textHeight));
	if (IsFailed(r))
	{
		AppLogException( " Canvas' Clear failed.");
		return r;
	}

	/*
	 * Recording size
	 */
	if ( recSizeInByte >= 1024 )
		recSizeInKb = recSizeInByte/1024;			// byte to kilobyte
	else
		recSizeInKb = 0;

	r = pOverlayCanvas->DrawText( Point(_SelectivePosition(textXinWide,textXinNarrow)
										,_SelectivePosition(textYinWide,textYinNarrow)),
		L" " + Integer(recSizeInKb).ToString() +
		L" / " + Integer(maxRecSizeInKb).ToString() +
		L" KBytes",25,Color::COLOR_BLACK);
	if (IsFailed(r))
	{
		AppLogException( " DrawText, recording size failed.");
		return r;
	}

	/*
	 * Recording time
	 */
	if ( recTimeInMsec >= 1000 )
		recTimeInSec = recTimeInMsec/1000;			// msec to sec
	else
		recTimeInSec = 0;

	r = pOverlayCanvas->DrawText( Point(_SelectivePosition(textXinWide,textXinNarrow)
										,_SelectivePosition(textYinWide,textYinNarrow)+30),
		L" " + Integer(recTimeInSec).ToString() +
		L" / " + Integer(maxRecTimeInSec).ToString() +
		L" second",25,Color::COLOR_BLACK);
	if (IsFailed(r))
	{
		AppLogException( " DrawText, recording time failed.");
		return r;
	}

	pOverlayCanvas->Show(Rectangle(_SelectivePosition(textXinWide,textXinNarrow)
									,_SelectivePosition(textYinWide,textYinNarrow)
									,textWidth,textHeight));
	if (IsFailed(r))
	{
		AppLogException( " Canvas' Show failed.");
		return r;
	}

	return r;
}

result
CameraRecorderForm::__ViewVideo(Osp::Base::String& videoPath)
{
	result r = E_SUCCESS;
	AppControl* pAc = AppManager::FindAppControlN(APPCONTROL_PROVIDER_VIDEO, APPCONTROL_OPERATION_PLAY);
	if(pAc != null)
	{
		AppLog("Video player started.");
		ArrayList* pDataList = null;
		pDataList = new ArrayList();
		pDataList->Construct();
		String* pData = null;

		pData = new String(L"type:video");
		pDataList->Add(*pData);

		pData = new String(L"path:");
		(*pData).Append(videoPath);
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

void
CameraRecorderForm::OnVideoRecorderStopped(result r)
{
	AppLog("OnVideoRecorderStopped");
	AppLog("Recording time :%d", __pCameraRecorder->GetRecordingTime());
	AppLog("Recording size :%d", __pCameraRecorder->GetRecordingSize());
	__isStarted = false;

	if ( r == E_SUCCESS )
	{
		__isCanceled = false;
		_SetButtonEnabled(__pBackButton, false);
		_SetButtonEnabled(__pRecordButton, false);
		_SetButtonEnabled(__pStopButton, false);
		__pCameraRecorder->Close();
	}
	else
	{
		__isCanceled = true;
		_SetButtonEnabled(__pBackButton, false);
		_SetButtonEnabled(__pRecordButton, false);
		_SetButtonEnabled(__pStopButton, false);
		__pCameraRecorder->Close();
	}
}


void
CameraRecorderForm::OnVideoRecorderCanceled(result r)
{
	AppLog("OnVideoRecorderCanceled");
	AppLog("Recording time :%d", __pCameraRecorder->GetRecordingTime());
	AppLog("Recording size :%d", __pCameraRecorder->GetRecordingSize());
	__isStarted = false;
	__isCanceled = true;

	_SetButtonEnabled(__pBackButton, false);
	_SetButtonEnabled(__pRecordButton, false);
		_SetButtonEnabled(__pStopButton, false);
		__pCameraRecorder->Close();
	}


void
CameraRecorderForm::OnVideoRecorderStarted(result r)
{
	AppLog("OnVideoRecorderStarted");
	AppLog("Recording time :%d", __pCameraRecorder->GetRecordingTime());
	AppLog("Recording size :%d", __pCameraRecorder->GetRecordingSize());
	__isStarted = true;
	__isCanceled = false;

	if ( r == E_SUCCESS )
	{
		_SetButtonEnabled(__pBackButton, true);
		_SetButtonEnabled(__pRecordButton, false);
		_SetButtonEnabled(__pStopButton, true);
		r = __timer.Start(__inforTimerPeriod);
		if (IsFailed(r))
		{
			AppLogException( " Timer start failed.");
		}
	}
	else
	{
		_SetButtonEnabled(__pBackButton, false);
		_SetButtonEnabled(__pRecordButton, true);
		_SetButtonEnabled(__pStopButton, false);
		__pCameraRecorder->Close();
	}
}

void
CameraRecorderForm::OnVideoRecorderPaused(result r)
{
	AppLog("OnVideoRecorderPaused");
	AppLog("Recording time :%d", __pCameraRecorder->GetRecordingTime());
	AppLog("Recording size :%d", __pCameraRecorder->GetRecordingSize());
	__isStarted = true;
	__isCanceled = false;

	if ( r == E_SUCCESS )
	{
		_SetButtonEnabled(__pBackButton, true);
		_SetButtonEnabled(__pRecordButton, true);
		_SetButtonEnabled(__pStopButton, false);
	}
	else
	{
		_SetButtonEnabled(__pBackButton, true);
		_SetButtonEnabled(__pRecordButton, true);
		_SetButtonEnabled(__pStopButton, false);
		__pCameraRecorder->Close();
	}
}

void
CameraRecorderForm::OnVideoRecorderEndReached(RecordingEndCondition endCondition)
{
	AppLog("OnVideoRecorderEndReached");
	AppLog("Recording time :%d", __pCameraRecorder->GetRecordingTime());
	AppLog("Recording size :%d", __pCameraRecorder->GetRecordingSize());
	__isStarted = false;
	__isCanceled = false;

	_SetButtonEnabled(__pBackButton, false);
	_SetButtonEnabled(__pRecordButton, false);
	_SetButtonEnabled(__pStopButton, false);
	__updateTimeSize(false);
	__pCameraRecorder->Close();
}

void
CameraRecorderForm::OnVideoRecorderClosed(result r)
{
	AppLog("OnVideoRecorderClosed");
	if ( r == E_SUCCESS )
	{
		if ( CameraForm::IsStarted() )
		{
			AppLog(" CameraForm::IsStarted.. is true ");
			CameraForm::Cancel();
		}

		AppLog(" The value of __isCanceled :%d  __isFromError:: %d  __isBackGround ::%d ",__isCanceled,__isFromError,__isBackGround);

		if ( !__isCanceled && !__isFromError && !__recordingFile.IsEmpty() && !__isBackGround)
		{
			r = __ViewVideo(__recordingFile);
		}

		/*else
		{
			r = CameraForm::Start();
			if (IsFailed(r))
			{
				AppLog( "OnVideoRecorderClosed:CameraForm start failed. \n");
				return;
			}
		}*/
	}
	else
	{
		delete __pCameraRecorder;
		__pCameraRecorder = null;
	}


	__isFromError = false;
}

void
CameraRecorderForm::OnVideoRecorderErrorOccurred(RecorderErrorReason r)
{
	MessageBox msgBoxError;
	int msgBoxErrorResult = 0;

	AppLog("OnVideoRecorderErrorOccurred");
	AppLog("Recording time :%d", __pCameraRecorder->GetRecordingTime());
	AppLog("Recording size :%d", __pCameraRecorder->GetRecordingSize());
	__isStarted = false;
	__isCanceled = false;
	__isFromError = true;

	// An error occurred
	switch(r)
	{
	case RECORDER_ERROR_OUT_OF_STORAGE:
		msgBoxError.Construct(L"WARNING",L"Memory full",MSGBOX_STYLE_OK,0);
		break;
	case RECORDER_ERROR_STORAGE_FAILED:
		msgBoxError.Construct(L"WARNING",L"Storage Access Failed",MSGBOX_STYLE_OK,0);
		break;
	case RECORDER_ERROR_DEVICE_FAILED:
		msgBoxError.Construct(L"WARNING",L"Recording Device Failed",MSGBOX_STYLE_OK,0);
		break;
	default:
		msgBoxError.Construct(L"WARNING",L"An Error Occured",MSGBOX_STYLE_OK,0);
		break;
	}
	msgBoxError.ShowAndWait(msgBoxErrorResult);

	_SetButtonEnabled(__pBackButton, false);
	_SetButtonEnabled(__pRecordButton, false);
	_SetButtonEnabled(__pStopButton, false);

	__pCameraRecorder->Close();
}

void
CameraRecorderForm::OnTimerExpired (Osp::Base::Runtime::Timer &timer)
{
	result r = E_SUCCESS;
	AppLog("Enter.");

	if ( IsStarted())
	{
		__updateTimeSize(false);

		r = __timer.Start(__inforTimerPeriod);
		if (IsFailed(r))
		{
			AppLogException( "Timer start failed.");
			return;
		}
	}
}
