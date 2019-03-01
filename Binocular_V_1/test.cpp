#pragma once
#include "stdafx.h"


int ApcFunc(frameindex_t picNr, struct fg_apc_data *data)
{
	// For Image Process and Display
     m_pthis->status = picNr;
	 //帧率统计
	if (GetTickCount() > m_pthis->ticks + 1000)
	{
		m_pthis->fps = 1000.0 * (m_pthis->status - m_pthis->oldstatus) / (GetTickCount() - m_pthis->ticks);
		m_pthis->oldstatus = m_pthis->status;
		m_pthis->ticks = GetTickCount();

	}

	void* dma_data_pointer0 = Fg_getImagePtrEx(data->FrameGrabber, picNr, 0, data->mem);
	BYTE* pData = (BYTE*)(dma_data_pointer0);
	if (!m_pthis->b_measure)
	{
		if (WaitForSingleObject(m_pthis->m_DrawEvent1, 0) == WAIT_OBJECT_0)//等待显示完毕，返回有效信号
		{
			ResetEvent(m_pthis->m_DrawEvent1);
			if (m_pthis->Display_Buffer1 != NULL)
				delete[]m_pthis->Display_Buffer1;
			m_pthis->Display_Buffer1 = new unsigned char[(m_pthis->width)*(m_pthis->height)];
			memcpy(m_pthis->Display_Buffer1, pData, (m_pthis->width)*(m_pthis->height));
			SetEvent(m_pthis->m_PutEvent1);//copy 数据完毕，m_PutEvent1为有效信号
		}
	}
	else
	{
		
		if (WaitForSingleObject(m_pthis->sequenceVacant, 100) == WAIT_OBJECT_0)//等待堆栈线程存入堆栈，等待最多100ms
		{
			ResetEvent(m_pthis->sequenceVacant);//接收到信号后，重置信号
			if (m_pthis->Seq_Buffer1 != NULL)
				delete[]m_pthis->Seq_Buffer1;//重置存入堆栈内存
			m_pthis->Seq_Buffer1 = new unsigned char[(m_pthis->width)*(m_pthis->height)];//数据复制给堆栈内存
			memcpy(m_pthis->Seq_Buffer1, pData, (m_pthis->width)*(m_pthis->height));//数据复制给堆栈内存
			SetEvent(m_pthis->sequenceBusy);//copy 数据完毕，通知堆栈线程开始执行为有效信号
		}

	}
	//用于保存出图片实现深度学习检测
	if (m_pthis->status % 30 == 0)
	{
		if (WaitForSingleObject(m_pthis->m_DrawEvent2, 0) == WAIT_OBJECT_0)//等待显示完毕，返回有效信号
		{
			ResetEvent(m_pthis->m_DrawEvent2);
			if (m_pthis->Display_Buffer2 != NULL)
				delete[]m_pthis->Display_Buffer2;
			m_pthis->Display_Buffer2 = new unsigned char[(m_pthis->width)*(m_pthis->height)];
			memcpy(m_pthis->Display_Buffer2, pData, (m_pthis->width)*(m_pthis->height));
			SetEvent(m_pthis->m_PutEvent2);//copy 数据完毕，m_PutEvent1为有效信号
		}
	}
	return 0;
}

DWORD WINAPI SeqThread(LPVOID param)
{
	//这是一个线程
	CSiso_APC_CXP_08Dlg* m_pthis = (CSiso_APC_CXP_08Dlg*)param;
	Mat SaveImage;
	IplImage* pImage = cvCreateImage(cvSize(m_pthis->width, m_pthis->height), IPL_DEPTH_8U, 1);

	//给定一个线程的开关状态，用于控制线程执行和结束
	while (m_pthis->seqThreState > 0)
	{
		if (WaitForSingleObject(m_pthis->sequenceBusy, 100) == WAIT_OBJECT_0)//等待回调中的copy完毕，就返回有效信号，等待100，其实由于循环相当于一直在等待，不过可以帮助其迅速关闭
		{
			ResetEvent(m_pthis->sequenceBusy);//一旦执行，重置信号
			
			//把imageData做成Mat并压入frameSequ
			memcpy(pImage->imageData, m_pthis->Seq_Buffer1, (m_pthis->width)*(m_pthis->height));
			SaveImage = cvarrToMat(pImage);
			m_pthis->frameSequ.push_back(SaveImage);
			//这里拷贝的逻辑是，一旦processSequ中的数据为空，则将frameSequ当前存入数据整体迁移过去
			if (WaitForSingleObject(m_pthis->swap, 0) == WAIT_OBJECT_0)//这个就是判断当前processSequ是否为空的信号量
			{
				ResetEvent(m_pthis->swap);//一但开始执行，信号量重置
				double t1, t2;
				t1 = getTickCount();//这个是用来取进行观察整体迁移所耗时间，实际基本为1ms左右
				m_pthis->processSequ.swap(m_pthis->frameSequ);//deque迁移
				t2 = (t1 - getTickCount()) / getTickFrequency();
				SetEvent(m_pthis->swapOver);//一旦迁移完毕，通知后面可以继续处理
			}
			SetEvent(m_pthis->sequenceVacant);//这个是存入完成后的
		}
	}
	cvReleaseImage(&pImage);//这里需要将iplimage内存空间释放，要不然会出现内存累计的现象。
	return 0;
}

DWORD WINAPI ProThread(LPVOID param)//处理线程，实际一次性激活多个该线程，用于对内存进行多线程处理
{
	int count = 0;//好像没用到，标记！！！！
	double sumTime = 0;//用于计时
	bool b_err = 0;//可能是用于进行判断当前线程里是否有数据，稍后更新
	syInfo* t_info1 = (syInfo*)param;//用于进行线程传入量
	int threadNum = t_info1->num;//代表当前线程的线程数
	int nextThreadNum = 0;//用于将线程循环起来，每一个线程找到其对应的下一个线程。
	if (threadNum == THREADNUM - 1)
	{
		nextThreadNum == 0;
	}
	else
	{
		nextThreadNum = threadNum + 1;
	}
	int num_circle = 0;//这个用来表示当前线程的循环次数，用于计算进入当前线程的图像文件序号
	Mat image;//这个是用来保存获取进入当前线程图像的临时文件


	CSiso_APC_CXP_08Dlg* m_pthis = (CSiso_APC_CXP_08Dlg*)t_info1->ptr;//获取整个dlg类的指针
	while (m_pthis->threadState[threadNum] > 0)//多线程load图片到对应的线程
		//这个线程状态，为控制线程是否继续无限循环的作用
	{
		double t1 = getTickCount();
		int frameNum = threadNum + 1 + num_circle*THREADNUM;//通过循环次数，获取图像序号
		if (WaitForSingleObject(m_pthis->NextProcess[threadNum], INFINITE) == WAIT_OBJECT_0)
			//用来接收上一个线程的信号，即上一个线程通知下一个线程可以开始对队列中图像进行预处理
		{
			b_err = 0;
			ResetEvent(m_pthis->NextProcess[threadNum]);//重置信号量
			if (!m_pthis->threadState[THREADNUM])//判断当前的线程状态，如果线程已经被关闭，

			{
				SetEvent(m_pthis->NextProcess[THREADNUM]);//就通知下面的线程不要堵塞再等待之后，再自行关闭。
				break;
			}
			WaitForSingleObject(m_pthis->g_mtx_swap, INFINITE);//这个市对于堆栈线程的操作锁，保证
			//同时只有一个线程在访问该堆栈数据
			if (!m_pthis->processSequ.empty())//判断当前用于处理的堆栈是否为空
			{
				//若不空
				image = m_pthis->processSequ.front();//提取序列最前图像
				m_pthis->processSequ.pop_front();//排出已提取图像
				ReleaseMutex(m_pthis->g_mtx_swap);//释放锁定信号量

			}
			else
			{
				//若为空
				SetEvent(m_pthis->swap);//置换
				ReleaseMutex(m_pthis->g_mtx_swap);//锁定堆栈

				if (WaitForSingleObject(m_pthis->swapOver, 100) == WAIT_OBJECT_0)
					//这个为置换成果信号，由于会存在图像采集停止，所以等待100ms
				{
					ResetEvent(m_pthis->swapOver);//重置信号
					WaitForSingleObject(m_pthis->g_mtx_swap, INFINITE);//锁定
					if (!m_pthis->processSequ.empty())//判断是否为空，一般不为空
					{
						image = m_pthis->processSequ.front();//提取数据
						m_pthis->processSequ.pop_front();

					}
					ReleaseMutex(m_pthis->g_mtx_swap);
				}
				else
				{
					//没有等待交换成果，默认为没有新的图像进入，不过一般这种情况不会出现
					//这个是一个用来备用调试的选项
					WaitForSingleObject(m_pthis->g_mtx_swap, INFINITE);

					ReleaseMutex(m_pthis->g_mtx_swap);
					SetEvent(m_pthis->NextProcess[nextThreadNum]);
					b_err = 1;
				}
			}
		}
	}
}