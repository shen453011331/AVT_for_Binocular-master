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
//һЩȫ�ֱ����Ķ���
//boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer_ptr(new pcl::visualization::PCLVisualizer("test"));		//չʾ���ڵ�ָ���ʼ��
//pcl::PointCloud<pcl::PointXYZ>::Ptr ori_cloud_ptr(new pcl::PointCloud<pcl::PointXYZ>);								//ԭʼ���Ƴ�ʼ��

#define SYS_PAUSE	system("pause")
int main()
{
	/*
	һЩ������־����Ϣ
	*/

	Proj_Strategy strtgy;
	strtgy.col_steps = { 3,3 };
	strtgy.col_cycles = { 72,75 };
	strtgy.is_calibrating = false;
	strtgy.is_defocusing = true;
	Line_1D col_line;
	//�ļ��е�����
	string floder_name = "sample";
	string base_floder_name = "base_image_b";

	/*
	���ñ�׼ͼ�񣬽�ͶӰ���ϵ���λ����������Ķ�Ӧ��ϵ,���ֱ��
	ע�� ���ֱ�߽������x��Ҫ��2�������յĽ��
	*/

	Mat col_aphase_std;
	Mat col_aphase_map2;
	Line_1D tow_cycle_line;
	Init_Phase_Map_From_Load(base_floder_name, strtgy, col_aphase_std,true,false);
	Fit_Line(col_aphase_std, false, tow_cycle_line);

	/*
	��ȡһ��ͼ���е�baseͼ�����࣬�۲���λЧ��,�����λͼ������֮�����λ������ؽ�����Ҫ������һ�����˲��Ȳ���
	*/
	Init_Phase_Map_From_Load(floder_name, strtgy, col_aphase_map2,false,true);

	/*
	������λ���ؽ�����ͼ�����λ��Ϣ��Ȼ����Ϊ���������������������
	*/
	Mat col_aphase_map3_V1,col_aphase_map3_V2;
	strtgy.col_cycles.push_back(36);
	strtgy.col_steps.push_back(3);
	//Get_Final_PhaseV1(floder_name,strtgy,col_aphase_map2,col_aphase_map3_V1);
	Get_Final_PhaseV2(floder_name, strtgy, col_aphase_map3_V2);

}
