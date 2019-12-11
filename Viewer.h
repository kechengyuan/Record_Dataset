#pragma once
#include <pangolin/pangolin.h>
#include <thread>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <memory>
#include <atomic>




class Viewer
{
public:
	Viewer() {};
	Viewer(int w, int h,int w2,int h2);
	void run();
	void publishRsFrame(std::shared_ptr<cv::Mat> frame);

	void publishKinectFrame(cv::Mat frame);
	void publishWelcome(cv::Mat frame);
	void setEndWelcome();
private:
	bool showWelcome = true;
	std::mutex welcomeVideoMutex;
	std::shared_ptr<cv::Mat> internalWelcomeImg;
	bool welcomeVideoChanged = false;

	int kinect_w, kinect_h;
	int rs_w, rs_h;
	bool running;

	std::thread runThread;

	std::shared_ptr<cv::Mat> internalRsImg;

	std::mutex rsVideoMutex;
	bool rsVideoChanged;


	std::shared_ptr<cv::Mat> internalKinectImg;

	std::mutex kinectVideoMutex;

	bool kinectVideoChanged;

	int settingActionID = 0;
	int settingPeopleID = 0;
};