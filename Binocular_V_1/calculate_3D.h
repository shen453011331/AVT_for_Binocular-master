#pragma once

/*
用于
1、将拍摄到的图像进行解相
2、求取所有点的三维坐标
3、剔除错误点，并减少整个点云的数据量
*/


//数据类型头文件
#include <string>
#include <vector>
#include <array>

//OpenCV库类型文件
#include <opencv2\core.hpp>

enum MODE
{
	BINARY_SHIFT,		//二值相移
	SIN_SHIFT,			//正弦相移
	BINARY,				//条纹相移
	PRELOADE			//从设定中预加载需要的图像
};
//分段直线 只有一个覆盖整个值域的分段直线
struct Line_1D
{
	float a0, b0;
	float range_x[4];	//自变量范围
};
//图像数据存储的类型
enum WRITE_TYPE
{
	ALL_DATA,
	ROW_DATA,
	COL_DATA
};
//标定数据存储类型
struct Calib_Data
{
	cv::Mat	R, T;
	cv::Mat Cam_Mat, Proj_Mat;
	cv::Mat	Cam_Dst, Proj_Dst;
};

//投影策略记录

/*
在不同的投影过程中需要进行修改
col_step row_step 多个频率的相移步数
col_cycle row_cycle	多个频率的相移周期
col_offset	row_offset 处理图像的时候需要的读取顺序 从而能够使绝对相位不在中间留下跳变

is_defocusing 是否是离焦的
is_calibrating 是否处于标定过程中，在标定过程中横条纹也十分重要

需要满足要求：
col_cycles[0]<col_cycles[1];
col_cycles[2] represent the final meserment
*/

struct Proj_Strategy
{
	std::vector<int> col_steps;
	std::vector<int> col_cycles;
};



/********** 数据操作相关 **********/
/*
获取当前运行程序的运行目录
o	path	文件夹路径 存储
*/
void Get_Now_Path(std::string& path);
/*
获取某个文件夹下的文件名称 保存
i	path	文件夹路径
i	format	文件的格式
o	files	文件名称存储
*/
void Get_All_Files(const std::string& path, const std::string& format, std::vector<std::string>& files);
/*
获取文件夹的名称
i	root_name		每一个文件夹的根名称 命名方式为root0,root1,...
i	number			输入的每一组图像的编号
o	scene_dir_path	文件夹的路径
*/
void Get_Data_Path(const std::string& root_name, int number, std::string& scene_dir_path);
/*
保存图像中的float数据
i	name	文件名称
i	data	图像
i	type	存储的数据量
*/
void Save_Float_Mat(const std::string& name, const cv::Mat& data, WRITE_TYPE type);
/*
从xml文件中获取标定信息
i	file_name	文件名称
o	data		标定信息
*/
void Xml_Calibration_Data(const std::string& file_name, Calib_Data& data);
/*
将点云数据保存在文件中
i	points		点云数据
i	filename	文件名称
*/
void Save_Points(const std::vector<cv::Point3f>& points, const std::string& filename);

/********** 投影重建相关 **********/
bool Get_Wrap_Phase_Steps(std::vector<cv::Mat>& src_img, cv::Mat& wraped_phase);
void Init_Phase_Map_From_Load(const std::string& col_phase_dir, const Proj_Strategy& strtgy, cv::Mat& col_aphase_map, bool,bool);
void Get_Absolute_Phase_3(std::vector<cv::Mat>& ph_img, std::vector<int> cycle, cv::Mat& out, bool need_filter = false);
void Get_Absolute_Phase_2(std::vector<cv::Mat>& ph_img, std::vector<int> cycle, cv::Mat& out,bool need_filter = false);
float Fit_Line(const cv::Mat& aphase_map, bool is_row, Line_1D& line);
void Get_Final_PhaseV1(const std::string& phase_dir_name, const Proj_Strategy& strtgy, const cv::Mat& aphase2, cv::Mat& aphase3);
void Get_Final_PhaseV2(const std::string& phase_dir_name, const Proj_Strategy& strtgy, cv::Mat& aphase3);