#include "stdafx.h"
#include "Projector.h"

//ͶӰ�ǲ�����
#include "dlpc350_api.h"
#include "dlpc350_common.h"
#include "dlpc350_usb.h"
#include "dlpc350_version.h"
#include "dlpc350_firmware.h"
#include "dlpc350_BMPParser.h"
#include "dlpc350_error.h"

//��׼std
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
using namespace std;

Projector::Projector()
{
	numImgInFlash = 0;
	firmwareTag[0] = '\0';
	versionStr[0] = '\0';
	is_standby = false;
	is_connected = true;
	SLmode = false;
	action = 3;
	period_time = 0;
	exposure_time = 0;
	isRepeat = true;
	Log = "";
	//��ʼ��token_list
	token_list.clear();
	token_list.push_back( "DEFAULT.DISPMODE" );
	token_list.push_back( "DEFAULT.SHORT_FLIP" );
	token_list.push_back( "DEFAULT.LONG_FLIP" );
	token_list.push_back( "DEFAULT.TRIG_OUT_1.POL" );
	token_list.push_back( "DEFAULT.TRIG_OUT_1.RDELAY" );
	token_list.push_back( "DEFAULT.TRIG_OUT_1.FDELAY" );
	token_list.push_back( "DEFAULT.TRIG_OUT_2.POL" );
	token_list.push_back( "DEFAULT.TRIG_OUT_2.WIDTH" );
	token_list.push_back( "DEFAULT.TRIG_IN_1.DELAY" );
	token_list.push_back( "DEFAULT.TRIG_IN_2.POL" );
	token_list.push_back( "DEFAULT.RED_STROBE.RDELAY" );
	token_list.push_back( "DEFAULT.RED_STROBE.FDELAY" );
	token_list.push_back( "DEFAULT.GRN_STROBE.RDELAY" );
	token_list.push_back( "DEFAULT.GRN_STROBE.FDELAY" );
	token_list.push_back( "DEFAULT.BLU_STROBE.RDELAY" );
	token_list.push_back( "DEFAULT.BLU_STROBE.FDELAY" );
	token_list.push_back( "DEFAULT.INVERTDATA" );
	token_list.push_back( "DEFAULT.LEDCURRENT_RED" );
	token_list.push_back( "DEFAULT.LEDCURRENT_GRN" );
	token_list.push_back( "DEFAULT.LEDCURRENT_BLU" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.PAT_EXPOSURE" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.PAT_PERIOD" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.PAT_MODE" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.TRIG_MODE" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.PAT_REPEAT" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.NUM_PATTERNS" );
	token_list.push_back( "DEFAULT.PATTERNCONFIG.NUM_SPLASH" );
	token_list.push_back( "DEFAULT.SPLASHLUT" );
	token_list.push_back( "DEFAULT.SEQPATLUT" );
	token_list.push_back( "DEFAULT.LED_ENABLE_MAN_MODE" );
	token_list.push_back( "DEFAULT.MAN_ENABLE_RED_LED" );
	token_list.push_back( "DEFAULT.MAN_ENABLE_GRN_LED" );
	token_list.push_back( "DEFAULT.MAN_ENABLE_BLU_LED" );
	token_list.push_back( "DEFAULT.PORTCONFIG.PORT" );
	token_list.push_back( "DEFAULT.PORTCONFIG.BPP" );
	token_list.push_back( "DEFAULT.PORTCONFIG.PIX_FMT" );
	token_list.push_back( "DEFAULT.PORTCONFIG.PORT_CLK" );
	token_list.push_back( "DEFAULT.PORTCONFIG.ABC_MUX" );
	token_list.push_back( "DEFAULT.PORTCONFIG.PIX_MODE" );
	token_list.push_back( "DEFAULT.PORTCONFIG.SWAP_POL" );
	token_list.push_back( "DEFAULT.PORTCONFIG.FLD_SEL" );
	//��ʼ��USBͨѶ
	DLPC350_USB_Init();
}

Projector::~Projector()
{
	//������ʱ����Ҫ��ͶӰ��ת��power_standbyģʽ
	DLPC350_SetPowerMode(true);//��ͶӰ��תΪpower_stand_byģʽ�����رճ���
}

//����ʱ�� �����Ƿ�������ͶӰ��
bool Projector::Discover_Timer_Call()
{
	//һ��ʼ�ʹ�������״̬��
	if (DLPC350_USB_IsConnected())
	{
		unsigned char HWStatus, SysStatus, MainStatus;
		//��ȡ״̬��Ϣ
		if (DLPC350_GetStatus(&HWStatus, &SysStatus, &MainStatus) != 0)
		{
			//have problem
			Log = "Error: projector is connected but can not get info.\r\n";
			is_connected = false;
			return false;
		}
		else
		{
			is_connected = true;
			return true;
		}
	}
	//��û�д�������״̬ ��������
	else
	{
		Log += "Trying to connect projector...";
		if (DLPC350_USB_Open() == 0) //��ͶӰ��USB����
		{
			if (DLPC350_USB_IsConnected())
			{
				Log += "Projector connneted.\r\n ";
				//��ȡ״̬��Ϣ
				if (Get_DLPC_Status())
				{

				}
				else
				{

				}

				//��ȡ�汾��
				unsigned int API_ver, App_ver, SWConfig_ver, SeqConfig_ver; //��������İ汾��
				if (DLPC350_GetVersion(&App_ver, &API_ver, &SWConfig_ver, &SeqConfig_ver) == 0)
				{
					sprintf(versionStr, "%d.%d.%d", (App_ver >> 24), ((App_ver << 8) >> 24), ((App_ver << 16) >> 16));
				}
				else
				{
					sprintf(versionStr, "can not read.\r\n");
				}

				//��ȡ�̼��ı�ǩ tag
				if (DLPC350_GetFirmwareTagInfo(firmwareTag) != 0)
				{
					sprintf((char*)firmwareTag, "can not read.\r\n");
				}

				//��ȡͶӰ����flashͼ��ĸ���
				if (DLPC350_GetNumImagesInFlash(&numImgInFlash) == 0)
				{
					Log += "Flash: " + to_string(numImgInFlash) + " pictures.\r\n";
				}
				else
				{
					Log += "Warinning: Can not read the pic number in flash.\r\n";
				}

				//��ȡ��Դ״̬
				DLPC350_GetPowerMode(&is_standby);

				//��ȡͶӰ�ǵ���ʾ״̬
				if ((DLPC350_GetMode(&SLmode) == 0) && (is_standby == false))
				{
					//�����mode ����һ������videoMode
					if (SLmode)
					{
						if (DLPC350_GetPatternTriggerMode(&trigMode) == 0)
						{
							//���trigMode <= 2��1��2����ôӦ����SLMode(������Ҫ��) ���ֿ��� һ���Ǵ�flash�ж�ȡ һ���Ǵ�video�˿ڶ�ȡ
							if (trigMode <= 2)
							{
								Log += "Disp Mode: fixed exposure pattern ";
								if (DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
									if (isExtPatDisplayMode) //if������ͼ
										//��video�˻�ȡparten����
										Log += "from Video\r\n";
									else
										//��flash�˶�ȡparten����
										Log += "from Flash\r\n";
								else
									Log += "\r\nWarnning: Can not read the Display Mode.(fixed exposure)\r\n";
							}
							//�������ع���ʽ
							else
							{
								Log += "Disp Mode: variable exposure pattern,";
								if (DLPC350_GetPatternDisplayMode(&isExtPatDisplayMode) == 0)
									if (isExtPatDisplayMode) //if set to external DVI/FPD port
										Log += "from Video\r\n";
									else					
										Log += "from Flash\r\n";
								else
									Log += "\r\nWarnning: Can not read the Display Mode.(fixed exposure)\r\n";
							}
						}
					}
					//���SLmode��true ��ô��ֻ�ܸ���videoMode
					else
					{
						Log += "Disp Mode: Video\r\n";
					}
				}
				else if(is_standby)
				{
					Log += "Disp Mode: power standby.\r\n";
				}
				else
				{
					Log += "Warnning: Disp Mode getting error.\r\n";
				}

				is_connected = true;
				return true;
			}
			else
			{
				Log += "Connected failed, USB connect Error.\r\n";
				is_connected = false;
				return false;
			}
		}
		else
		{
			Log += "Projector connected failed.\r\n";
			is_connected = false;
			return false;
		}
	}
}

//��ȡͶӰ�ǵĻ���״̬
bool Projector::Get_DLPC_Status()
{
	unsigned char HWStatus, SysStatus, MainStatus;
	if (DLPC350_GetStatus(&HWStatus, &SysStatus, &MainStatus) == 0)
	{
		if ((HWStatus&BIT0) == BIT0)
			Log += "HWState:Init Done.\r\n";
		if ((HWStatus&BIT3) == BIT3)
			Log += "HWState:Forced Swap.\r\n";
		if ((HWStatus&BIT6) == BIT6)
			Log += "HWState:Sequence Abort.\r\n";
		if ((HWStatus&BIT2) == BIT2)
			Log += "HWState:DRC Error.\r\n";
		if ((HWStatus&BIT7) == BIT7)
			Log += "HWState:Sequence Error.\r\n";

		if ((MainStatus&BIT0) == BIT0)
			Log += "MainState:DMD parked.\r\n";
		if ((MainStatus&BIT1) == BIT1)
			Log += "MainState:Sequence Running.\r\n";
		if ((MainStatus&BIT2) == BIT2)
			Log += "MainState:Buffer frozen.\r\n";

		return true;
	}
	else
	{
		Log += "Warnning: Cant get the state of DLPC.\r\n";
		return false;
		//û�гɹ������״̬��Ϣ
	}
}

//��ʼ��ͶӰ��
bool Projector::Projector_Init()
{
	//תΪpatternģʽ
	if (Config_PlayMode2SL())
	{
		Log += "Have Changed Mode to Pattern Sequence.\r\n";
	}
	else
	{
		return false;
	}

	//�ı��ǿ ֻҪ���� ��ֵԽ�� ��ǿԽ�� ֻ������ɫ�� �Ұ������ǿ�ȵ������
	DLPC350_SetLedEnables(0, 0, 0, 1);	//��һλ��Ϊ0������֮ǰ��ͶӰ����ʲô�� �˴���������
	DLPC350_SetLedCurrents(255, 255, 0);
	//Read ini
	if(ReadIni(ini_file_path.c_str()) == false)
		return false;
	//��֤�Ƿ��б���
	if (PatSeqLUT_data.size() == 0)
	{
		Log += "Warnning: No Pattern data aviliable.\r\n";
		return false;
	}
	//Send to DLPC the data
	is_validate = false;
	if (Send_PrtSeqCode())
	{
		Log += "Send Pattern info over.\r\n";
	}
	else
	{
		return false;
	}
	//valid the sequence
	if (ValidatePatSeq())
	{
		is_validate = true;
		Log += "Pattern Sequence Init Done.\r\n";
		return true;
	}
	else
	{
		return false;
	}
}

//��ͶӰ�Ǵ�������״̬�趨��patternģʽ
bool Projector::Config_PlayMode2SL()
{
	//�ж��Ƿ���power_standbyģʽ 
	DLPC350_GetPowerMode(&is_standby);
	if (is_standby)	//�����standby ����
	{
		DLPC350_SetPowerMode(0);
		this_thread::sleep_for(chrono::seconds(2));
		int i = 0;
		while (1)
		{
			DLPC350_GetPowerMode(&is_standby);
			if (!is_standby)
				break;
			this_thread::sleep_for(chrono::microseconds(200));
			if (i++ > 10)
			{
				Log += "Error: Reboot error, cannot change the power mode. Need Reset...\r\n";
				return false;
			}
		}
	}

	//�ж�ͶӰ��Ŀǰ��ͶӰģʽ
	if (DLPC350_GetMode(&SLmode) == 0)
	{
		if (SLmode)
		{
			if (Stop())
			{
				Log += "Projector already in SLmode.\r\n ";
				return true;//ͣ����
			}
				
			else
				return false;
		}
		else
		{
			int i = 0;
			//ת��pattern sequenceģʽ��
			DLPC350_SetMode(true);
			this_thread::sleep_for(chrono::microseconds(100));
			while (1)
			{
				DLPC350_GetMode(&SLmode);
				if (SLmode)
				{
					Log += "Projector changed to SLmode.\r\n";
					if (Stop())
						return true;//ͣ����
					else
						return false;
				}
				this_thread::sleep_for(chrono::microseconds(100));
				if (i++ > 5)
				{
					Log += "Error: Set DLPC Mode Error (SLMode).\r\n";
					return false;
				}
			}
		}
	}
	else
	{
		Log += "Error: Get Mode error.\r\n";
		return false;
	}
}

//����ini�ļ��к�ͼ�������йص���Ϣ ����������������ϵ���Ϣ������ͶӰ�Ƿ���Ϣ
bool Projector::ReadIni(const char* file_path)
{
	Log += "Loading from " + string(file_path)+"...\r\n";
	ifstream file_ini_in(file_path);
	if (!file_ini_in.is_open())
	{
		Log += "Error: Error in Open ini file.\r\n";
		return false;
	}
	string line_data;
	char cFirsttoken[128];
	uint32 iniParams[1824 * 3];
	int numIniParams;
	while (getline(file_ini_in, line_data))
	{
		char* line_data_c = new char[line_data.length()+1];//��ȡ�ַ���������
		unsigned int length = line_data.copy(line_data_c, line_data.length(), 0);	//�����Ƿ���/0����� ���ص��׿����˶����ַ�
		if (DLPC350_Frmw_ParseIniLines(line_data_c))
			continue;
		DLPC350_Frmw_GetCurrentIniLineParam(&cFirsttoken[0], &iniParams[0], &numIniParams);//���������ַ�ת��Ϊʵ����Ҫ�Ķ��� ÿһ������������2��16λ���������
		string token(cFirsttoken);
		ApplyIni(token, iniParams, numIniParams);
	}
	file_ini_in.close();
	return true;
}

//��ini�е����ݶ��������洢�����У�Ҳ��������ڸ����������
void Projector::ApplyIni(string token, unsigned int*params, int numParams)
{
	int token_index = -1;
	int i = 0;
	unsigned int ini_SwpflashIndex = 0;
	int frameIndex;

	for (i = 0;i<token_list.size();i++)
	{
		if (!token.compare(token_list[i]))
		{
			token_index = i;
			break;
		}
	}
	char dispStr[255];
	switch (token_index)
	{
		/*display config*/
		//case 0:				//DEFAULT.DISPMODE
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.DISPMODE");
		//	if (params[0])
		//		//ui->Radiobutton_SLmode->click();
		//		ui->radioButton_SLMode->setChecked(true);
		//	else
		//		ui->radioButton_VideoMode->setChecked(true);
		//	break;
		//case 1:				//DEFAULT.SHORT_FLIP
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.SHORT_FLIP");
		//	if (params[0])
		//		ui->checkBox_shortAxisFlip->setChecked(true);
		//	else
		//		ui->checkBox_shortAxisFlip->setChecked(false);
		//	break;
		//case 2:				//DEFAULT.LONG_FLIP
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LONG_FLIP");
		//	if (params[0])
		//		ui->checkBox_longAxisFlip->setChecked(true);
		//	else
		//		ui->checkBox_longAxisFlip->setChecked(false);
		//	break;

		/*triger setting*/
		//case 3:				//DEFAULT.TRIG_OUT_1.POL
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_1.POL");
		//	if (params[0])
		//		ui->checkBox_InvertTrig1Out->setChecked(true);
		//	else
		//		ui->checkBox_InvertTrig1Out->setChecked(false);
		//	break;
		//case 4:				//DEFAULT.TRIG_OUT_1.RDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_1.RDELAY");
		//	ui->spinBox_Trig1OutRDly->setValue(params[0]);
		//	break;
		//case 5:				//DEFAULT.TRIG_OUT_1.FDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.DISPMODE\r\n");
		//	ui->spinBox_Trig1OutFDly->setValue(params[0]);
		//	break;
		//case 6:				//DEFAULT.TRIG_OUT_2.POL
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_1.FDELAY");
		//	if (params[0])
		//		ui->checkBox_InvertTrig2Out->setChecked(true);
		//	else
		//		ui->checkBox_InvertTrig2Out->setChecked(false);
		//	break;
		//case 7:				//DEFAULT.TRIG_OUT_2.WIDTH
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_OUT_2.WIDTH");
		//	ui->spinBox_Trig2OutRDly->setValue(params[0]);
		//	break;
		//case 8:				//DEFAULT.TRIG_IN_1.DELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_IN_1.DELAY");
		//	ui->spinBox_TrigIn1->setValue(params[0]);
		//	break;
		//case 9:				//DEFAULT.TRIG_IN_2.POL
		//	if ((numParams > 1) && (params[0] <= 1))
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.TRIG_IN_2.POL");
		//	ui->comboBox_TrigIn2Pol->setCurrentIndex(params[0]);
		//	break;

		/*LED config*/

		//case 10:				//DEFAULT.RED_STROBE.RDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.RED_STROBE.RDELAY");
		//	ui->spinBox_LedDlyCtrlRedREdgeDly->setValue(params[0]);
		//	break;
		//case 11:				//DEFAULT.RED_STROBE.FDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.RED_STROBE.FDELAY");
		//	ui->spinBox_LedDlyCtrlRedFEdgeDly->setValue(params[0]);
		//	break;
		//case 12:				//DEFAULT.GRN_STROBE.RDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.GRN_STROBE.RDELAY");
		//	ui->spinBox_LedDlyCtrlGreenREdgeDly->setValue(params[0]);
		//	break;
		//case 13:				//DEFAULT.GRN_STROBE.FDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.GRN_STROBE.FDELAY");
		//	ui->spinBox_LedDlyCtrlGreenFEdgeDly->setValue(params[0]);
		//	break;
		//case 14:				//DEFAULT.BLU_STROBE.RDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.BLU_STROBE.RDELAY");
		//	ui->spinBox_LedDlyCtrlBlueREdgeDly->setValue(params[0]);
		//	break;
		//case 15:				//DEFAULT.BLU_STROBE.FDELAY
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.BLU_STROBE.FDELAY");
		//	ui->spinBox_LedDlyCtrlBlueFEdgeDly->setValue(params[0]);
		//	break;
		//case 16:				//DEFAULT.INVERTDATA
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.INVERTDATA");
		//	if (params[0])
		//		ui->checkBox_PatSeqCtrlGlobalDataInvert->setChecked(true);
		//	else
		//		ui->checkBox_PatSeqCtrlGlobalDataInvert->setChecked(false);
		//	break;
		//case 17:				//DEFAULT.LEDCURRENT_RED
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LEDCURRENT_RED");
		//	ui->lineEdit_RedLEDCurrent->setText(QString::number(255 - params[0]));
		//	break;
		//case 18:				//DEFAULT.LEDCURRENT_GRN
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LEDCURRENT_GRN");
		//	ui->lineEdit_GreenLEDCurrent->setText(QString::number(255 - params[0]));
		//	break;
		//case 19:				//DEFAULT.LEDCURRENT_BLU
		//	if (numParams > 1)
		//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LEDCURRENT_BLU");
		//	ui->lineEdit_BlueLEDCurrent->setText(QString::number(255 - params[0]));
		//	break;

		/*pattern config*/
	case 20:				//DEFAULT.PATTERNCONFIG.PAT_EXPOSURE
	{
		if (numParams > 1)
			Log += ("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_EXPOSURE.\r\n");
		ini_exposure = params[0];

		break;
	}
	case 21:				//DEFAULT.PATTERNCONFIG.PAT_PERIOD
	{
		if (numParams > 1)
			Log += ("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_PERIOD.\r\n");
		ini_period = params[0];
		break;

	}
	case 22:				//DEFAULT.PATTERNCONFIG.PAT_MODE
	{
		if (numParams > 1)
			Log += ("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_MODE.\r\n");
		if (params[0] == 0x3)	//
		{
			ini_isFrmFlash = 1;
		}
		else if (params[0] == 0x0)
		{
			ini_isFrmFlash = 0;
		}
		else
			Log += ("Wrong value as argument for DEFAULT.PATTERNCONFIG.PAT_MODE.\r\n");
		break;
	}

	case 23:				//DEFAULT.PATTERNCONFIG.TRIG_MODE
	{
		ini_isVarExpTrigMode = false;
		if (numParams > 1 && params[0] <= 4)
			Log += ("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.TRIG_MODE.\r\n");

		if (params[0]) //0 1 2
			ini_isIntExtTrigMode = true;
		else
			ini_isIntExtTrigMode = false;

		if (params[0] > 2 && params[0] <= 4)
		{
			ini_isVarExpTrigMode = true;
			if (params[0] == 3)
				ini_isIntExtTrigMode = true;
			else
				ini_isIntExtTrigMode = false;
		} //3 4
		break;
	}

	case 24:				//DEFAULT.PATTERNCONFIG.PAT_REPEAT
		if (numParams > 1)
			Log +=("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.PAT_REPEAT.\r\n");
		if (params[0])
			ini_isRepeat = true;
		else
			ini_isRepeat = false;
		break;

	case 25:                //DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES
		if (numParams > 1)
			Log +=("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.NUM_LUT_ENTRIES.\r\n");
		ini_LutEntriesNum = params[0]+1;
		break;
	//wrong 26
	case 27:                //DEFAULT.PATTERNCONFIG.NUM_SPLASH
		if (numParams > 1)
		{
			Log += ("Wrong number of parameters in the chosen .ini file for DEFAULT.PATTERNCONFIG.NUM_SPLASH.\r\n");
		}
			
		ini_SwpflashNum = params[0]+1;
		if (ini_SwpflashNum >= 64)
			Log += "Warnning: Swap flash number larger than 64, Only write first 64 paras.\r\n";
		break;

	case 28:				//DEFAULT.SPLASHLUT
		if (ini_SwpflashNum != numParams)
			Log +=("Number of Splash Lut entries not matching with actual Lut size.\r\n");
		for (int i = 0; i < numParams; i++)
			ini_SwpflashLut[i] = params[i];
		break;

	case 29:				//DEFAULT.SEQPATLUT
	{
		/* decode the pattern info from inifile*/
		if (ini_isVarExpTrigMode == false)
		{
			if (ini_LutEntriesNum != numParams)
				Log +=("Number of Splash Lut entries not matching with actual Lut size.\r\n");
			PatSeqLUT_data.clear();

			unsigned char patNum, bitDepth, trigger_type, maxPatNum, ledSelect;
			bool invertPat, insertBlack, bufSwap, trigOutPrev;
			for (int i = 0; i < numParams; i++)
			{
				trigger_type = params[i] & 0x3;
				//InsertTriggerItem(trigger_type);

				patNum = (params[i] >> 2) & 0x3F;	//pattern_index(exp: 1 bit, this value can changed from 0 to 23 (also 24 can be, we dont consider it))
				bitDepth = (params[i] >> 8) & 0xF;	//bit deepth

				if (bitDepth < 1 || bitDepth > 8)
				{
					sprintf(dispStr, "Invalid bit-depth in PAT LUT entry%d \r\n", i);
					Log +=(dispStr);
					continue;
				}
				if (bitDepth == 1)
					maxPatNum = 23;	 //24-1 (if==24 original is playing the black or the white for all. We dont consider it)
				else if (bitDepth == 5)
					maxPatNum = 3;
				else if (bitDepth == 7)
					maxPatNum = 2;
				else
					maxPatNum = 24 / bitDepth - 1;
				if (patNum > maxPatNum)
				{
					sprintf(dispStr, "Invalid pattern-number in PAT LUT entry%d \r\n", i);
					Log +=(dispStr);
					continue;
				}
				ledSelect = (params[i] >> 12) & 0x7;	//Led choice
				if ((params[i] & 0x00010000))			//need for invert
					invertPat = true;
				else
					invertPat = false;
				if ((params[i] & 0x00020000))			//need DMD clear
					insertBlack = true;
				else
					insertBlack = false;

				if ((params[i] & 0x00040000) || (!i))	//need swap flash index
				{
					if (ini_SwpflashIndex >= ini_SwpflashNum)
					{
						Log +=("Bad .ini! SplashLUT entries do not match the number of buffer swaps in PAT LUT.\r\n");
						continue;
					}
					else
					{
						bufSwap = true;
						frameIndex = ini_SwpflashLut[ini_SwpflashIndex++];
					}
				}
				else
					bufSwap = false;

				//�Ƿ���Ҫtrioutprev
				if ((params[i] & 0x00080000))
					trigOutPrev = true;
				else
					trigOutPrev = false;

				/*
				logic:
				if patNum == 24 
					append a black in all sequence
				else
					append a normal in all sequence
				*/
				//append to vector
				Pattern pattern = { trigger_type, patNum,bitDepth,ledSelect,frameIndex,invertPat,insertBlack,bufSwap,trigOutPrev };
				PatSeqLUT_data.push_back(pattern);
			}
		}
		else
		{
			Log += "varyExp is Not support yet...\r\n";
			break;
		}
		/*
		else 
		{
			////In variable exposure the number of LUT size is 3x 32bit word per pattern therefore +2 for
			////each pattern exposure and period
			//if ((ini_LutEntriesNum * 3) != numParams)
			//	Log +=("Number of Splash Lut entries not matching with actual Lut size");

			//emit on_pushButton_VarExpPatSeqClearLUTFrmGUI_clicked();

			//unsigned char patNum, bitDepth, trigger_type, maxPatNum, ledSelect;
			//bool invertPat, insertBlack, bufSwap, trigOutPrev;
			//unsigned int patExposure, patPeriod;

			//for (int i = 0; i < numParams; i += 3)
			//{
			//	trigger_type = params[i] & 0x3;

			//	VarExpInsertTriggerItem(trigger_type);

			//	patNum = (params[i] >> 2) & 0x3F;
			//	bitDepth = (params[i] >> 8) & 0xF;

			//	if (bitDepth < 1 || bitDepth > 8)
			//	{
			//		sprintf(dispStr, "Invalid bit-depth in Var Exp PAT LUT entry%d ", i);
			//		Log +=(dispStr);
			//		continue;
			//	}

			//	if (bitDepth == 1)
			//		maxPatNum = 24;
			//	else if (bitDepth == 5)
			//		maxPatNum = 3;
			//	else if (bitDepth == 7)
			//		maxPatNum = 2;
			//	else
			//		maxPatNum = 24 / bitDepth - 1;

			//	if (patNum > maxPatNum)
			//	{
			//		sprintf(dispStr, "Invalid pattern-number in Var Exp PAT LUT entry%d ", i);
			//		Log +=(dispStr);
			//		continue;
			//	}

			//	ledSelect = (params[i] >> 12) & 0x7;

			//	if ((params[i] & 0x00010000))
			//		invertPat = true;
			//	else
			//		invertPat = false;

			//	if ((params[i] & 0x00020000))
			//		insertBlack = true;
			//	else
			//		insertBlack = false;

			//	if ((params[i] & 0x00040000) || (!i))
			//	{
			//		if (splashIndex >= numSplashLutEntries)
			//		{
			//			Log +=("Bad .ini! SplashLUT entries do not match the number of buffer swaps in PAT LUT");
			//			continue;
			//		}
			//		else
			//		{
			//			bufSwap = true;
			//			frameIndex = splashLut[splashIndex++];
			//		}
			//	}
			//	else
			//	{
			//		bufSwap = false;
			//	}

			//	if ((params[i] & 0x00080000))
			//		trigOutPrev = true;
			//	else
			//		trigOutPrev = false;

			//	patExposure = params[i + 1];
			//	patPeriod = params[i + 2];

			//	if (patNum == 24)
			//		ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(frameIndex, bitDepth, 0, 0, 0, 0, 0));
			//	else
			//		ui->listWidget_VarExpPatSeqLUT->addItem(VarExpGenerateItemText(frameIndex, bitDepth, patNum*bitDepth, (patNum + 1)*bitDepth - 1, invertPat, patExposure, patPeriod));

			//	int seqListLength = ui->listWidget_VarExpPatSeqLUT->count();

			//	QColor bgColor = GetColorFromIndex(ledSelect);
			//	if (patNum == 24)
			//		bgColor = Qt::black;

			//	ui->listWidget_VarExpPatSeqLUT->item(seqListLength - 1)->setBackgroundColor(bgColor);
			//	VarExpUpdateSeqItemData(trigger_type, patNum, bitDepth, ledSelect, frameIndex, invertPat, insertBlack, bufSwap, trigOutPrev, patExposure, patPeriod);
			//}
		}
		*/
		
		if (ini_SwpflashNum > ini_SwpflashIndex)
		{
			ini_ExtraSwapFlashNum = ini_SwpflashNum - ini_SwpflashIndex;
			for (unsigned int i = 0; i < ini_ExtraSwapFlashNum; i++)
				ini_ExtraSwapFlashLUT[i] = ini_SwpflashLut[ini_SwpflashIndex++];
		}
		break;
	}

	/*LED config 2 man made*/
	//case 30:                //DEFAULT.LED_ENABLE_MAN_MODE
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.LED_ENABLE_MAN_MODE");
	//	tempBoolVar = (params[0]) ? true : false;
	//	ui->radioButton_ColorDisplayAuto->setChecked(!tempBoolVar);
	//	ui->radioButton_ColorDisplayManual->setChecked(tempBoolVar);
	//	break;
	//case 31:                //DEFAULT.MAN_ENABLE_RED_LED
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.MAN_ENABLE_RED_LED");
	//	tempBoolVar = params[0] ? true : false;
	//	if (ui->radioButton_ColorDisplayManual->isChecked())
	//		ui->checkBox_RedEnable->setChecked(tempBoolVar);
	//	else
	//		ui->checkBox_RedEnable->setChecked(false);
	//	break;
	//case 32:                //DEFAULT.MAN_ENABLE_GRN_LED
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.MAN_ENABLE_GRN_LED");
	//	tempBoolVar = params[0] ? true : false;
	//	if (ui->radioButton_ColorDisplayManual->isChecked())
	//		ui->checkBox_GreenEnable->setChecked(tempBoolVar);
	//	else
	//		ui->checkBox_GreenEnable->setChecked(false);
	//	break;
	//case 33:                //DEFAULT.MAN_ENABLE_BLU_LED
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.MAN_ENABLE_BLU_LED");
	//	tempBoolVar = params[0] ? true : false;
	//	if (ui->radioButton_ColorDisplayManual->isChecked())
	//		ui->checkBox_BlueEnable->setChecked(tempBoolVar);
	//	else
	//		ui->checkBox_BlueEnable->setChecked(false);
	//	break;
	/*video config*/
	//case 34:				//DEFAULT.PORTCONFIG.PORT
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PORT");
	//	ui->comboBox_InputSourceList->setCurrentIndex(params[0]);
	//	break;
	//case 35:				//DEFAULT.PORTCONFIG.BPP
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.BPP");
	//	ui->comboBox_InputSourceOptionList->setCurrentIndex(params[0]);
	//	break;
	//case 36:				//DEFAULT.PORTCONFIG.PIX_FMT
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PIX_FMT");
	//	if ((params[0] > 0) && ((ui->comboBox_InputSourceList->currentIndex() == 1) || (ui->comboBox_InputSourceList->currentIndex() == 3)))
	//	{
	//		ShowError("Wrong pixel format in the .ini file for the chosen port\r\n");
	//		break;
	//	}
	//	if (ui->comboBox_InputSourceList->currentIndex() == 2)
	//	{
	//		if ((params[0] == 1) || (params[0] > 2))
	//		{
	//			ShowError("Wrong pixel format in the .ini file for the chosen port\r\n");
	//			break;
	//		}
	//		if (params[0] == 2)
	//			params[0] = 1;
	//	}
	//	if ((ui->comboBox_InputSourceList->currentIndex() == 0) && (params[0] > 2))
	//	{
	//		ShowError("Wrong pixel format in the .ini file for the chosen port\r\n");
	//		break;
	//	}
	//	ui->comboBox_PixelFormatList->setCurrentIndex(params[0]);
	//	break;
	//case 37:				//DEFAULT.PORTCONFIG.PORT_CLK
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PORT_CLK");
	//	ui->comboBox_PortClockList->setCurrentIndex(params[0]);
	//	break;
	//case 38:				//DEFAULT.PORTCONFIG.ABC_MUX
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.ABC_MUX");
	//	ui->comboBox_SwapSelectList->setCurrentIndex(params[0]);
	//	break;
	//case 39:				//DEFAULT.PORTCONFIG.PIX_MODE
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.PIX_MODE");
	//	ui->spinBox_FPDPixMode->setValue(params[0]);
	//	break;
	//case 40:				//DEFAULT.PORTCONFIG.SWAP_POL
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.SWAP_POL");
	//	if (params[0])
	//		ui->checkBox_FPDInvPol->setChecked(true);
	//	else
	//		ui->checkBox_FPDInvPol->setChecked(false);
	//	break;
	//case 41:				//DEFAULT.PORTCONFIG.FLD_SEL
	//	if (numParams > 1)
	//		ShowError("Wrong number of parameters in the chosen .ini file for DEFAULT.PORTCONFIG.FLD_SEL");
	//	ui->comboBox_FPDFieldSelectList->setCurrentIndex(params[0]);
	//	break;

	default:
		break;
	}
}

//��������Ϣ ���͸�ͶӰ�� ��Ҫ���͵���Ϣ�������
/*
	0����֤����ʱ����ع�ʱ���Ƿ���ȷ
	1������API��ջ���
	2������ͼ����Ϣ�����µĻ���
	3����֡��֤����ʱ����ع�ʱ���Ƿ���ȷ
	4��*��ԭSPLASH���� ��*���Բ�����
	5��������API���趨displayMode:false��FLAH����true��ͶӰ�Ǽ���
	6��������API) �趨һЩͶӰ������Ϣ��һ��������ͼ �Ƿ�������ͶӰ �Լ�����֡��Ҫ����
	7��������API) �ϴ��ع�ʱ�������ʱ��
	8��������API) �趨trigermode ��Ҫʲô���͵Ĵ�����ʽ �ڲ���������VSYNC����
	9��������API) Pattern LUT ����
	10��������API) Swap Img LUT ����
*/
bool Projector::Send_PrtSeqCode()
{
	if (ini_isVarExpTrigMode)
	{
		Log += "Error: Pattern is VarExpMode, Plase using Send_VarExpPrtSeqCode function.\r\n";
		return false;
	}
	unsigned int min_pat_exposure[8] = { 235, 700, 1570, 1700, 2000, 2500, 4500, 8333 };//��С�ع�ʱ��
	unsigned int worstCaseBitDepth = 0;
	char errorStr[256];
	unsigned char splashLut[64];
	//checking time.
	if (exposure_time > period_time)
	{
		Log += "Error: Pattern exposure setting voilation, it should be, Pattern Exposure = Pattern Period or (Pattern Period - Pattern Exposure) > 230us.\r\n";
		return false;
	}
	if (exposure_time != period_time && period_time - exposure_time <= 230)
	{
		Log += "Error: Pattern exposure setting voilation, it should be, Pattern Exposure = Pattern Period or (Pattern Period - Pattern Exposure) > 230us.\r\n";
		return false;
	}
	DLPC350_ClearPatLut();//ImgLut dont need to clean up
	int i;
	int num_pats_in_exposure = 1;

	if (PatSeqLUT_data.size() == 0)
	{
		Log += "Error: No Pat data aviliable.\r\n";
		return false;
	}

	for (i = 0; i < PatSeqLUT_data.size(); i++)	//for each pat
	{
		Pattern pat = PatSeqLUT_data[i];
		if (i == 0 && pat.triger_type == 3)	//first pat cannot be No trigger
		{
			Log += "Error:First Item must be triggered.Please select a Trigger_In_Type other than No Trigger.\r\n";
			return false;
		}

		//if trigOut = New 
		//default trigOutPrev is false(0) no matter what mode in.
		if (pat.trigOutPrev == false)
		{
			if (num_pats_in_exposure != 1)
			{
				if (exposure_time / num_pats_in_exposure < min_pat_exposure[worstCaseBitDepth])
				{
					sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d.\r\n", exposure_time / num_pats_in_exposure, min_pat_exposure[worstCaseBitDepth], worstCaseBitDepth + 1);
					Log += errorStr;
					return false;
				}
			}
			else//only one pattern have been setted if it is 8bit need to forsure
			{
				if (i == 0)
				{
					//leave for next frame to validate the time of exposure this frame
				}
				else
				{
					if (exposure_time < min_pat_exposure[worstCaseBitDepth])
					{
						sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d.\r\n", exposure_time / num_pats_in_exposure, min_pat_exposure[worstCaseBitDepth], worstCaseBitDepth + 1);
						Log += errorStr;
						return false;
					}
				}
			}
			worstCaseBitDepth = 0;
			if (pat.bit_depth - 1 > worstCaseBitDepth)
			{
				worstCaseBitDepth = pat.bit_depth - 1;
			}
			num_pats_in_exposure = 1;

		}
		//if trigOut = PREV
		else
		{
			num_pats_in_exposure++;
			if (pat.bit_depth - 1 > worstCaseBitDepth) //remenber worstBitDepth
				worstCaseBitDepth = pat.bit_depth - 1;
		}

		//write into memory on pc
		if (DLPC350_AddToPatLut(pat.triger_type, pat.pat_num, pat.bit_depth, pat.color_code,
			pat.need_inv, pat.need_clear_DMD, pat.buffer_swap, pat.trigOutPrev) < 0)
		{
			Log += "Error: Error in Updating LUT.\r\n";
			return false;
		}
		//rebuilt PatLut(not necessary)
	}

	//clean up the final pattern info
	if (num_pats_in_exposure != 1)
	{
		if (exposure_time / num_pats_in_exposure < min_pat_exposure[worstCaseBitDepth])
		{
			sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d.\r\n", exposure_time / num_pats_in_exposure, min_pat_exposure[worstCaseBitDepth], worstCaseBitDepth + 1);
			Log += errorStr;
			return false;
		}
	}
	else//only one pattern have been setted if it is 8bit need to forsure
	{
		if (exposure_time < min_pat_exposure[worstCaseBitDepth])
		{
			sprintf(errorStr, "Exposure time %d < Minimum Exposure time %d for bit depth %d.\r\n", exposure_time / num_pats_in_exposure, min_pat_exposure[worstCaseBitDepth], worstCaseBitDepth + 1);
			Log += errorStr;
			return false;
		}
	}
	//setting to flash mode
	DLPC350_SetPatternDisplayMode(false);
	//if play once or continue
	unsigned int numPatterns = 0;
	if (!isRepeat)
	{
		numPatterns = PatSeqLUT_data.size();
	}
	else
	{
		numPatterns = 1;	//����
	}
	//config DLPC pattern basic info
	if (DLPC350_SetPatternConfig(PatSeqLUT_data.size(), isRepeat, numPatterns, ini_SwpflashNum)<0)
	{
		Log += "Error: Error Sending Pattern Config.\r\n";
		return false;
	}
	//config exposure and period time
	if (DLPC350_SetExposure_FramePeriod(exposure_time, period_time)<0)
	{
		Log += "Error: Error Sending Exposure/Period time.\r\n";
		return false;
	}
	//�趨trigmode һ�㶼��internal 
	int trigMode = 1;
	if (DLPC350_SetPatternTriggerMode(trigMode) < 0)
	{
		Log += "Error: Error Sending Trigger mode.\r\n";
		return false;
	}
	//Send PatLUT
	if (DLPC350_SendPatLut() < 0)
	{
		Log += "Error: Error Sending PatLUT.\r\n";
		return false;
	}
	//change the data type to unsigned char*
	for (i = 0; i < ini_SwpflashNum; i++)
		splashLut[i] = ini_SwpflashLut[i];

	//Send ImgLUT
	if (DLPC350_SendImageLut(&splashLut[0], ini_SwpflashNum)==-1)
	{
		Log += "Error: Error Sending ImgLut.\r\n";
		return false;
	}
	return true;
}

bool Projector::Play()
{
	if (DLPC350_PatternDisplay(2) >= 0)
	{
		Log += "Projector: Playing the pattern...\r\n";
		return true;
	}
	else
	{
		Log += "Error:Playing pattern Error.\r\n";
		return false;
	}
}

bool Projector::Pause()
{
	int i = 0;
	unsigned int patMode;
	DLPC350_PatternDisplay(1);
	this_thread::sleep_for(chrono::microseconds(100));
	while (1)
	{
		DLPC350_GetPatternDisplay(&patMode);
		if (patMode == 1)
		{
			Log += "Projector: Pause...\r\n";
			return true;
		}
		else
			DLPC350_PatternDisplay(1);
		this_thread::sleep_for(chrono::microseconds(100));
		if (i++ > 5)
		{
			Log += "Error: Pause no respond.\r\n";
			return false;
		}
			
	}
}

bool Projector::Stop()
{
	if (DLPC350_GetMode(&SLmode) == 0)
	{
		if (SLmode)
		{
			if (DLPC350_GetPatternDisplay(&action) >= 0)
			{
				if (action != 0)
				{
					int i = 0;
					DLPC350_PatternDisplay(0);	//�����ͶӰ���ڽ��� ֹͣ
					this_thread::sleep_for(chrono::microseconds(100));
					while (1)
					{
						DLPC350_GetPatternDisplay(&action);
						if (action == 0)
						{
							Log += "Stop playing.\r\n";
							return true;
						}
						else
						{
							DLPC350_PatternDisplay(0);
						}
						this_thread::sleep_for(chrono::microseconds(100));
						if (i++ > 5)
						{
							Log += "Error: Set DLPC Pattern Display Error(Stop).\r\n";
							return false;
						}

					}
				}
				else
				{
					return true;
				}
			}
			else
			{
				Log += "Error: Get Pattern Display Status Error.\r\n";
				return false;
			}

		}
		else
		{
			Log += "Warnning: Projector not in Parttern Sequence Mode.\r\n";
			return false;
		}
	}
	else
	{
		Log += "Error:Can not stop because of can not get the Mode of proj.\r\n";
		return false;
	}
}

bool Projector::ValidatePatSeq()
{
	if (!SLmode)
	{
		Log += "Please change operating mode to Pattern Sequence before validating sequence.\r\n";
		return false;
	}
	
	Stop();

	if (DLPC350_StartPatLutValidate())
	{
		Log += "Error: Error validating Lut data.\r\n";
		return false;
	}
	unsigned int status;
	bool ready;
	int i = 0;

	while (true)
	{
		if (DLPC350_CheckPatLutValidate(&ready, &status) < 0)
		{
			Log+= "Error: Error validating LUT data.\r\n";
			return false;
		}

		if (ready)
		{
			break;
		}
		else
		{
			this_thread::sleep_for(chrono::seconds(1));
		}

		if (i++ > 5)
		{
			Log += "Error: Validating LUT data no respond.\r\n";
			return false;
		}
	}
	 ExpOOR = (status&BIT0) == BIT0;
	 PatNumOOR = (status&BIT1) == BIT1;
	 TrigOutOverLap = (status&BIT2) == BIT2;
	 BlkVecMiss = (status&BIT3) == BIT3;
	 PatPeriodShort = (status&BIT4) == BIT4;
	 if (!(status & BIT0) && !(status & BIT1))
	 {
		 Log += "Pattern Sequence pass the validation.\r\n";
		 return true;
	 }
	 else
	 {
		 Log += "Error: Pattern Sequence not pass the validation.\r\n";
		 return false;
	 }
		 
}

/*
�ȹرշ���ʱ��
����
�ٿ�������ʱ��
*/
void Projector::SoftwareReset()
{
	DLPC350_SoftwareReset();
	DLPC350_USB_Close();
	this_thread::sleep_for(chrono::seconds(5));
}

void Projector::GetSeqStatus()
{
	DLPC350_GetPatternDisplay(&action);
}
