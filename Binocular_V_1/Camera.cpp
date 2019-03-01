#include "stdafx.h"
#include "Camera.h"
#include <string>

using namespace std;
//标准的空白构造函数
Camera::Camera()
{
	H_Camera = 0;
	IpAddr = 0;
	Width = 0;
	Height = 0;
	BytesPerFrame = 0;
	CameraName = "";
	MaxSize = 0;
	pFrames = NULL;
	Expose = 0;
	FrameRate = 30;
	isStreaming = false;
	TrigerMode = 0;
	FrameCount = 0;
	outLog = "Black data of Camera";
	//初始化可能需要Initialized Protection
}


Camera::~Camera()
{
	if (isStreaming)
	{
		Close();
	}
}

Camera::Camera(tPvUint32 ip_addr, std::string cam_name)
{
	H_Camera = 0;
	IpAddr = ip_addr;
	Width = 0;
	Height = 0;
	BytesPerFrame = 0;
	CameraName = cam_name;
	MaxSize = 0;
	pFrames = NULL;
	Expose = 0;
	FrameRate = 30;
	isStreaming = false;
	TrigerMode = 0;
	FrameCount = 0;
	outLog = "Camera " + CameraName + " has been created.";

	//初始化交换buffer
	Seq_Buffer = NULL;
	//初始化各类信号量
	for (int tNum = 0; tNum < THREADNUM; tNum++)
	{
		NextProcess[tNum] = CreateEvent(NULL, TRUE, FALSE, NULL);
		ProcessImage[tNum] = CreateEvent(NULL, TRUE, FALSE, NULL);
		threadState[tNum] = 1;
		threadProState[tNum] = 1;
	}
	g_mtx_swap = CreateMutex(NULL, FALSE, NULL);
	sequenceBusy = CreateEvent(NULL, TRUE, FALSE, NULL);//设置为false
	sequenceVacant = CreateEvent(NULL, TRUE, TRUE, NULL);//设置为true
	swap = CreateEvent(NULL, TRUE, FALSE, NULL);//设置为false
	swapOver = CreateEvent(NULL, TRUE, FALSE, NULL);//设置为true
}

bool Camera::AttrSet(AttrType attrtype, const char* name, const char* value)
{
	if (!H_Camera)
	{
		outLog = "Error: Attribute of Camera Setting Error: Camera is not open.";
		return false;
	}
	else
	{
		tPvErr Err;
		switch (attrtype)
		{
		case Enum:
		{
			Err = PvAttrEnumSet(H_Camera, name, value);
			break;
		}
		case Uint32:
		{
			Err = PvAttrUint32Set(H_Camera, name, atoi(value));
			break;
		}
		case Float32:
		{
			Err = PvAttrFloat32Set(H_Camera, name, atof(value));
			break;
		}
		case Int64:
		{
			Err = PvAttrInt64Set(H_Camera, name, atol(value));
			break;
		}
		case Bool:
		{
			Err = PvAttrBooleanSet(H_Camera, name, atoi(value));
			break;
		}
		case AttrType::String:
		{
			Err = PvAttrStringSet(H_Camera, name, value);
			break;
		}
		default:
			break;
		}
		if (Err != ePvErrSuccess)
		{
			outLog = "Warnning: Arrtibute of Camera Setting failed in Camera of value " + string(name) + " in camera " + CameraName;
		}
	}
}

bool Camera::AttrGet(AttrType attrtype, const char* name, char* value_s)
{
	//仿照上面
	outLog = "";
	if (!H_Camera)
	{
		outLog = "Error: Attribute of Camera Getting Error: Camera is not open.";
		return false;
	}
	else
	{
		tPvErr Err;
		switch (attrtype)
		{
		case Enum:
		{
			unsigned long buffer_size;
			Err = PvAttrEnumGet(H_Camera, name, value_s,50,&buffer_size);
			if (buffer_size > 50)
			{
				outLog = "Warnning: to long of attr size of value " + string(name);
			}
			break;
		}
		case Uint32:
		{
			unsigned long value;
			Err = PvAttrUint32Get(H_Camera, name, &value);
			_ltoa(value, value_s, 10);
			break;
		}
		case Float32:
		{
			//float value;
			//Err = PvAttrFloat32Get(H_Camera, name, &value);
			break;
		}
		case Int64:
		{
			long long value;
			Err = PvAttrInt64Get(H_Camera, name, &value);
			_ltoa(value, value_s, 20);
			break;
		}
		case Bool:
		{
			unsigned char flag;
			Err = PvAttrBooleanGet(H_Camera, name, &flag);
			if (flag == 0)
			{
				value_s = new char;
				value_s = "0";
			}
			else
			{
				value_s = new char;
				value_s = "1";
			}
			break;
		}
		case AttrType::String:
		{
			unsigned long buffer_size;
			Err = PvAttrStringGet(H_Camera, name, value_s, 50, &buffer_size);
			if (buffer_size > 50)
			{
				outLog = "Warnning: to long of attr size of value " + string(name);
			}
			break;
		}
		default:
			break;
		}

		if (Err != ePvErrSuccess)
		{
			outLog += "Warnning: Arrtibute of Camera Getting failed in Camera of value " + string(name) + " in camera " + CameraName;
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool Camera::Open()
{
	tPvErr Err;
	Err = PvCameraOpenByAddr(IpAddr, ePvAccessMaster, &H_Camera);
	if (Err != ePvErrSuccess)
	{
		Err = PvCameraOpenByAddr(IpAddr, ePvAccessMaster, &H_Camera);
		if (Err != ePvErrSuccess)
		{
			outLog = "Error: Can't Open Camera "+CameraName+" with IP";
			return false;
		}
	}

	int ErrCode = 0;
	//获取相机的基本属性参数
	ErrCode |= PvAttrUint32Get(H_Camera, "Width", &Width);
	ErrCode |= PvAttrUint32Get(H_Camera, "Height", &Height);
	ErrCode |= PvAttrUint32Get(H_Camera, "PacketSize", &MaxSize);
	ErrCode |= PvAttrUint32Get(H_Camera, "ExposureValue", &Expose);

	//设置属性
	ErrCode |= PvAttrEnumSet(H_Camera, "ExposureMode", "Manual");				//设定手动曝光模式
	ErrCode |= PvAttrEnumSet(H_Camera, "PixelFormat", "Mono8");					//像素格式为Mono8
	ErrCode |= PvAttrEnumSet(H_Camera, "AcquisitionMode", "Continuous");		//采集模式为连续
	
	ErrCode |= PvCaptureAdjustPacketSize(H_Camera, 8228);						//设定最大的数据传输量
	if (ErrCode != 0)
	{
		outLog = "Warnning: Can't Get&Set Camera's Attribues in Camera" + CameraName;
		return false;
	}


	//建立两个线程
	DWORD id ;
	h_seqThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SeqThread, this, 0, &id);
	syInfo t_info[THREADNUM];
	for (int tNum = 0; tNum < THREADNUM; tNum++)
	{
		t_info[tNum].num = tNum;
		t_info[tNum].ptr = this;
		h_proThread[tNum] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProThread, &t_info[tNum], 0, &id);
	}
	return true;
}

bool Camera::Close()
{
	if (isStreaming)
	{
		if (!CloseCapture())
		{
			return false;
		}
	}
	tPvErr Err;
	Err = PvCameraClose(H_Camera);
	if (Err != ePvErrSuccess)
	{
		outLog = "Warnning: Failed to close camera " + CameraName;
	}
	else
	{
		H_Camera = NULL;
		outLog = CameraName + " closed successful!";
	}
	return false;
}

bool Camera::StartCapture()
{
	if (isStreaming)
	{
		outLog = "Warnning: Camera: " + CameraName + " is Capturing now.";
		return false;
	}

	//试着从网口先打开相机
	if (H_Camera == NULL)
	{
		if (!Open())
		{
			return false;
		}
	}

	//初始中间变量
	tPvErr Err = ePvErrSuccess;
	int ErrCode = 0;
	int i, j;
	//申请空间
	pFrames = new tPvFrame[MAX_FRAMES];
	//慎重使用ZeroMemory
	ZeroMemory(pFrames, sizeof(tPvFrame)*MAX_FRAMES);

	//设置触发模式 在不同的情况下改变 则有不同的设定方式 触发模式选择从窗口获得
	switch (TrigerMode)
	{
	case 0:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "Freerun");
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	case 1:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "SyncIn1");
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	case 2:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "SyncIn2");
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	case 3:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "FixedRate");
		ErrCode |= PvAttrFloat32Set(H_Camera, "FrameRate", FrameRate);
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	default:
		break;
	}
	if (ErrCode != ePvErrSuccess)
	{
		outLog = "Error: Attribute setting Problem in " + CameraName + " ErrorCode: " + to_string(ErrCode);
		delete[] pFrames;
		return false;
	}
	



	//开始接收图像
	if (PvCaptureStart(H_Camera) == ePvErrSuccess)
	{
		//内存分配
		if (ePvErrSuccess == PvAttrUint32Get(H_Camera, "TotalBytesPerFrame", &BytesPerFrame) && BytesPerFrame)
		{
			for (i = 0; i < MAX_FRAMES; i++)
			{
				if (pFrames[i].ImageBufferSize != BytesPerFrame)
				{
					delete pFrames[i].ImageBuffer;
					pFrames[i].ImageBuffer = new BYTE[BytesPerFrame];
					pFrames[i].ImageBufferSize = BytesPerFrame;
				}
				if (!pFrames[i].ImageBuffer)
				{
					Err = ePvErrResources;
				}
				else
				{
					pFrames[i].Context[0] = this;				//给予相机类指针
					pFrames[i].Context[1] = (void*)H_Camera;	//给予相机标识
				}
				if (Err != ePvErrSuccess)
				{
					for (j = 0; j < i; j++)
					{
						//清除本相机已分配的序列图像数据缓存
						delete pFrames[j].ImageBuffer;
						pFrames[j].ImageBuffer = NULL;
					}
					PvCameraClose(H_Camera);
					H_Camera = NULL;
					outLog = "Error: No RAM for ImgBuffer of Camera " + CameraName;
					delete[] pFrames;
					return false;
				}
			}
		}
		else
		{
			outLog = "Error: BytesPerFrame cant get or it is 0 in " + CameraName;
			delete[] pFrames;
			return false;
		}

	

		//内存分配完毕 构建队列
		int ErrCode = 0;
		for (i = 0; i < MAX_FRAMES; i++)
		{
			ErrCode |= PvCaptureQueueFrame(H_Camera, &pFrames[i], FrameDoneCB);
		}

		//开启处理和队列线程
		seqThreState = 1;
		//proThreState = 1;
		SetEvent(NextProcess[0]);
		SetEvent(ProcessImage[0]);


		//队列构建完毕 开始读图
		if (ErrCode == ePvErrSuccess)
		{
			Err = PvCommandRun(H_Camera, "AcquisitionStart");//this Action will trigger one acquisition
			isStreaming = true;
			outLog = "Camera Open Successful in Camera:" + CameraName;
			return true;
		}
		else
		{
			PvCaptureQueueClear(H_Camera);
			PvCaptureEnd(H_Camera);
			H_Camera = NULL;
			outLog = "Error: Queue making Error in " + CameraName;
			for (i = 0; i < MAX_FRAMES; i++)
			{
				delete pFrames[i].ImageBuffer;
				pFrames[i].ImageBuffer = NULL;
			}
			delete[] pFrames;
			return false;
		}
	}
	else
	{
		outLog = "Error: Capture Start Error in " + CameraName;
		delete[] pFrames;
		return false;
	}
}

bool Camera::CloseCapture()
{
	int i = 0;
	tPvErr Err;
	//发送停止命令
	if (isStreaming)
	{
		Err = PvCommandRun(H_Camera, "AcquisitionStop");
		Sleep(200);
		if (Err == ePvErrSuccess)
		{
			Err = PvCaptureQueueClear(H_Camera);		//2、CaptureQueueClear


			//关闭线程
			seqThreState = 0;
			proThreState = 0;

			if (Err == ePvErrSuccess)
			{
				
				Err = PvCaptureEnd(H_Camera);
				if (Err == ePvErrSuccess)
				{
					isStreaming = false;
					outLog = CameraName + " stop Capturing.";
				}
				else
				{
					outLog = "Warnning: Failed to stop the capture of camera " + CameraName;
				}
			}
			else
			{
				outLog = "Warnning: Camera Queue Clear not finished in camera " + CameraName;
			}
		}
		else
		{
			outLog = "Warnning: Failed to stop the acquisition of camera " + CameraName;
		}

		//不论上述是否停止成功 都要进行清理
		for (i = 0; i<MAX_FRAMES; i++)
		{
			delete pFrames[i].ImageBuffer;
			pFrames[i].ImageBuffer = NULL;
		}
		if (!pFrames)
		{
			delete[]pFrames;
			pFrames = NULL;
		}

		if (Err == ePvErrSuccess)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		outLog = "Camera is not on Capturing. Camera " + CameraName;
	}


	
}

bool Camera::ChangeTrigerMode(int triger_mode)
{
	int ErrCode = 0;
	switch (triger_mode)
	{
	case 0:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "Freerun");
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	case 1:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "SyncIn1");
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	case 2:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "SyncIn2");
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	case 3:
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerMode", "FixedRate");
		ErrCode |= PvAttrFloat32Set(H_Camera, "FrameRate", FrameRate);
		ErrCode |= PvAttrEnumSet(H_Camera, "FrameStartTriggerEvent", "EdgeFalling");//下降沿触发
		break;
	default:
		break;
	}
	if (ErrCode != ePvErrSuccess)
	{
		outLog = "Error: Attribute setting Problem in " + CameraName + " ErrorCode: " + to_string(ErrCode);
		return false;
	}
	else
	{
		outLog = CameraName + " Camera has changed triggerMode to Index " + to_string(triger_mode);
		TrigerMode = triger_mode;
		return true;
	}
		
}

bool Camera::ChangeExposeValue(unsigned long evalue)
{
	int ErrCode = 0;
	ErrCode|= PvAttrUint32Set(H_Camera, "ExposureValue", evalue);
	ErrCode |= PvAttrUint32Get(H_Camera, "ExposureValue", &Expose);
	if (ErrCode == 0 && Expose == evalue)
	{
		outLog = CameraName + " expose has set to " + to_string(evalue);
		return true;
	}
	else
	{
		outLog = "Warnning: Expose time set failed in " + CameraName;
		return false;
	}
}

//相机每一帧采集完成后 对每一帧数据进行的操作 基本操作是将处理完的数据重新排回帧队列中 这个必须放在类外
//相机50hz最大 20ms一帧的处理速度
void __stdcall FrameDoneCB(tPvFrame * pFrame)
{
	Camera* this_cam = (Camera*)pFrame->Context[0];
	this_cam->FrameCount = pFrame->FrameCount;

	//对每一个进入的图像数据，copy到一个临时buffer中，最多等待100ms
	if (WaitForSingleObject(this_cam->sequenceVacant, 100) == WAIT_OBJECT_0)//等待堆栈线程存入堆栈，等待最多100ms
	{
		ResetEvent(this_cam->sequenceVacant);//接收到信号后，重置信号
		if (this_cam->Seq_Buffer != NULL)delete[]this_cam->Seq_Buffer;//重置存入堆栈内存
		this_cam->Seq_Buffer = new unsigned char[(pFrame->Width)*(pFrame->Height)];//数据复制给堆栈内存
		memcpy(this_cam->Seq_Buffer, pFrame->ImageBuffer, (pFrame->Width)*(pFrame->Height));//数据复制给堆栈内存
		SetEvent(this_cam->sequenceBusy);//copy 数据完毕，通知堆栈线程开始执行为有效信号
	}
	PvCaptureQueueFrame(tPvHandle(pFrame->Context[1]), pFrame, FrameDoneCB);
}

DWORD WINAPI SeqThread(LPVOID param)
{
	Camera* this_cam = (Camera*)param;
	IplImage* pImage = cvCreateImage(cvSize(this_cam->Width, this_cam->Height), IPL_DEPTH_8U, 1);//准备一个iplimage变量，用于保存
	Mat SaveImage;//提前准备出mat类变量
	while (this_cam->seqThreState > 0)
	{
		if (WaitForSingleObject(this_cam->sequenceBusy, 100) == WAIT_OBJECT_0)//等待回调中的copy完毕，就返回有效信号，等待100，其实由于循环相当于一直在等待，不过可以帮助其迅速关闭
		{
			ResetEvent(this_cam->sequenceBusy);//一旦执行，重置信号
			//这里可能需要判断相机的宽度和类中的宽度是否一致。
			memcpy(pImage->imageData, this_cam->Seq_Buffer, (this_cam->Width)*(this_cam->Height));
			SaveImage = cvarrToMat(pImage);//将数据拷贝到图像中，进一步转换为mat类

			this_cam->frameSequ.push_back(SaveImage);//将mat类存入一个mat的deque变量中，用于后面存，前面取
													//这里拷贝的逻辑是，一旦processSequ中的数据为空，则将frameSequ当前存入数据整体迁移过去
			if (WaitForSingleObject(this_cam->swap, 0) == WAIT_OBJECT_0)//这个就是判断当前processSequ是否为空的信号量
			{
				ResetEvent(this_cam->swap);//一但开始执行，信号量重置
				this_cam->processSequ.swap(this_cam->frameSequ);//deque迁移
				SetEvent(this_cam->swapOver);//一旦迁移完毕，通知后面可以继续处理
			}
			SetEvent(this_cam->sequenceVacant);//这个是存入完成后的
		}
	}
	cvReleaseImage(&pImage);
	return 0;
}
DWORD WINAPI ProThread(LPVOID param)
{
	syInfo* t_info1 = (syInfo*)param;//用于进行线程传入量
	int threadNum = t_info1->num;//代表当前线程的线程数
	int nextThreadNum = 0;//用于将线程循环起来，每一个线程找到其对应的下一个线程。
	Camera* this_cam = (Camera*)t_info1->ptr;
	if (threadNum == THREADNUM - 1)
		nextThreadNum == 0;
	else
		nextThreadNum = threadNum + 1;
	Mat image;//这个是用来保存获取进入当前线程图像的临时文件
	int num_circle = 0;//这个用来表示当前线程的循环次数，用于计算进入当前线程的图像文件序号
	int b_err;
	while (this_cam->threadState[threadNum] > 0)
	{
		int frameNum = threadNum + 1 + num_circle*THREADNUM;//通过循环次数，获取图像序号
		if (WaitForSingleObject(this_cam->NextProcess[threadNum], INFINITE) == WAIT_OBJECT_0)
			//用来接收上一个线程的信号，即上一个线程通知下一个线程可以开始对队列中图像进行预处理
		{
			b_err = 0;
			ResetEvent(this_cam->NextProcess[threadNum]);//重置信号量
			b_err = getImage(this_cam, image, nextThreadNum);//读取图像完成
			if (b_err == -1)
					break;
			if (b_err=1)//一旦之前成果拿到数据
			{//多线程并行进行图像预处理
				SetEvent(this_cam->NextProcess[nextThreadNum]);//即可以通过下一个线程开始获取图像
				char filename[50];
				sprintf(filename, "C1//C1Frame%05d.bmp", frameNum);
				imwrite(filename, image);
			}
		}
	}
	return 0;
}


// the function can get the image from the deque
int getImage(Camera* this_cam,Mat & image,int nextThreadNum)
{
	int b_err;
	if (!this_cam->threadState[THREADNUM])//判断当前的线程状态，如果线程已经被关闭，

	{
		SetEvent(this_cam->NextProcess[THREADNUM]);//就通知下面的线程不要堵塞再等待之后，再自行关闭。
		return -1;
	}
	WaitForSingleObject(this_cam->g_mtx_swap, INFINITE);//这个市对于堆栈线程的操作锁，保证
														//同时只有一个线程在访问该堆栈数据
	if (!this_cam->processSequ.empty())//判断当前用于处理的堆栈是否为空
	{
		//若不空
		image = this_cam->processSequ.front();//提取序列最前图像
		this_cam->processSequ.pop_front();//排出已提取图像
		ReleaseMutex(this_cam->g_mtx_swap);//释放锁定信号量

	}
	else
	{
		//若为空
		SetEvent(this_cam->swap);//置换
		ReleaseMutex(this_cam->g_mtx_swap);//锁定堆栈

		if (WaitForSingleObject(this_cam->swapOver, 100) == WAIT_OBJECT_0)
			//这个为置换成果信号，由于会存在图像采集停止，所以等待100ms
		{
			ResetEvent(this_cam->swapOver);//重置信号
			WaitForSingleObject(this_cam->g_mtx_swap, INFINITE);//锁定
			if (!this_cam->processSequ.empty())//判断是否为空，一般不为空
			{
				image = this_cam->processSequ.front();//提取数据
				this_cam->processSequ.pop_front();

			}
			ReleaseMutex(this_cam->g_mtx_swap);
		}
		else
		{
			//没有等待交换成果，默认为没有新的图像进入，不过一般这种情况不会出现
			//这个是一个用来备用调试的选项
			WaitForSingleObject(this_cam->g_mtx_swap, INFINITE);

			ReleaseMutex(this_cam->g_mtx_swap);
			SetEvent(this_cam->NextProcess[nextThreadNum]);
			b_err = 1;
		}
	}
	return b_err;
} 