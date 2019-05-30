#include "calculate_3D.h"
//#include "pointcloud_3D.h"
#include <iostream>

#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

//#include <pcl\visualization\pcl_visualizer.h>
//#include <pcl\io\io.h>

using namespace std;
using namespace cv;

#define SHOW 0
//一些全局变量的定义
//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer_ptr(new pcl::visualization::PCLVisualizer("test"));		//展示窗口的指针初始化
//pcl::PointCloud<pcl::PointXYZ>::Ptr ori_cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>);								//原始点云初始化

#define SYS_PAUSE	system("pause")
int main()
{
	/*
	一些用作标志的信息
	*/

	Proj_Strategy strtgy;
	strtgy.col_steps = { 3,3 };
	strtgy.col_cycles = { 72,75 };
	strtgy.is_calibrating = false;
	strtgy.is_defocusing = true;
	Line_1D col_line;
	//文件夹的名称
	string floder_name = "sample";
	string base_floder_name = "base_image_b";

	/*
	利用标准图像，解投影仪上的相位与像素坐标的对应关系,获得直线
	注意 这个直线解出来的x需要乘2才是最终的结果
	*/

	Mat col_aphase_std;
	Mat col_aphase_map2;
	Line_1D tow_cycle_line;
	Init_Phase_Map_From_Load(base_floder_name, strtgy, col_aphase_std,true,false);
	Fit_Line(col_aphase_std, false, tow_cycle_line);

	/*
	读取一组图像中的base图，解相，观察相位效果,这个相位图将用于之后的相位解相和重建，需要对其有一定的滤波等操作
	*/
	Init_Phase_Map_From_Load(floder_name, strtgy, col_aphase_map2,false,true);

	/*
	根据相位，重建其他图像的相位信息，然后作为第三个波长加入整体测量
	*/
	Mat col_aphase_map3_V1,col_aphase_map3_V2;
	strtgy.col_cycles.push_back(36);
	strtgy.col_steps.push_back(3);
	//Get_Final_PhaseV1(floder_name,strtgy,col_aphase_map2,col_aphase_map3_V1);
	Get_Final_PhaseV2(floder_name, strtgy, col_aphase_map3_V2);

}
