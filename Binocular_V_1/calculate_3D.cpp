#include "stdafx.h"
#include "calculate_3D.h"


#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\calib3d.hpp>

using namespace std;
using namespace cv;


#define PI CV_PI
#define SHOW_PIC 
cv::Size proj_img_size = cv::Size(1824, 1140);
cv::Size cam_img_size = cv::Size(2456, 2058);
/********** ���ݲ������ **********/
void Save_Float_Mat(const string& name, const cv::Mat& data, WRITE_TYPE type)
{
	ofstream file;
	file.open(name.c_str());
	if (!file)
	{
		return;
	}
	if (type == ROW_DATA)
	{
		int row = int(data.rows / 2);
		const float* data_ptr = data.ptr<float>(row);
		for (int i = 0; i < data.cols; i++)
		{
			file << data_ptr[i] << " ";
		}
		file << endl;
	}
	else if (type == COL_DATA)
	{
		int col = int(data.cols / 2);
		for (int i = 0; i < data.rows; i++)
		{
			file << data.ptr<float>(i)[col] << " ";
		}
		file << endl;
	}
	else if (type == ALL_DATA)
	{
		int i, j;
		for (i = 0; i < data.rows; i++)
		{
			const float* data_ptr = data.ptr<float>(i);
			for (j = 0; j < data.cols; j++)
			{
				file << data_ptr[j] << " ";
			}
			file << endl;
		}
	}
	file.close();
}
void Save_Points(const vector<Point3f>& points, const string& filename)
{
	ofstream outfile;
	outfile.open(filename);
	if (!outfile)
	{
		return;
	}
	else
	{
		for (int i = 0; i < points.size(); i++)
		{
			outfile << points[i].x << " " << points[i].y << " " << points[i].z << endl;
		}
	}
}
void Get_Now_Path(string& path)
{
	char buffer[_MAX_PATH];
	_getcwd(buffer, _MAX_PATH);
	for (int i = 0; i < _MAX_PATH; i++)
		if (buffer[i] == '\\')
			buffer[i] = '/';
	path = buffer;
}
void Get_All_Files(const string& path, const string& format, vector<string>& files)
{
	files.clear();
	intptr_t  hFile = 0;//�ļ����  
	struct _finddata_t fileinfo;//�ļ���Ϣ 
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*" + format).c_str(), &fileinfo)) != -1) //�ļ�����
	{
		do
		{
			files.push_back(fileinfo.name);//��������ļ��У������ļ���
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
void Get_Data_Path(const string& root_name, int number, string& scene_dir_path )
{
	//��ȡ��ǰ���е��ļ���
	string now_path;
	Get_Now_Path(now_path);
	scene_dir_path = now_path + "/" + root_name + to_string(number);
}
void Xml_Calibration_Data(const string& filename, Calib_Data& data)
{
	FileStorage fsread;
	fsread.open(filename, FileStorage::READ);
	fsread["R"] >> data.R;
	fsread["T"] >> data.T;
	fsread["cam_left"] >> data.Cam_Mat;
	fsread["cam_right"] >> data.Proj_Mat;
	fsread["distCoeff_Left"] >> data.Cam_Dst;
	fsread["distCoeff_Right"] >> data.Proj_Dst;
	cout << "Stereo Cam Data loaded.\n" <<
		"R:\n" << data.R << endl <<
		"T:\n" << data.T << endl <<
		"Cam Mat:\n" << data.Cam_Mat << endl <<
		"Proj Mat:\n" << data.Proj_Mat << endl <<
		"DistCoeffs of Cam:\n" << data.Cam_Dst << endl <<
		"DistCoeffs of proj:\n" << data.Proj_Dst << endl;
}
/********** ͶӰ�ؽ���� **********/
//�ڲ���һЩС����
int get_element_pos(valarray<float> arr, float val)
{
	int i;
	for (i = 0; i < arr.size(); i++)
	{
		if (abs(arr[i] - val) < FLT_EPSILON)
			break;
	}
	return i;
}
float fit_line(const valarray<float> _x, const valarray<float> _y, float& a, float& b)
{
	int size_x = _x.size();
	int size_y = _y.size();
	if (size_x != size_y)
		return -1.0;
	int n = size_x;
	float sum_x = _x.sum();
	float sum_y = _y.sum();
	valarray<float> _xy = _x*_y;
	float sum_xy = _xy.sum();
	valarray<float> _xx = _x*_x;
	float sum_xx = _xx.sum();
	//y = bx + a
	b = (n*sum_xy - sum_x*sum_y) / (n*sum_xx - sum_x*sum_x);
	a = sum_y / n - b*sum_x / n;
	//���㷽��
	valarray<float> y = b*_x + a;
	valarray<float> rms = (y - _y)*(y - _y);
	return rms.sum() / rms.size();
}
void outlier_detector(Mat& souce, Mat& mask)
{
	int height = souce.rows;
	int width = souce.cols;
	Mat temp_out;
	Mat kernal = (Mat_<float>(1, 2) << 1, -1);
	sepFilter2D(souce, temp_out, souce.depth(), kernal, Mat::ones(1, 1, CV_32FC1));
	mask.create(height, width, CV_8UC1);
	int i, j;
	for (i = 0; i < height; i++)
	{
		float* data_temp = temp_out.ptr<float>(i);
		unsigned char* data_mask = mask.ptr<unsigned char>(i);
		for (j = 0; j < width; j++)
		{
			if (abs(data_temp[j]) > PI / 2)
				data_mask[j] = 0;
			else
				data_mask[j] = 1;
		}
	}
}
float fit_plan(vector<Point3f>& xyz, array<float, 3>& a, bool if_RMS)
{
	int n_points = xyz.size();
	float sum_xx = 0, sum_yy = 0, sum_xy = 0;
	float sum_x = 0, sum_y = 0, sum_z = 0;
	float sum_xz = 0, sum_yz = 0;
	int i, x, y, z;
	for (i = 0; i < n_points; i++)
	{
		x = xyz[i].x;
		y = xyz[i].y;
		z = xyz[i].z;
		sum_xx += x*x;
		sum_yy += y*y;
		sum_xy += x*y;

		sum_x += x;
		sum_y += y;
		sum_z += z;

		sum_xz += x*z;
		sum_yz += y*z;
	}
	float p[] = 
	{ sum_xx,sum_xy,sum_x,
		sum_xy,sum_yy,sum_y,
		sum_x, sum_y, (float)n_points };
	float q[] = { sum_xz, sum_yz, sum_z };
	Mat A = Mat::Mat(3, 3, CV_32FC1, p);
	Mat B = Mat::Mat(3, 1, CV_32FC1, q);
	Mat X = Mat::Mat(3, 1, CV_32FC1);
	if (!solve(A, B, X, DECOMP_CHOLESKY))
	{
		return -1;
	}

	a[0] = X.ptr<float>(0)[0];
	a[1] = X.ptr<float>(1)[0];
	a[2] = X.ptr<float>(2)[0];

	//����RMS
	if (if_RMS)
	{
		/*
		�����ţ��кܶ�ĵ㣬���ƽ��֮�� RMS �ܴ���Ϊ��Щ�㶼�����ر�õ��ؽ��㣬��Χ������һ���Ĵ���
		���ֿ��ܵķ���
		1������RMSɸѡ�㣬 ˭��RMS�ر�� ���������Եķֲ㣬 �ʹ����޳�����Щ�㡣
		2���кܴ�RMS�ĵ㣬ѡ�񲻲�������ı궨���̣����������Ļ����궨��������Ʊػ���١�
		*/
		float RMS = 0;
		float _z = 0;
		for (i = 0; i < n_points; i++)
		{
			x = xyz[i].x;
			y = xyz[i].y;
			z = xyz[i].z;

			_z = a[0] * x + a[1] * y + a[2];
			RMS = (_z - z)*(_z - z);
		}
		RMS /= n_points;
		RMS = sqrt(RMS);
		return RMS;
	}
	else
		return 0;
}
void coordinate_to_phase(const Mat& row_phase_map, const Mat& col_phase_map, const Mat& row_phase_mask, const Mat& col_phase_mask,
	const vector<Point2f>& cord_points, vector<Point2f>& phase_points, Size wnd_size = Size(5, 5))
{
	phase_points.clear();
	int half_wnd_height = (wnd_size.height - 1) / 2;
	int half_wnd_width = (wnd_size.width - 1) / 2;
	int height = row_phase_map.rows;
	int width = row_phase_map.cols;
	Point2f phase_pointf;
	Point2i cord_pointi;
	int n_points = cord_points.size();
	vector<Point3f> row_points;
	vector<Point3f> col_points;
	Point3f xyz;
	array<float, 3> row_a = { 0,0,0 };
	array<float, 3> col_a = { 0,0,0 };

	int i, j, k;
	for (i = 0; i < n_points; i++)
	{
		//�Ȼ�Ϊ��������
		const Point2f& cord_pointf = cord_points[i];
		cord_pointi = Point2i(int(round(cord_pointf.x)), int(round(cord_pointf.y)));
		//��ȡcord_pointi��Χ����ĵ�
		for (j = cord_pointi.y - half_wnd_height; j <= cord_pointi.y + half_wnd_height; j++)
		{
			if (j < 0 || j >= height)
				continue;
			else
				for (k = cord_pointi.x - half_wnd_width; k <= cord_pointi.x + half_wnd_height; k++)
				{
					if (k < 0 || j >= width)
						continue;
					//���� ����ͼ���� ��������ѭ��
					if (col_phase_mask.ptr<float>(j)[k] != 0)
					{
						xyz.x = float(k);
						xyz.y = float(j);
						xyz.z = col_phase_map.ptr<float>(j)[k];
						col_points.push_back(xyz);
					}
					if (row_phase_mask.ptr<float>(j)[k] != 0)
					{
						xyz.x = float(k);
						xyz.y = float(j);
						xyz.z = row_phase_map.ptr<float>(j)[k];
						row_points.push_back(xyz);
					}
				}
		}
		/*
		���ƽ��
		������Ϊ z = a0*x + a1*y +a2
		*/
		cout << "point(" << i << "): col_rms:";
		cout << fit_plan(col_points, col_a, true) << "||row_rms";
		cout << fit_plan(row_points, row_a, true) << endl;
		//����궨�������ƽ���ϵ���λ����
		phase_pointf.x = col_a[0] * cord_pointf.x + col_a[1] * cord_pointf.y + col_a[2];
		phase_pointf.y = row_a[0] * cord_pointf.x + row_a[1] * cord_pointf.y + row_a[2];
		phase_points.push_back(phase_pointf);
	}
}
void sort_num(const vector<int>& in_vec, vector<int>& idx)
{
	/*�����밴��С�������� ����¼ԭʼ��idx��˳��*/
	vector<int> temp(in_vec);
	idx.resize(in_vec.size());
	for (int i = 0; i < idx.size(); i++)
	{
		idx[i] = i;
	}
	//���� ð������ idxһ�����䶯
	int temp_int = 0;
	for (int i = 0; i < temp.size() - 1; i++)
	{
		for (int j = 0; j < temp.size() - 1 - i; j++)
		{
			if (temp[j] > temp[j + 1])
			{
				swap(temp[j], temp[j + 1]);
				swap(idx[j], idx[j + 1]);
			}
		}
	}
}
//ͷ�ļ��еĺ���
/*��С���˷��Ȳ����� ��ð�����λ*/
bool Get_Wrap_Phase_Steps(vector<Mat>& src_img, Mat& wraped_phase)
{

	int step = src_img.size();
	if (step < 3)
	{
		return false;
	}
	int height = src_img[0].rows;
	int width = src_img[0].cols;

	//��ʼ��
	wraped_phase.create(height, width, CV_32FC1);
	Mat b = Mat::zeros(height, width, CV_32FC1);
	Mat c = Mat::zeros(height, width, CV_32FC1);
	int n;
	for (n = 0; n < step; n++)
		src_img[n].convertTo(src_img[n], CV_32FC1);
	float* phi = new float[step];
	float* cos_phi = new float[step];
	float* sin_phi = new float[step];
	for (n = 0; n < step; n++)
	{
		phi[n] = 2 * PI / step*n;
		cos_phi[n] = cos(phi[n]);
		sin_phi[n] = sin(phi[n]);
	}
	//���������λ
	for (n = 0; n < step; n++)
	{
		addWeighted(src_img[n], -cos_phi[n], b, 1, 0, b);
		addWeighted(src_img[n], sin_phi[n], c, 1, 0, c);
	}
	//��������������
	phase(c, b, wraped_phase);

	delete[] phi;
	delete[] cos_phi;
	delete[] sin_phi;
	return true;
}
/*��ʼ����λ��Ϣ ���Գ�ʼ����׼��λ�� Ҳ���Գ�ʼ�������ǰ������λ��*/
void Init_Phase_Map_From_Load(const string& col_phase_dir, const Proj_Strategy& strtgy,Mat& col_aphase_map, bool need_gauss,bool need_filter)
{
	string now_path;
	Get_Now_Path(now_path);
	string std_col_path = now_path + "/" + col_phase_dir;

	Mat buffer;					//��ͼ���������Buffer
	vector<vector<Mat>> src_img;//����ԭʼͼ���Buffer
	vector<Mat> ph_img;			//��Ű�����λͼ���Buffer

	int j, k;
	//��ȡ����������ͼ��
	src_img.clear();
	ph_img.clear();
	src_img.resize(strtgy.col_steps.size());
	for (j = 0; j < strtgy.col_steps.size(); j++)
	{
		for (k = 0; k < strtgy.col_steps[j]; k++)
		{
			string name = std_col_path + "/" + to_string(strtgy.col_cycles[j]) + "_" + to_string(k) + ".bmp";
			buffer = imread(name, 0);
			if (buffer.data == NULL)
			{
				cout << "Init Phase Map From Load Error: "<<to_string(strtgy.col_cycles[j]) + "_" + to_string(k)<< ".bmp read error." << endl;
				return;
			}
			if (need_gauss)
			{
				double sigmaX = ceil(strtgy.col_cycles[0] / 8);
				int size = int(round(sigmaX*4.5)) * 2 + 1;
				GaussianBlur(buffer, buffer, Size(size, size), sigmaX);
			}
			src_img[j].push_back(buffer.clone());
		}
	}
	//���������Ƶİ�����λ
	for (j = 0; j < src_img.size(); j++)
	{
		Get_Wrap_Phase_Steps(src_img[j], buffer);
		ph_img.push_back(buffer.clone());
	}
	//˫�������෽��
	if (strtgy.col_steps.size() == 2)
	{
		if (need_filter)
			Get_Absolute_Phase_2(ph_img, strtgy.col_cycles, buffer, need_filter);
		else
			Get_Absolute_Phase_2(ph_img, strtgy.col_cycles, buffer, need_filter);
	}
	else if(strtgy.col_steps.size() == 3)
	{
		if (need_filter)
			Get_Absolute_Phase_3(ph_img, strtgy.col_cycles, buffer, need_filter);
		else
			Get_Absolute_Phase_3(ph_img, strtgy.col_cycles, buffer, need_filter);
	}
	/*
	ʵ�ʲ������� 2�����Ķ������ƾ��Ȳ�̫�ࡣ
	��������������������������75��78�������������ƽ��н���
	*/
	col_aphase_map = buffer.clone();
}
/*���������෽��*/
//��˫�������෽�����Ȳ�������ô�ߵ�ʱ��ʹ�á� ��Ƶ�����һ��
void Get_Absolute_Phase_3(const vector<Mat>& ph_img,const vector<int>& cycle, Mat& out,bool need_filter)
{
	//��������������λ ��������������������λ ˳�� 72 75 90 �����ͼ��Ҫô�ϸ����ظ�˳��Ҫô��д���з���
	Mat fh[3];
	//���� ����¼�任���
	vector<int> idx;
	sort_num(cycle, idx);
	//ע�⣬��Զ�Ƕ̲�����������
	fh[0] = ph_img[idx[0]] - ph_img[idx[1]];
	fh[1] = ph_img[idx[0]] - ph_img[idx[2]];
	//ע�� ������һ��ǳ����
	fh[2] = ph_img[idx[0]];			

	float cycle_new[3] =
	{
		float(cycle[idx[0]] * cycle[idx[1]]) / float(cycle[idx[1]] - cycle[idx[0]]),
		float(cycle[idx[0]] * cycle[idx[2]]) / float(cycle[idx[2]] - cycle[idx[0]]),
		float(cycle[idx[0]])
	};

	int height = ph_img[idx[0]].size().height;
	int width = ph_img[idx[0]].size().width;

	//�������г���PI����С��-PI�ĵ�
	int i, j, k;
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < height; j++)
		{
			for (k = 0; k < width; k++)
			{
				if (fh[i].ptr<float>(j)[k] < 0)
					fh[i].ptr<float>(j)[k] += 2 * CV_PI;
			}
		}
	}
	//ÿһ�������Ƽ��δ洢
	Mat foit[3];
	for (i = 1; i < 3; i++)
		foit[i] = Mat::zeros(height, width, CV_32FC1);

	//������Ҫ���õײ�ָ���������ʹ��OpenCV��װ�õĲ��� �����һЩ�߼����﷨���� �ܹ�������ǵĴ����ٶ�
	foit[1] = (((cycle_new[0] / cycle_new[1])* fh[0]) - fh[1]) / (2 * CV_PI);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			foit[1].ptr<float>(i)[j] = round(foit[1].ptr<float>(i)[j]);
	//Сģ����ֵ�˲�һ��
	if (need_filter)
		medianBlur(foit[1], foit[1], 3);

	foit[2] = ((cycle_new[1] / cycle_new[2]) * (foit[1] * 2 * CV_PI + fh[1]) - fh[2]) / (2 * CV_PI);
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			foit[2].ptr<float>(i)[j] = round(foit[2].ptr<float>(i)[j]);
	if (need_filter)
		medianBlur(foit[2], foit[2], 3);

	out.create(height, width, CV_32FC1);
	out = foit[2] * 2 * CV_PI + fh[2];
	if (need_filter)
		cv::medianBlur(out, out, 3);
}

/*˫�������෽��*/
void Get_Absolute_Phase_2(const vector<Mat>& ph_img, const vector<int>& cycle, Mat& out,bool need_filter)
{
	//��������������λ ��������������������λ
	Mat fh[2];
	vector<int> idx;
	int offset = 0;
	sort_num(cycle, idx);	//idx�����С�������еĲ�����Ϣ
	if (cycle.size() != 2)
		offset = cycle.size() - 2;
	fh[0] = ph_img[idx[0+offset]] - ph_img[idx[1 + offset]];
	fh[1] = ph_img[idx[0 + offset]];

	float cycle_new[2] =
	{
		float(cycle[idx[0 + offset]] * cycle[idx[1 + offset]]) / float(cycle[idx[1 + offset]] - cycle[idx[0 + offset]]),
		float(cycle[idx[0 + offset]])
	};

	int height = ph_img[idx[0 + offset]].size().height;
	int width = ph_img[idx[0 + offset]].size().width;

	//�������г���PI����С��-PI�ĵ�
	int i, j, k;
	for (j = 0; j < height; j++)
	{
		for (k = 0; k < width; k++)
		{
			if (fh[0].ptr<float>(j)[k] < 0)
				fh[0].ptr<float>(j)[k] += 2 * CV_PI;
		}
	}

	//ÿһ�������Ƽ��δ洢
	Mat foit= Mat::zeros(height, width, CV_8UC1);

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			unsigned char foit_number = unsigned char(round(((cycle_new[0] / cycle_new[1])* fh[0].ptr<float>(i)[j] - fh[1].ptr<float>(i)[j]) / (2 * CV_PI)));
			if (foit_number > 100)
				foit_number = 0;
			foit.ptr<uchar>(i)[j] = foit_number;	
		}

	//��ֵ�˲� ���֤����ֵ�˲�Ч������
	if (need_filter)
	{
		//medianBlur(foit, foit, 11);
		Mat element = getStructuringElement(MORPH_RECT,Size(3,3));
		morphologyEx(foit, foit, MORPH_OPEN, element);
		medianBlur(foit, foit, 3);
	}

	out.create(height, width, CV_32FC1);
	foit.convertTo(foit, CV_32FC1);
	out = foit * 2 * CV_PI + fh[1];
	//���������Ʊ߽紦�����ǻ��кܲ��õ�Ч�����������ǽ�һ����out������ֵ�˲�
	if (need_filter)
	{
		medianBlur(out, out, 3);
		medianBlur(out, out, 3);
	}
}

/*���ֱ��*/
float Fit_Line(const Mat& aphase_map, bool is_row, Line_1D& line)
{
	Mat aphase_map_new;
	if (is_row)
		aphase_map_new = aphase_map.t();
	else
		aphase_map_new = aphase_map.clone();

	int length = aphase_map_new.cols;
	valarray<float> x(length);
	valarray<float> y(length);
	int i;
	//ȡ���м��һ��
	int row_ptr = aphase_map_new.rows / 2;
	float* row_data = aphase_map_new.ptr<float>(row_ptr);
	for (i = 0; i < aphase_map_new.cols; i++)
	{
		x[i] = (float)i;
		y[i] = row_data[i];
	}

	//�ų��߽�
	int offset = aphase_map_new.cols / 30;

	line.range_x[0] = offset;
	line.range_x[1] = length - 1 - offset;

	//����������ߵĸ������
	float rms;

	valarray<float> y0(y[slice(line.range_x[0], (line.range_x[1] - line.range_x[0]) + 1, 1)]);
	valarray<float> x0(x[slice(line.range_x[0], (line.range_x[1] - line.range_x[0]) + 1, 1)]);
	if ((rms = fit_line(x0, y0, line.a0, line.b0)) >= 0)
	{
		cout << "a,b:" << line.a0 << line.b0 << endl;
		cout << "fit error:" << rms << endl;
		return sqrt(rms);
	}
	else
	{
		return -1;
	}
}

/*��֮ǰ��õ���λ��Ϣ�������һ����������λ��ע�⣬��Ҫ����λ���й�һ��*/
void Get_Final_PhaseV1(const string& phase_dir_name, const Proj_Strategy& strtgy, const Mat& aphase2, Mat& aphase3)
{
	string now_path;
	Get_Now_Path(now_path);
	string std_col_path = now_path + "/" + phase_dir_name;

	Mat buffer;					//��ͼ���������Buffer
	vector<Mat> src_img;		//����ԭʼͼ���Buffer
	vector<Mat> ph_img;			//��Ű�����λͼ���Buffer

	int  k;
	//��ȡ����������ͼ��
	src_img.clear();
	ph_img.clear();
	for (k = 0; k < strtgy.col_steps[2]; k++)
	{
		string name = std_col_path + "/" + to_string(strtgy.col_cycles[2]) + "_" + to_string(k) + ".bmp";
		buffer = imread(name, 0);
		if (buffer.data == NULL)
		{
			cout << "Init Phase Map From Load Error: " << to_string(strtgy.col_cycles[2]) + "_" + to_string(k) << ".bmp read error." << endl;
			return;
		}
		src_img.push_back(buffer.clone());
	}

	//���������Ƶİ�����λ
	Get_Wrap_Phase_Steps(src_img, buffer);

	//����һ���ο���λ����һ����λ
	vector<int> cycle = strtgy.col_cycles;
	int new_cycle = abs(float(cycle[0] * cycle[1]) / float(cycle[0] - cycle[1]));
	int ori_cycle = float(cycle[1]);
	float rito = new_cycle / ori_cycle;
	ph_img.push_back(aphase2 / rito);
	ph_img.push_back(buffer);
	vector<int> new_cycles = { new_cycle,cycle[2] };
	//ע�� ���ﲻ���ò�Ƶ�ķ��������λ������Ҫ�õ��ֲ�����˼·��������һ�����������������λ��
	Get_Absolute_Phase_2(ph_img, new_cycles, aphase3, true);
}

/*����ֱ��ʹ���������Ʒ���������Ҫ��������ƥ��*/
void Get_Final_PhaseV2(const string& phase_dir_name, const Proj_Strategy& strtgy, Mat& aphase3)
{
	string now_path;
	Get_Now_Path(now_path);
	string std_col_path = now_path + "/" + phase_dir_name;

	Mat buffer;					//��ͼ���������Buffer
	vector<vector<Mat>> src_img;//����ԭʼͼ���Buffer
	vector<Mat> ph_img;			//��Ű�����λͼ���Buffer

	int j, k;
	//��ȡ����������ͼ��
	src_img.clear();
	ph_img.clear();
	src_img.resize(strtgy.col_steps.size());
	for (j = 0; j < strtgy.col_steps.size(); j++)
	{
		for (k = 0; k < strtgy.col_steps[j]; k++)
		{
			string name = std_col_path + "/" + to_string(strtgy.col_cycles[j]) + "_" + to_string(k) + ".bmp";
			buffer = imread(name, 0);
			if (buffer.data == NULL)
			{
				cout << "Init Phase Map From Load Error: " << to_string(strtgy.col_cycles[j]) + "_" + to_string(k) << ".bmp read error." << endl;
				return;
			}
			src_img[j].push_back(buffer.clone());
		}
	}
	//���������Ƶİ�����λ
	for (j = 0; j < src_img.size(); j++)
	{
		Get_Wrap_Phase_Steps(src_img[j], buffer);
		ph_img.push_back(buffer.clone());
	}

	Get_Absolute_Phase_3(ph_img, strtgy.col_cycles, aphase3,true);
}