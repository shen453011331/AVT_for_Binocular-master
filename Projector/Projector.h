/*
ͶӰ���� �����ļ� �� C API �������ٷ�װ

�����ʵ���˴�ini�ļ��м���ͼ����Ϊ�ֹ�������ͶӰ���ں�ʱ���Ľӿڡ�
��֧�ֱ��ع⣬�����Ҫ���ع⣬����Ҫ�Լ������ļ��е����ݡ�
��֧�ֽ�ͶӰһ�ε�ģʽ���������TI�ٷ����� https://e2e.ti.com/support/dlp/f/94/t/803934
ע�� ���в��õ�USB������ʽ �̰߳�ȫ�Բ���֤ ����ͶӰ�ǿ���ֻ��ʹ��һ���̡߳�
���������TI �ṩ��GUI���棬��ͼ����ʱ����֤������һ���Ĳ���(��ȥ���ƺ��������ɲ���BUG)
*/
#pragma once
#include <string>
#include <vector>

//9������ ��ʾͶӰ���е�һ��ͼ
//�����DLPC350_AddToPatLut�������˽ṹ�����һ����ϢframeIndex �����Ϣ�����浽��һ��upload��������
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
ͶӰ���� 
*/
class Projector
{
private:
	std::string Log;	//��Ҫ�������־
	std::vector<std::string> token_list;
	
	//ini�ļ���Ϣ
	bool ini_isFrmFlash = false;			//�Ƿ��Flash�м���
	bool ini_isVarExpTrigMode = false;		//�Ƿ�Ϊ���ع�ģʽ ��Ϊ�����ع�ģʽ
	bool ini_isIntExtTrigMode = false;		//�Ƿ������ⲿ���� ��ΪVSYNC����
	bool ini_isRepeat = false;				//�Ƿ����ظ���
	unsigned int ini_LutEntriesNum = 0;		//�ɶ���ͼ������ 	
	unsigned int ini_SwpflashNum = 0;			//��Ҫ�������ٴ�flash img				//not only read from ini but also send to DLPC
	unsigned int ini_SwpflashLut[64] = { 0 };	//ÿһ�ν���flash��ʱ��ת����Index	//not only read from ini but also send to DLPC
	unsigned int ini_exposure = 0;			//exposure time								
	unsigned int ini_period = 0;			//preiod cycle time							
	std::vector<Pattern> PatSeqLUT_data;	//���������������Ϣ
	unsigned int ini_ExtraSwapFlashNum = 0;
	unsigned int ini_ExtraSwapFlashLUT[64] = { 0 };	//�����û�б�ʹ�õ���swapflash��Ϣ

public:
	//����������Ϣ
	unsigned int numImgInFlash;									//flash�е�ͼ�����
	unsigned char firmwareTag[33];								//�û��̼�tag
	char versionStr[255];										//App�汾��

	//״̬��Ϣ 
	bool is_connected;											//�����Ƿ�������ͶӰ��
	bool is_standby;											//�Ƿ��ڵ�Դstandby��
	bool SLmode;												//�Ƿ���SLmode ������� �� SLMODE ��VIDEOMODE
	int trigMode;												//SLMODE �� Ҫô�� trigmode <= 2Ϊ �̶��ع��ͶӰģʽ sequence flashģʽ/videoģʽ trigmode > 2 ���ع��ͶӰģʽ sequence flashģʽ/videoģʽ
	bool isExtPatDisplayMode;									//�Ƿ��Ǵ�����ȡ��ͼ�񣨻��Ǵ�FLASH�м��أ�
	unsigned int action;										//���ں���ͶӰ״̬��
	unsigned char Red, Green, Blue;								//��·���� ע�� һ·Ϊ255����2�������,һ�������������ж�Ϊ��

	//�û���Ҫ��������� ע��Ŀǰֻ��isRepeat = true ����ʹ��
	unsigned int exposure_time = 0;			//�ع�ʱ��
	unsigned int period_time = 0;			//����ʱ��
	bool isRepeat = true;					//�Ƿ����ظ�ͶӰ
	
	//validate state
	bool ExpOOR, PatNumOOR, TrigOutOverLap, BlkVecMiss, PatPeriodShort;

public:
	Projector();	//���캯��	�����ڲ�������ʼ�� 
	~Projector();	//��������

	//����ʱ�� ����ͶӰ�������� ֻ��ͶӰ����״̬��ȡ �˲���Ӧ���Log��Ϣ
	bool Discover_Timer_Call();

	//��ȡͶӰ�ǵĵ�ǰ״̬
	bool Get_DLPC_Status();

	//����ͶӰ��Ϊ����Ҫ��falsh patternģʽ ���趨��ͼ�����У���׼��ͶӰ����validate��
	/*
	1���趨Ϊ�̶��ع�ʱ���ͶӰģʽ
	2�����ù�Դ��Ϣ(B255)
	4�������������תΪ����д��ͶӰ��
	5����֤����ȷ��

	Ҫ�� һ��playֻ��ͶӰһ��(���������ʵ��֮���ֲ������У�Ӳ������BUG, ������Ҫ����һЩ�������ֶ�ȥ������ͣ�����������������ͼ��֡����
	���� ֻ���趨�����ͶӰΪѭ��ͶӰ��
	*/
	bool Projector_Init(std::string ini_filename);

	//ͶӰ�ǿ���
	bool Play();
	bool Stop();
	bool Pause();

	bool Config_PlayMode2SL();
	bool Send_PrtSeqCode();
	bool ValidatePatSeq();
	void SoftwareReset();

	void ReadIni(const char* file_path);	//���øú��� ���Խ�ini��Ϣ��ȡ�����ڡ���ȡ��ɺ���Ҫ���ͼ��������
	void ApplyIni(std::string token, unsigned int* para, int para_num);

	std::string getLog() 
	{
		std::string rtn_str = Log; 
		Log = ""; 
		return rtn_str;
	}
};

