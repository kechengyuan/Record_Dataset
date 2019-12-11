#pragma once
#include <librealsense2/rs.hpp>
#include <opencv2/highgui.hpp>
#include <memory>
#include <vector>
#include <Kinect.h>

struct Joint_Info;
class SavedInfo
{
public:
	std::vector<std::shared_ptr<cv::Mat>> rs_ColorImgs, rs_DepthImgs,kinect_ColorImgs,kinect_DepthImgs;
	std::vector<std::shared_ptr<Joint_Info>> Joints;
	int actionId = 0;
	int peopleId = 0;
	int sequenceId = 0;
};


class RealSense
{
public:
	rs2::pipeline pipe;
	rs2::config cfg;
	rs2::colorizer color_map;

	int width;
	int height;

	RealSense();
	~RealSense();

	void getFrameSet(std::shared_ptr<cv::Mat>& color, std::shared_ptr<cv::Mat>& depth);
};

struct Joint_Info
{
	std::vector<int> state;
	std::vector<cv::Point3f> p_worlds;
	std::vector<cv::Point2f> p_depths;
	std::vector<cv::Point2f> p_colors;
	std::vector<Vector4> rotations;
	INT64 timestamps;
	int body_num = 0;
	int joint_num = 0;

};

class Kinect
{
public:
	IKinectSensor* sensor = nullptr;
	IKinectPointerPoint* Kpp = nullptr;

	IKinectCoreWindow* pCoreWindow = nullptr;
	IKinectPointerEventArgs* pPointerEvent = nullptr;


	IColorFrameSource* colorSource = nullptr;
	IColorFrameReader* colorReader = nullptr;
	IColorFrame* colorFrame = nullptr;
	IFrameDescription* colorDescription = nullptr;
	int colorHeight = 0, colorWidth = 0;
	std::vector<UINT16> depthBuffer;

	IDepthFrameSource* depthSource = nullptr;
	IDepthFrameReader* depthReader = nullptr;
	IDepthFrame* depthFrame = nullptr;
	IFrameDescription* depthDescription = nullptr;
	int depthHeight = 0, depthWidth = 0;

	IBodyFrameSource* bodySource = nullptr;
	IBodyFrameReader* bodyReader = nullptr;
	IBodyFrame* bodyFrame = nullptr;
	ICoordinateMapper* mapper = nullptr;
	int bodyCount = 0;


	Kinect();
	~Kinect();

	void initColorFrame();
	void initDepthFrame();
	void initJoint();
	void draw(cv::Mat& img, Joint& r_1, Joint& r_2, ICoordinateMapper* mapper);
	void getData(std::shared_ptr<cv::Mat>& color, std::shared_ptr<cv::Mat>& depth, cv::Mat& skeleton, std::shared_ptr<Joint_Info>& joints);
};