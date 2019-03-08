#include "stdafx.h"
#include "Camera.h"
#include <string>
#include <thread>


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
	isSaving = false;
	TrigerMode = 0;
	FrameCount = 0;
	outLog = "Camera " + CameraName + " has been created.";
	filepath = "";
	//初始化交换buffer
	Seq_Buffer = NULL;
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

		//队列构建完毕 开始读图
		if (ErrCode == ePvErrSuccess)
		{
			InitThreads();
			//已知相机的规格和图像大小 首先建立一个Seq_buffer的大小，以后这个buffer就固定了
			Seq_Buffer = new unsigned char[Width*Height];
			//建立两组线程 一个是单线程 用于拷贝
			DWORD id;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SeqThread, this, 0, &id);
			//一个是多线程 用来处理内容
			syInfo t_info[THREADNUM];
			for (int tNum = 0; tNum < THREADNUM; tNum++)
			{
				t_info[tNum].num = tNum;
				t_info[tNum].ptr = this;
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProThread, &t_info[tNum], 0, &id);
				Sleep(10);		
			}
			//以上内容可以放在一个threadInit()函数里
			//发送相机命令
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
		Sleep(200);	//停止200ms 
		CloseThreads();	//关闭所有线程
		//删除内存中的图
		delete[]Seq_Buffer;
		Seq_Buffer = NULL;
		if (Err == ePvErrSuccess)
		{
			Err = PvCaptureQueueClear(H_Camera);		//2、CaptureQueueClear

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

		//不论上述是否停止成功 都要进行清理 这里应该可以有更好的处理方法
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

		//确定返回值
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
		return true;
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
	ErrCode |= PvAttrUint32Set(H_Camera, "ExposureValue", evalue);
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

void Camera::InitThreads()
{
	//开启处理和队列flag,开启后开启它 关闭先关闭它
	seqThreadState = 1;
	acqThreadState = 1;
	for (int i = 0; i < THREADNUM; i++)
		proThreadState[i] = 1;

	//初始化各类信号量
	for (int tNum = 0; tNum < THREADNUM; tNum++)
		NextProcess[tNum] = CreateEvent(NULL, TRUE, FALSE, NULL);		//初始化都为false
	sequenceBusy = CreateEvent(NULL, TRUE, FALSE, NULL);		//设置为false
	sequenceVacant = CreateEvent(NULL, TRUE, TRUE, NULL);		//设置为true
	swap = CreateEvent(NULL, TRUE, FALSE, NULL);				//设置为false
	swapOver = CreateEvent(NULL, TRUE, FALSE, NULL);			//设置为true

	SetEvent(NextProcess[0]);	//开启第一个的处理线程
}

void Camera::CloseThreads()
{
	//关闭线程
	acqThreadState = 0;	//这个直接就可以导致采集线程停止 因为采集线程依靠这个使每个CB函数都退出
	seqThreadState = 0;	//这个直接就可以导致队列线程停止 因为队列线程是依靠这个量无限循环的 直接return0
	for (int i = 0; i < THREADNUM; i++)
		proThreadState[i] = 0;		//这个直接可以导致处理线程停止 因为处理线程是靠这个量无限循环的 直接return0

	//等待所有线程结束 110>100
	Sleep(110);
	//关闭所有信号量 
	for (int i = 0; i < THREADNUM; i++)
		CloseHandle(NextProcess[i]);
	CloseHandle(sequenceBusy);
	CloseHandle(sequenceVacant);
	CloseHandle(swap);
	CloseHandle(swapOver);
}

//相机每一帧采集完成后 对每一帧数据进行的操作 基本操作是将处理完的数据重新排回帧队列中 这个必须放在类外
//相机50hz最大 20ms一帧的处理速度
void __stdcall FrameDoneCB(tPvFrame * pFrame)
{
	Camera* this_cam = (Camera*)pFrame->Context[0];
	//帧数计数，表示目前接收了多少帧
	this_cam->FrameCount = pFrame->FrameCount;
	if (this_cam->acqThreadState > 0)
	{
		//对每一个进入的图像数据，copy到一个临时buffer中，最多等待20ms
		if (WaitForSingleObject(this_cam->sequenceVacant, 100) == WAIT_OBJECT_0)
		{
			ResetEvent(this_cam->sequenceVacant);													//接收到信号后，重置信号
			memcpy(this_cam->Seq_Buffer, pFrame->ImageBuffer, (pFrame->Width)*(pFrame->Height));	//数据复制给buffer
			SetEvent(this_cam->sequenceBusy);														//copy 数据完毕，通知堆栈线程开始执行为有效信号
		}
		PvCaptureQueueFrame(tPvHandle(pFrame->Context[1]), pFrame, FrameDoneCB);
	}
}

//单线程队列拷贝函数
DWORD WINAPI SeqThread(LPVOID param)
{
	Camera* this_cam = (Camera*)param;
	Mat SaveImage;
	SaveImage.create(this_cam->Height, this_cam->Width, CV_8UC1);
	
	while (this_cam->seqThreadState > 0)
	{
		//在没信号的时候每隔100ms检查一次，有信号立刻接入
		if (WaitForSingleObject(this_cam->sequenceBusy, 100) == WAIT_OBJECT_0)
		{
			ResetEvent(this_cam->sequenceBusy);														//一旦执行，重置信号
			memcpy(SaveImage.data, this_cam->Seq_Buffer, (this_cam->Width)*(this_cam->Height));
			this_cam->frameSequ.push_back(SaveImage);												//将mat类存入一个mat的deque变量中，用于后面存，前面取
			//当swap有效的时候，调用swap方法将数据转给processSequ
			if (WaitForSingleObject(this_cam->swap, 0) == WAIT_OBJECT_0)							//这个就是判断当前processSequ是否为空的信号量
			{
				ResetEvent(this_cam->swap);															//一但开始执行，信号量重置
				this_cam->processSequ.swap(this_cam->frameSequ);									//队列指针交换
				SetEvent(this_cam->swapOver);														//一旦迁移完毕，通知后面可以继续处理
			}
			SetEvent(this_cam->sequenceVacant);														//这个是存入完成后的
		}
	}
	return 0;
}

//多线程处理函数 循环多线程对数据进行处理
DWORD WINAPI ProThread(LPVOID param)
{
	syInfo* t_info1 = (syInfo*)param;			//用于进行线程传入量
	int threadNum = t_info1->num;				//代表当前线程的线程数
	int nextThreadNum = 0;						//用于将线程循环起来，每一个线程找到其对应的下一个线程。
	Camera* this_cam = (Camera*)t_info1->ptr;	//本相机指针
	if (threadNum == THREADNUM - 1)
		nextThreadNum = 0;
	else
		nextThreadNum = threadNum + 1;
	Mat image;									//这个是用来保存获取进入当前线程图像的临时文件
	int saving_circle = 0;
	int frame_circle = 0;
	while (this_cam->proThreadState[threadNum] > 0)
	{		
		int b_err = 0;
		int SavingNum = threadNum + 1 + saving_circle*THREADNUM;//通过循环次数，获取图像序号 这就是用来存储图像的而已
		int FrameNum = threadNum + 1 + frame_circle*THREADNUM;
		if (WaitForSingleObject(this_cam->NextProcess[threadNum], INFINITE) == WAIT_OBJECT_0)
			//用来接收上一个线程的信号，即上一个线程通知下一个线程可以开始对队列中图像进行预处理
		{
			ResetEvent(this_cam->NextProcess[threadNum]);				//重置本线程信号量
			b_err = getImage(this_cam, image, threadNum);//调用获取图像的函数 并且设置下一个线程信号量
			//这里是并行存储
			if (b_err == -1)		//如果线程要停止了，那就彻底停止该线程并退出(break)
			{
				SetEvent(this_cam->NextProcess[nextThreadNum]);					//开启下一个线程，导致所有线程关闭
				break;
			}
			else if (b_err == 0)	//如果因交换失败而拿不到数据，那就等待下一次while(continue)
			{
				SetEvent(this_cam->NextProcess[threadNum]);					//让本线程继续跑
				continue;
			}
				
			else if (b_err == 1)	//一旦之前成功拿到数据
			{				
				SetEvent(this_cam->NextProcess[nextThreadNum]);				//获得图像成功，让位置给下一个线程
				//多线程并行进行图像预处理
				if (this_cam->isSaving)
				{
					char filename[50];
					sprintf(filename, "Frame%05d_%05d.bmp", SavingNum,FrameNum);	//第一个为存图的数量	第二个为获取图的数量
					string filename_all = this_cam->filepath + "\\" + /*this_cam->CameraName + "\\" + */filename;
					imwrite(filename_all, image);
					saving_circle++;
				}
				frame_circle++;
			}
		}
	}
	return 0;
}


// the function can get the image from the deque
int getImage(Camera* this_cam, Mat & image, int threadNum)
{
	//判断当前的线程状态，如果线程已经被关闭
	if (!this_cam->proThreadState[threadNum])
	{
		//就通知下面的线程不要堵塞再等待之后，循环关闭所有线程
		return -1;
	}
	if (!this_cam->processSequ.empty())//判断当前用于处理的堆栈是否为空
	{
		//若不空
		image = this_cam->processSequ.front();			//提取序列最前图像		这里是浅拷贝，但是由于图像被pop了，所以并不影响，对image处理完是否会被处理掉，是否会造成内存泄露？并不影响程序运行
		this_cam->processSequ.pop_front();				//排出已提取图像
		//已成功提取图像 那么可以开启下一个线程接着读入图像
		return 1;
	}
	else
	{
		//若为空
		SetEvent(this_cam->swap);					//开启置换功能
		if (WaitForSingleObject(this_cam->swapOver, 100) == WAIT_OBJECT_0)	//这个为置换成果信号，由于会存在图像采集停止，所以等待100ms
		{
			ResetEvent(this_cam->swapOver);				//重置信号
			if (!this_cam->processSequ.empty())			//判断是否为空，一般不为空 如果为空则停止该线程？
			{
				image = this_cam->processSequ.front();	//提取序列最前图像
				this_cam->processSequ.pop_front();		//排出已提取图像
					//已成功提取图像 那么可以开启下一个线程接着读入图像
				return 1;
			}
			else
			{
				//尽管没有读取图像 但是我们可以跳过这一帧的处理过程，等待下一次
				return 0;
			}
		}
		else
		{
			//没有等待交换成果，默认为没有新的图像进入，不过一般这种情况不会出现 这个是一个用来备用调试的选项
			return 0;
		}
	}
}