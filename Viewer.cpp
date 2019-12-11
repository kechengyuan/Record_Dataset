#include <string>

#include "Viewer.h"
#include "Settings.h"
#include <unistd.h>

volatile bool isShow = false;


Viewer::Viewer(int w_, int h_, int w_2, int h_2)
{
	kinect_w = w_;
	kinect_h = h_;
	rs_w = w_2;
	rs_h = h_2;

	running = true;

	internalRsImg = std::shared_ptr<cv::Mat>(new cv::Mat(rs_h, rs_w, CV_8UC3, cv::Scalar(255, 255, 255)));
	internalKinectImg = std::shared_ptr<cv::Mat>(new cv::Mat(kinect_h, kinect_w, CV_8UC3, cv::Scalar(255, 255, 255)));
	internalWelcomeImg = std::shared_ptr<cv::Mat>(new cv::Mat(1080, 1920, CV_8UC3, cv::Scalar(255, 255, 255)));

	runThread = std::thread(&Viewer::run, this);
}

void Viewer::run()
{
	printf("Viewer Thead Start\n");
	const int UI_WIDTH = 320;
	const int IMAGE_WIDTH = 1920/3;
	const int IMAGE_HIGHT = 1080/3;
	pangolin::CreateWindowAndBind("Main", UI_WIDTH + IMAGE_WIDTH*2, 2*IMAGE_HIGHT);


	pangolin::View& rs_video = pangolin::Display("rs_video").SetBounds(0.0, pangolin::Attach::Pix(IMAGE_HIGHT), 0, pangolin::Attach::Pix(IMAGE_WIDTH)); 
	pangolin::View& kinect_video = pangolin::Display("kinect_video").SetBounds(pangolin::Attach::Pix(IMAGE_HIGHT), pangolin::Attach::Pix(2 * IMAGE_HIGHT), pangolin::Attach::Pix(0), pangolin::Attach::Pix(IMAGE_WIDTH));
	pangolin::View& welcome_video = pangolin::Display("welcome_video").SetBounds(0.0, pangolin::Attach::Pix(IMAGE_HIGHT*2), 0.0, pangolin::Attach::Pix(IMAGE_WIDTH*2)); 
	pangolin::GlTexture text_rs_Video(rs_w, rs_h, GL_RGB, false, 0, GL_RGB, GL_UNSIGNED_BYTE);
	pangolin::GlTexture text_kinect_Video(kinect_w, kinect_h, GL_RGB, false, 0, GL_RGB, GL_UNSIGNED_BYTE);
	pangolin::GlTexture text_welcome_Video(1920, 1080, GL_RGB, false, 0, GL_RGB, GL_UNSIGNED_BYTE);

	pangolin::CreateDisplay().SetBounds(0.0, 0.3, pangolin::Attach::Pix(UI_WIDTH), 1.0)
		.SetLayout(pangolin::LayoutOverlay).AddDisplay(rs_video).AddDisplay(kinect_video).AddDisplay(welcome_video);

	pangolin::CreatePanel("menu").SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));


	pangolin::Var<std::string> recordState("menu.Status:", isRecord?"Recording":"Waiting", false);
	pangolin::Var<int> currentPeople("menu.Current People :", settingPeopleID, 0, 0, false);
	pangolin::Var<int> currentAction("menu.Current Action :", settingActionID, 0, 0, false);

	pangolin::Var<bool> record("menu.Record", false, false);
	pangolin::Var<bool> saved("menu.Save", false, false);
	pangolin::Var<bool> next_Action("menu.Next Action", false, false);
	pangolin::Var<bool> next_People("menu.Next People", false, false);
	pangolin::Var<bool> prev_Action("menu.Prev Action", false, false);
	pangolin::Var<bool> prev_People("menu.Prev People", false, false);
	pangolin::Var<bool> cancel("menu.Cancel", false, false);

	pangolin::Var<int> currentSize("menu.Queue Size :", QueueSize, 0, 0, false);
	pangolin::Var<int> lastRecordSize("menu.Last Record Size :", LastRecordSize, 0, 0, false);
	
	pangolin::Var<bool> showKinect("menu.Show Kinect", true, true);
	pangolin::Var<bool> showRs("menu.Show Realsense", true, true);

	pangolin::Var<std::string> lastRecordFile("menu.Lsat Saved:", LastRecordFile, false);

	while (!pangolin::ShouldQuit() && running) {
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		if(showWelcome){
			welcomeVideoMutex.lock();
			if (welcomeVideoChanged) {text_welcome_Video.Upload(internalWelcomeImg->data, GL_BGR, GL_UNSIGNED_BYTE) ;welcomeVideoChanged = false;};
			welcomeVideoMutex.unlock();

			welcome_video.Activate();
			glColor3f(1.0, 1.0, 1.0);
			text_welcome_Video.RenderToViewportFlipY();
			pangolin::FinishFrame();
			continue;
		}
		
		rsVideoMutex.lock();
		if (rsVideoChanged) {text_rs_Video.Upload(internalRsImg->data, GL_BGR, GL_UNSIGNED_BYTE); rsVideoChanged = false;}
		rsVideoMutex.unlock();

		rsVideoMutex.lock();
		if (kinectVideoChanged) {text_kinect_Video.Upload(internalKinectImg->data, GL_BGR, GL_UNSIGNED_BYTE); kinectVideoChanged = false;}
		rsVideoMutex.unlock();

		//d_cam.Activate(s_cam);

		if(showKinect){
			kinect_video.Activate();
			glColor3f(1.0, 1.0, 1.0);
			text_kinect_Video.RenderToViewportFlipY();
		}

		if(showRs){
			rs_video.Activate();
			glColor3f(1.0, 1.0, 1.0);
			text_rs_Video.RenderToViewportFlipY();
		}

		if (record.GuiChanged()) {
			if (!isRecord) {
				isRecord = true;
				recordState = "recording";
			}
		}
		if (saved.GuiChanged()) {
			if (isRecord) {
				isRecord = false;
				recordState = "waiting";
				isSaved = true;
			}
		}

		if (prev_Action.GuiChanged() && (!isRecord)) {
			if (ActionID > 0) {
				ActionID--;
				settingActionID--;
				currentAction = settingActionID;
			}
		}

		if (next_Action.GuiChanged() && (!isRecord)) {
			ActionID++;
			settingActionID++;
			currentAction = settingActionID;
		}

		if (prev_People.GuiChanged() && (!isRecord)) {
			if(settingPeopleID>0){
				PeopleID--;
				settingPeopleID--;
				currentPeople = settingPeopleID;
			}
		}

		if (next_People.GuiChanged() && (!isRecord)) {
			PeopleID++;
			settingPeopleID++;
			currentPeople = settingPeopleID;
			if (settingActionID >= MAX_ActionID) {
				ActionID = 0;
				settingActionID = 0;
				currentAction = settingActionID;
			}

		}

		if (cancel.GuiChanged()) {
			if (isRecord) {
				isRecord = false;
				recordState = "waiting";
			}
		}


		currentSize = QueueSize;
		lastRecordSize = LastRecordSize;
		lastRecordFile = LastRecordFile;

		pangolin::FinishFrame();
	}

}


void Viewer::setEndWelcome()
{
	showWelcome = false;
}

void Viewer::publishWelcome(cv::Mat frame)
{
	welcomeVideoMutex.lock();
	memcpy(internalWelcomeImg->data, frame.data, 1920 * 1080 * sizeof(cv::Vec3b));
	welcomeVideoChanged = true;
	welcomeVideoMutex.unlock();
}


void Viewer::publishRsFrame(std::shared_ptr<cv::Mat> frame)
{
	rsVideoMutex.lock();
	memcpy(internalRsImg->data, frame->data, rs_w * rs_h * sizeof(cv::Vec3b));
	rsVideoChanged = true;
	rsVideoMutex.unlock();
}

void Viewer::publishKinectFrame(cv::Mat frame)
{
	kinectVideoMutex.lock();
	memcpy(internalKinectImg->data, frame.data, kinect_w * kinect_h * sizeof(cv::Vec3b));
	kinectVideoChanged = true;
	kinectVideoMutex.unlock();
}