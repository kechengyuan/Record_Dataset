#include <opencv2/imgproc/imgproc.hpp>
#include "Sensor.h"


using namespace std;

RealSense::RealSense()
{
	cfg.enable_stream(RS2_STREAM_DEPTH);
	cfg.enable_stream(RS2_STREAM_COLOR);
	pipe.start(cfg);

	rs2::frameset frameset = pipe.wait_for_frames();
	rs2::video_frame color_frame = frameset.get_color_frame();
	height = color_frame.get_height();
	width = color_frame.get_width();

}

RealSense::~RealSense()
{

}

void RealSense::getFrameSet(std::shared_ptr<cv::Mat>& color, std::shared_ptr<cv::Mat>& depth)
{

	rs2::align align_to_color(RS2_STREAM_COLOR);
	rs2::frameset frameset = pipe.wait_for_frames();
	frameset = align_to_color.process(frameset);
	rs2::video_frame color_frame = frameset.get_color_frame();
	rs2::depth_frame depth_frame = frameset.get_depth_frame();

	std::vector<std::shared_ptr<cv::Mat>> vImg;

	cv::Mat frame_color = cv::Mat(color_frame.get_height(), color_frame.get_width(), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);
	color = std::shared_ptr<cv::Mat>(new cv::Mat);
	cv::cvtColor(frame_color, *color, CV_RGB2BGR);

	cv::Mat frame_depth = cv::Mat(depth_frame.get_height(), depth_frame.get_width(), CV_16UC1, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);
	depth = std::shared_ptr<cv::Mat>(new cv::Mat);
	frame_depth.copyTo(*depth);
}

Kinect::Kinect()
{
	GetDefaultKinectSensor(&sensor);
	sensor->Open();

	GetKinectCoreWindowForCurrentThread(&pCoreWindow);

	initColorFrame();
	initDepthFrame();
	initJoint();
}

Kinect::~Kinect()
{
	mapper->Release();

	colorDescription->Release();
	colorReader->Release();
	colorSource->Release();

	depthDescription->Release();
	depthReader->Release();
	depthSource->Release();

	bodyReader->Release();
	bodySource->Release();
	sensor->Close();
	sensor->Release();
}

void Kinect::initColorFrame()
{
	sensor->get_ColorFrameSource(&colorSource);
	colorSource->OpenReader(&colorReader);
	colorSource->get_FrameDescription(&colorDescription);
	colorDescription->get_Height(&colorHeight);
	colorDescription->get_Width(&colorWidth);

}

void Kinect::initDepthFrame()
{
	sensor->get_DepthFrameSource(&depthSource);
	depthSource->OpenReader(&depthReader);

	depthSource->get_FrameDescription(&depthDescription);
	depthDescription->get_Height(&depthHeight);
	depthDescription->get_Width(&depthWidth);

	depthBuffer.resize(depthWidth * depthHeight);
}

void Kinect::initJoint()
{
	sensor->get_BodyFrameSource(&bodySource);
	bodySource->OpenReader(&bodyReader);

	bodySource->get_BodyCount(&bodyCount);

	sensor->get_CoordinateMapper(&mapper);
}

void Kinect::draw(cv::Mat& img, Joint& r_1, Joint& r_2, ICoordinateMapper* mapper)
{
	if (r_1.TrackingState == TrackingState_Tracked && r_2.TrackingState == TrackingState_Tracked)
	{
		ColorSpacePoint t_point;  
		cv::Point   p_1, p_2;
		mapper->MapCameraPointToColorSpace(r_1.Position, &t_point);
		p_1.x = (int)t_point.X;
		p_1.y = (int)t_point.Y;
		mapper->MapCameraPointToColorSpace(r_2.Position, &t_point);
		p_2.x = (int)t_point.X;
		p_2.y = (int)t_point.Y;

		cv::line(img, p_1, p_2, cv::Vec3b(0, 255, 0), 5);
		cv::circle(img, p_1, 10, cv::Vec3b(255, 0, 0), -1);
		cv::circle(img, p_2, 10, cv::Vec3b(255, 0, 0), -1);
	}
}

void Kinect::getData(shared_ptr<cv::Mat>& color, shared_ptr<cv::Mat>& depth, cv::Mat& skeleton, shared_ptr<Joint_Info>& joints)
{
	
	int i = 0;
	while ((colorReader->AcquireLatestFrame(&colorFrame) != S_OK) || (depthReader->AcquireLatestFrame(&depthFrame) != S_OK) || (bodyReader->AcquireLatestFrame(&bodyFrame) != S_OK));

	cv::Mat frame_color = cv::Mat(colorHeight, colorWidth, CV_8UC4);
	cv::Mat frame_depth = cv::Mat(depthHeight, depthWidth, CV_16UC1, &depthBuffer[0]);

	colorFrame->CopyConvertedFrameDataToArray(colorHeight * colorWidth * 4, frame_color.data, ColorImageFormat_Bgra);
	depthFrame->CopyFrameDataToArray(static_cast<UINT>(depthBuffer.size()), &depthBuffer[0]);

	color = shared_ptr<cv::Mat>(new cv::Mat);
	cv::cvtColor(frame_color,*color,CV_BGRA2BGR);
	depth = shared_ptr<cv::Mat>(new cv::Mat);
	frame_depth.copyTo(*depth);

	IBody** bodyArr = new IBody * [bodyCount];
	for (int i = 0; i < bodyCount; i++)
		bodyArr[i] = nullptr;

	(*color).copyTo(skeleton);

	if (bodyFrame->GetAndRefreshBodyData(bodyCount, bodyArr) == S_OK) 
		for (int i = 0; i < bodyCount; i++)
		{
			BOOLEAN result = false;
			if (bodyArr[i]->get_IsTracked(&result) == S_OK && result)
			{
				TIMESPAN timestamps;
				colorFrame->get_RelativeTime(&timestamps);

				joints->body_num++;
				joints->timestamps = timestamps;
				Joint   jointArr[JointType_Count];
				JointOrientation jointRArr[JointType_Count];
				if (bodyArr[i]->GetJoints(JointType_Count, jointArr) == S_OK)
				{
					draw(skeleton, jointArr[JointType_Head], jointArr[JointType_Neck], mapper);
					draw(skeleton, jointArr[JointType_Neck], jointArr[JointType_SpineShoulder], mapper);

					draw(skeleton, jointArr[JointType_SpineShoulder], jointArr[JointType_ShoulderLeft], mapper);
					draw(skeleton, jointArr[JointType_SpineShoulder], jointArr[JointType_SpineMid], mapper);
					draw(skeleton, jointArr[JointType_SpineShoulder], jointArr[JointType_ShoulderRight], mapper);

					draw(skeleton, jointArr[JointType_ShoulderLeft], jointArr[JointType_ElbowLeft], mapper);
					draw(skeleton, jointArr[JointType_SpineMid], jointArr[JointType_SpineBase], mapper);
					draw(skeleton, jointArr[JointType_ShoulderRight], jointArr[JointType_ElbowRight], mapper);

					draw(skeleton, jointArr[JointType_ElbowLeft], jointArr[JointType_WristLeft], mapper);
					draw(skeleton, jointArr[JointType_SpineBase], jointArr[JointType_HipLeft], mapper);
					draw(skeleton, jointArr[JointType_SpineBase], jointArr[JointType_HipRight], mapper);
					draw(skeleton, jointArr[JointType_ElbowRight], jointArr[JointType_WristRight], mapper);

					draw(skeleton, jointArr[JointType_WristLeft], jointArr[JointType_ThumbLeft], mapper);
					draw(skeleton, jointArr[JointType_WristLeft], jointArr[JointType_HandLeft], mapper);
					draw(skeleton, jointArr[JointType_HipLeft], jointArr[JointType_KneeLeft], mapper);
					draw(skeleton, jointArr[JointType_HipRight], jointArr[JointType_KneeRight], mapper);
					draw(skeleton, jointArr[JointType_WristRight], jointArr[JointType_ThumbRight], mapper);
					draw(skeleton, jointArr[JointType_WristRight], jointArr[JointType_HandRight], mapper);

					draw(skeleton, jointArr[JointType_HandLeft], jointArr[JointType_HandTipLeft], mapper);
					draw(skeleton, jointArr[JointType_KneeLeft], jointArr[JointType_FootLeft], mapper);
					draw(skeleton, jointArr[JointType_KneeRight], jointArr[JointType_FootRight], mapper);
					draw(skeleton, jointArr[JointType_HandRight], jointArr[JointType_HandTipRight], mapper);

					bodyArr[i]->GetJointOrientations(JointType_Count, jointRArr);

					for (int i = 0; i < JointType_Count; i++)
					{
						if (jointArr[i].TrackingState == TrackingState_NotTracked)
						{
							Vector4 R;
							R.w = R.x = R.y = R.z = -1;
							joints->p_worlds.push_back(cv::Point3f(-1, -1, -1));
							joints->p_colors.push_back(cv::Point2f(-1, -1));
							joints->p_depths.push_back(cv::Point2f(-1, -1));
							joints->rotations.push_back(R);
						}
						else
						{
							ColorSpacePoint tc_point;
							DepthSpacePoint td_point;
							mapper->MapCameraPointToColorSpace(jointArr[i].Position, &tc_point);
							const CameraSpacePoint csp = jointArr[i].Position;
							mapper->MapCameraPointsToDepthSpace(1, &csp, 1, &td_point);
							joints->p_worlds.push_back(cv::Point3f(jointArr[i].Position.X, jointArr[i].Position.Y, jointArr[i].Position.Z));
							joints->rotations.push_back(jointRArr[i].Orientation);

							cv::Point2f p_color(tc_point.X, tc_point.Y);
							cv::Point2f p_depth(td_point.X, td_point.Y);

							if (jointArr[i].TrackingState == TrackingState_Inferred)
							{
								if (p_color.y > colorHeight || p_color.y<0 || p_color.x>colorWidth || p_color.x < 0)
									p_color = cv::Point2f(-1, -1);
								if (p_depth.y > depthHeight || p_depth.y<0 || p_depth.x>depthWidth || p_depth.x < 0)
									p_depth = cv::Point2f(-1, -1);
							}
							joints->p_colors.push_back(p_color);
							joints->p_depths.push_back(p_depth);

						}
						joints->state.push_back(jointArr[i].TrackingState);
						joints->joint_num++;
					}

				}
			}
		}

	delete[] bodyArr;

	colorFrame->Release();
	depthFrame->Release();
	bodyFrame->Release(); 
}
