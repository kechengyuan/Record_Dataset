#include <pangolin/pangolin.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <iomanip>
#include <opencv2/opencv.hpp>

using namespace std;

enum _JointType {
  JointType_SpineBase = 0,
  JointType_SpineMid = 1,
  JointType_Neck = 2,
  JointType_Head = 3,
  JointType_ShoulderLeft = 4,
  JointType_ElbowLeft = 5,
  JointType_WristLeft = 6,
  JointType_HandLeft = 7,
  JointType_ShoulderRight = 8,
  JointType_ElbowRight = 9,
  JointType_WristRight = 10,
  JointType_HandRight = 11,
  JointType_HipLeft = 12,
  JointType_KneeLeft = 13,
  JointType_AnkleLeft = 14,
  JointType_FootLeft = 15,
  JointType_HipRight = 16,
  JointType_KneeRight = 17,
  JointType_AnkleRight = 18,
  JointType_FootRight = 19,
  JointType_SpineShoulder = 20,
  JointType_HandTipLeft = 21,
  JointType_ThumbLeft = 22,
  JointType_HandTipRight = 23,
  JointType_ThumbRight = 24,
  JointType_Count = (JointType_ThumbRight + 1)
};


struct P3D
{
	double x,y,z;
	P3D(){}
	P3D(double x_, double y_, double z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}
};

mutex myMutex;
volatile bool refresh = false;
P3D* joint = new P3D[25];

void getData();

string fileName;

int main(int argc,char* argv[])
{
	if(argc != 3)
	{
		cout<<"Please enter file"<<endl;
		return 0;
	}

	int type = 0;
	if(0 == strcmp("-skeleton",argv[1])){
		type = 1;
	}else if(0 == strcmp("-image",argv[1])){
		type = 2;
	}
	cout<<type<<endl;

	fileName = argv[2];

	if(type == 1)
	{
		thread t1(getData);

		int w =1280;
		int h = 720;
		pangolin::CreateWindowAndBind("Main",1280,720);

		pangolin::OpenGlRenderState Visualization3D_camera(
			pangolin::ProjectionMatrix(w,h,400,400,w/2,h/2,0.1,1000),
			pangolin::ModelViewLookAt(-0,-0,-1, 0,0,0, pangolin::AxisY)
			);

		pangolin::View& Visualization3D_display = pangolin::CreateDisplay()
			.SetBounds(0.0, 1.0, 0.0, 1.0, -1280/(float)720)
			.SetHandler(new pangolin::Handler3D(Visualization3D_camera));

		
		P3D* showJoint = new P3D[25];

		glEnable(GL_DEPTH_TEST);
		while( !pangolin::ShouldQuit())
		{
			if(refresh)
			{
				myMutex.lock();
					memcpy(showJoint,joint,sizeof(P3D)*25);
					refresh = false;
				myMutex.unlock();
			}
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			Visualization3D_display.Activate(Visualization3D_camera);
			glPointSize(2.0f);
			glBegin(GL_POINTS);
				glColor3f(1.0,0.0,0.0);
				for(int i =0;i<25;i++){
					glVertex3f(showJoint[i].x,showJoint[i].y,showJoint[i].z);
				}
			glEnd();
			
			glColor3f(0.0,1.0,0.0);
			glBegin(GL_LINES);
				glVertex3f(showJoint[JointType_Head].x,showJoint[JointType_Head].y,showJoint[JointType_Head].z);
				glVertex3f(showJoint[JointType_Neck].x,showJoint[JointType_Neck].y,showJoint[JointType_Neck].z);

				glVertex3f(showJoint[JointType_Neck].x,showJoint[JointType_Neck].y,showJoint[JointType_Neck].z);
				glVertex3f(showJoint[JointType_SpineShoulder].x,showJoint[JointType_SpineShoulder].y,showJoint[JointType_SpineShoulder].z);
				
				glVertex3f(showJoint[JointType_SpineShoulder].x,showJoint[JointType_SpineShoulder].y,showJoint[JointType_SpineShoulder].z);
				glVertex3f(showJoint[JointType_SpineMid].x,showJoint[JointType_SpineMid].y,showJoint[JointType_SpineMid].z);

				glVertex3f(showJoint[JointType_SpineMid].x,showJoint[JointType_SpineMid].y,showJoint[JointType_SpineMid].z);
				glVertex3f(showJoint[JointType_SpineBase].x,showJoint[JointType_SpineBase].y,showJoint[JointType_SpineBase].z);

				glVertex3f(showJoint[JointType_SpineShoulder].x,showJoint[JointType_SpineShoulder].y,showJoint[JointType_SpineShoulder].z);
				glVertex3f(showJoint[JointType_ShoulderLeft].x,showJoint[JointType_ShoulderLeft].y,showJoint[JointType_ShoulderLeft].z);
				
				glVertex3f(showJoint[JointType_SpineShoulder].x,showJoint[JointType_SpineShoulder].y,showJoint[JointType_SpineShoulder].z);
				glVertex3f(showJoint[JointType_ShoulderRight].x,showJoint[JointType_ShoulderRight].y,showJoint[JointType_ShoulderRight].z);
				
				glVertex3f(showJoint[JointType_SpineBase].x,showJoint[JointType_SpineBase].y,showJoint[JointType_SpineBase].z);
				glVertex3f(showJoint[JointType_HipLeft].x,showJoint[JointType_HipLeft].y,showJoint[JointType_HipLeft].z);
				
				glVertex3f(showJoint[JointType_SpineBase].x,showJoint[JointType_SpineBase].y,showJoint[JointType_SpineBase].z);
				glVertex3f(showJoint[JointType_HipRight].x,showJoint[JointType_HipRight].y,showJoint[JointType_HipRight].z);
				

				
				glVertex3f(showJoint[JointType_ShoulderRight].x,showJoint[JointType_ShoulderRight].y,showJoint[JointType_ShoulderRight].z);
				glVertex3f(showJoint[JointType_ElbowRight].x,showJoint[JointType_ElbowRight].y,showJoint[JointType_ElbowRight].z);
				
				glVertex3f(showJoint[JointType_ElbowRight].x,showJoint[JointType_ElbowRight].y,showJoint[JointType_ElbowRight].z);
				glVertex3f(showJoint[JointType_WristRight].x,showJoint[JointType_WristRight].y,showJoint[JointType_WristRight].z);

				glVertex3f(showJoint[JointType_WristRight].x,showJoint[JointType_WristRight].y,showJoint[JointType_WristRight].z);
				glVertex3f(showJoint[JointType_HandRight].x,showJoint[JointType_HandRight].y,showJoint[JointType_HandRight].z);
				
				glVertex3f(showJoint[JointType_HandRight].x,showJoint[JointType_HandRight].y,showJoint[JointType_HandRight].z);
				glVertex3f(showJoint[JointType_HandTipRight].x,showJoint[JointType_HandTipRight].y,showJoint[JointType_HandTipRight].z);

				glVertex3f(showJoint[JointType_HandRight].x,showJoint[JointType_HandRight].y,showJoint[JointType_HandRight].z);
				glVertex3f(showJoint[JointType_ThumbRight].x,showJoint[JointType_ThumbRight].y,showJoint[JointType_ThumbRight].z);
				


				glVertex3f(showJoint[JointType_ShoulderLeft].x,showJoint[JointType_ShoulderLeft].y,showJoint[JointType_ShoulderLeft].z);
				glVertex3f(showJoint[JointType_ElbowLeft].x,showJoint[JointType_ElbowLeft].y,showJoint[JointType_ElbowLeft].z);

				glVertex3f(showJoint[JointType_ElbowLeft].x,showJoint[JointType_ElbowLeft].y,showJoint[JointType_ElbowLeft].z);
				glVertex3f(showJoint[JointType_WristLeft].x,showJoint[JointType_WristLeft].y,showJoint[JointType_WristLeft].z);

				glVertex3f(showJoint[JointType_WristLeft].x,showJoint[JointType_WristLeft].y,showJoint[JointType_WristLeft].z);
				glVertex3f(showJoint[JointType_HandLeft].x,showJoint[JointType_HandLeft].y,showJoint[JointType_HandLeft].z);

				glVertex3f(showJoint[JointType_HandLeft].x,showJoint[JointType_HandLeft].y,showJoint[JointType_HandLeft].z);
				glVertex3f(showJoint[JointType_HandTipLeft].x,showJoint[JointType_HandTipLeft].y,showJoint[JointType_HandTipLeft].z);
				
				glVertex3f(showJoint[JointType_HandLeft].x,showJoint[JointType_HandLeft].y,showJoint[JointType_HandLeft].z);
				glVertex3f(showJoint[JointType_ThumbLeft].x,showJoint[JointType_ThumbLeft].y,showJoint[JointType_ThumbLeft].z);
				
				
				
				glVertex3f(showJoint[JointType_HipRight].x,showJoint[JointType_HipRight].y,showJoint[JointType_HipRight].z);
				glVertex3f(showJoint[JointType_KneeRight].x,showJoint[JointType_KneeRight].y,showJoint[JointType_KneeRight].z);

				glVertex3f(showJoint[JointType_KneeRight].x,showJoint[JointType_KneeRight].y,showJoint[JointType_KneeRight].z);
				glVertex3f(showJoint[JointType_AnkleRight].x,showJoint[JointType_AnkleRight].y,showJoint[JointType_AnkleRight].z);

				glVertex3f(showJoint[JointType_AnkleRight].x,showJoint[JointType_AnkleRight].y,showJoint[JointType_AnkleRight].z);
				glVertex3f(showJoint[JointType_FootRight].x,showJoint[JointType_FootRight].y,showJoint[JointType_FootRight].z);



				glVertex3f(showJoint[JointType_HipLeft].x,showJoint[JointType_HipLeft].y,showJoint[JointType_HipLeft].z);
				glVertex3f(showJoint[JointType_KneeLeft].x,showJoint[JointType_KneeLeft].y,showJoint[JointType_KneeLeft].z);
				
				glVertex3f(showJoint[JointType_KneeLeft].x,showJoint[JointType_KneeLeft].y,showJoint[JointType_KneeLeft].z);
				glVertex3f(showJoint[JointType_AnkleLeft].x,showJoint[JointType_AnkleLeft].y,showJoint[JointType_AnkleLeft].z);
				
				glVertex3f(showJoint[JointType_AnkleLeft].x,showJoint[JointType_AnkleLeft].y,showJoint[JointType_AnkleLeft].z);
				glVertex3f(showJoint[JointType_FootLeft].x,showJoint[JointType_FootLeft].y,showJoint[JointType_FootLeft].z);
				
				
				
			glEnd();


			glFlush();

			pangolin::FinishFrame();
		}
	}else if(type == 2 )
	{
		int i = 0;
		while(true)
		{
			stringstream ss;
			ss<<fileName<<"\\"<<setfill('0')<<setw(5)<<i++<<".png";
			cv::Mat image;
			try
			{
				image = cv::imread(ss.str(),1);
			}
			catch(const std::exception& e)
			{
				break;
			}
			imshow("image",image);
			cv::waitKey(333);
		}
	}
	

	return 0;
}

void getData()
{
	fstream fin(fileName);
	if(!fin)
	{
		cout<<"file not exist"<<endl;
		return;
	}
	int frame_num = 0;
	int line = 0;
	int current_joint = 0;
	while (!fin.eof()) {
		line++;
		char buf[500];
		fin.getline(buf, 500);

		if (line == 1) {
			sscanf(buf, "%d", &frame_num);
			continue;
		}

		if ((line - 2) % 26 == 0) {
			current_joint = 0;
			continue;
		}

		if ((line != 1) && ((line - 2) % 26 > 0)) {
			double a, b, c, d, e, f, g, h, i, j, k;
			int t;
			sscanf(buf, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %d", &a, &b, &c,
					&d, &e, &f, &g, &h, &i, &j, &k, &t);
			myMutex.lock();
				joint[current_joint].x = a;
				joint[current_joint].y = b;
				joint[current_joint].z = c;
			myMutex.unlock();
			current_joint++;
		}
		if(current_joint == 25)
		{
			refresh = true;
			usleep(330000);
		}
	}
	fin.close();
}