#pragma once

/*
����
1�������㵽��ͼ����н���
2����ȡ���е����ά����
3���޳�����㣬�������������Ƶ�������
*/


//��������ͷ�ļ�
#include <string>
#include <vector>
#include <array>

//OpenCV�������ļ�
#include <opencv2\core.hpp>

enum MODE
{
	BINARY_SHIFT,		//��ֵ����
	SIN_SHIFT,			//��������
	BINARY,				//��������
	PRELOADE			//���趨��Ԥ������Ҫ��ͼ��
};
//�ֶ�ֱ�� ֻ��һ����������ֵ��ķֶ�ֱ��
struct Line_1D
{
	float a0, b0;
	float range_x[4];	//�Ա�����Χ
};
//ͼ�����ݴ洢������
enum WRITE_TYPE
{
	ALL_DATA,
	ROW_DATA,
	COL_DATA
};
//�궨���ݴ洢����
struct Calib_Data
{
	cv::Mat	R, T;
	cv::Mat Cam_Mat, Proj_Mat;
	cv::Mat	Cam_Dst, Proj_Dst;
};

//ͶӰ���Լ�¼

/*
�ڲ�ͬ��ͶӰ��������Ҫ�����޸�
col_step row_step ���Ƶ�ʵ����Ʋ���
col_cycle row_cycle	���Ƶ�ʵ���������
col_offset	row_offset ����ͼ���ʱ����Ҫ�Ķ�ȡ˳�� �Ӷ��ܹ�ʹ������λ�����м���������

is_defocusing �Ƿ����뽹��
is_calibrating �Ƿ��ڱ궨�����У��ڱ궨�����к�����Ҳʮ����Ҫ

��Ҫ����Ҫ��
col_cycles[0]<col_cycles[1];
col_cycles[2] represent the final meserment
*/

struct Proj_Strategy
{
	std::vector<int> col_steps;
	std::vector<int> col_cycles;
};



/********** ���ݲ������ **********/
/*
��ȡ��ǰ���г��������Ŀ¼
o	path	�ļ���·�� �洢
*/
void Get_Now_Path(std::string& path);
/*
��ȡĳ���ļ����µ��ļ����� ����
i	path	�ļ���·��
i	format	�ļ��ĸ�ʽ
o	files	�ļ����ƴ洢
*/
void Get_All_Files(const std::string& path, const std::string& format, std::vector<std::string>& files);
/*
��ȡ�ļ��е�����
i	root_name		ÿһ���ļ��еĸ����� ������ʽΪroot0,root1,...
i	number			�����ÿһ��ͼ��ı��
o	scene_dir_path	�ļ��е�·��
*/
void Get_Data_Path(const std::string& root_name, int number, std::string& scene_dir_path);
/*
����ͼ���е�float����
i	name	�ļ�����
i	data	ͼ��
i	type	�洢��������
*/
void Save_Float_Mat(const std::string& name, const cv::Mat& data, WRITE_TYPE type);
/*
��xml�ļ��л�ȡ�궨��Ϣ
i	file_name	�ļ�����
o	data		�궨��Ϣ
*/
void Xml_Calibration_Data(const std::string& file_name, Calib_Data& data);
/*
���������ݱ������ļ���
i	points		��������
i	filename	�ļ�����
*/
void Save_Points(const std::vector<cv::Point3f>& points, const std::string& filename);

/********** ͶӰ�ؽ���� **********/
bool Get_Wrap_Phase_Steps(std::vector<cv::Mat>& src_img, cv::Mat& wraped_phase);
void Init_Phase_Map_From_Load(const std::string& col_phase_dir, const Proj_Strategy& strtgy, cv::Mat& col_aphase_map, bool,bool);
void Get_Absolute_Phase_3(const std::vector<cv::Mat>& ph_img, const std::vector<int>& cycle, cv::Mat& out, bool need_filter = false);
void Get_Absolute_Phase_2(const std::vector<cv::Mat>& ph_img, const std::vector<int>& cycle, cv::Mat& out,bool need_filter = false);
float Fit_Line(const cv::Mat& aphase_map, bool is_row, Line_1D& line);
void Get_Final_PhaseV1(const std::string& phase_dir_name, const Proj_Strategy& strtgy, const cv::Mat& aphase2, cv::Mat& aphase3);
void Get_Final_PhaseV2(const std::string& phase_dir_name, const Proj_Strategy& strtgy, cv::Mat& aphase3);