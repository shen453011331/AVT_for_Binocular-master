/*
投影仪类 定义文件 对 C API 进行了再封装

这个类实现了从ini文件中加载图像，人为手工控制其投影周期和时长的接口。
不支持变曝光，如果需要变曝光，则需要自己更改文件中的内容。
不支持仅投影一次的模式，具体参照TI官方帖子 https://e2e.ti.com/support/dlp/f/94/t/803934
并且相对于TI 提供的GUI界面，对图案的时间验证进行了一定的补充(过去的似乎是有嫌疑产生BUG)

注意：其中采用的USB交互方式 线程安全性不保证 所以对于一个投影仪控制能且只能使用一个线程。

*/
#pragma once
#include <string>
#include <vector>

//9个参数 表示投影仪中的一张图
//相对于DLPC350_AddToPatLut函数，此结构体多了一个信息frameIndex 这个信息将被存到另一个upload的数据中
typedef struct Pattern
{
	int triger_type;
	int pat_num;
	unsigned int bit_depth;
	int color_code; 
	int frame_index;
	bool need_inv;
	bool need_clear_DMD;
	bool buffer_swap;
	bool trigOutPrev;
}Pattern;

/*
投影仪类 
*/
class Projector
{
private:
	std::string Log;	//需要输出的日志
	std::vector<std::string> token_list;
	
	//ini文件信息
	bool ini_isFrmFlash = false;			//是否从Flash中加载
	bool ini_isVarExpTrigMode = false;		//是否为变曝光模式 否为不变曝光模式
	bool ini_isIntExtTrigMode = false;		//是否是内外部触发 否为VSYNC触发
	bool ini_isRepeat = false;				//是否是重复的
	
	unsigned int ini_SwpflashNum = 0;			//需要交换多少次flash img				//not only read from ini but also send to DLPC
	unsigned int ini_SwpflashLut[64] = { 0 };	//每一次交换flash的时候，转换的Index	//not only read from ini but also send to DLPC
	unsigned int ini_exposure = 0;			//exposure time								
	unsigned int ini_period = 0;			//preiod cycle time							
	std::vector<Pattern> PatSeqLUT_data;	//具体的条纹数据信息
	unsigned int ini_ExtraSwapFlashNum = 0;
	unsigned int ini_ExtraSwapFlashLUT[64] = { 0 };	//多余的没有被使用到的swapflash信息
	
public:
	//基本数据信息
	unsigned int numImgInFlash;									//flash中的图像个数
	unsigned char firmwareTag[33];								//用户固件tag
	char versionStr[255];										//App版本号
	std::string ini_file_path;
	//状态信息 
	bool is_connected;											//电脑是否连接着投影仪
	bool is_standby;											//是否在电源standby中
	bool is_validate = false;
	bool SLmode;												//是否处在SLmode 两种情况 是 SLMODE 否VIDEOMODE
	int trigMode;												//SLMODE 下 要么是 trigmode <= 2为 固定曝光的投影模式 sequence flash模式/video模式 trigmode > 2 变曝光的投影模式 sequence flash模式/video模式
	bool isExtPatDisplayMode;									//是否是从外界获取的图像（还是从FLASH中加载）
	unsigned int action = 3;										//处于何种投影状态。
	unsigned char Red, Green, Blue;								//三路电流 注意 一路为255就是2安培电流,一定不能让其所有都为亮

	//用户需要定义的数据 注意目前只有isRepeat = true 才能使用
	unsigned int exposure_time = 0;			//曝光时间
	unsigned int period_time = 0;			//周期时间
	bool isRepeat = true;					//是否是重复投影
	
	//validate state
	bool ExpOOR, PatNumOOR, TrigOutOverLap, BlkVecMiss, PatPeriodShort;
	unsigned int ini_LutEntriesNum = 0;		//由多少图案构成 
public:
	Projector();	//构造函数	对象内部参量初始化 
	~Projector();	//析构函数

	//发现时钟 不对投影仪做配置 只对投影仪做状态读取 此步后应输出Log信息
	bool Discover_Timer_Call();

	//获取投影仪的当前状态
	bool Get_DLPC_Status();

	//设置投影仪为所需要的falsh pattern模式 并设定好图像序列，并准备投影（已validate）
	/*
	1、设定为固定曝光时间的投影模式
	2、配置光源信息(B255)
	4、将读入的数据转为编码写入投影仪
	5、验证其正确性

	要求 一次play只能投影一次(这个条件在实验之后发现并不可行，硬件存在BUG, 我们需要采用一些其他的手段去让拍摄停下来，例如数相机的图像帧数）
	这里 只能设定整体的投影为循环投影。
	*/
	bool Projector_Init();

	//投影仪控制
	bool Play();
	bool Stop();
	bool Pause();

	bool Config_PlayMode2SL();
	bool Send_PrtSeqCode();
	bool ValidatePatSeq();
	void SoftwareReset();
	void GetSeqStatus();
	bool ReadIni(const char* file_path);	//调用该函数 可以将ini信息读取到类内。读取完成后需要检查图案的数据
	void ApplyIni(std::string token, unsigned int* para, int para_num);

	std::string getLog() 
	{
		std::string rtn_str = Log; 
		Log = ""; 
		return rtn_str;
	}
};

