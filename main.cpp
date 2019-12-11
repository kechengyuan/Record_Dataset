#include <algorithm>
#include <pangolin/pangolin.h>
#include <iomanip>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <unistd.h>
#include <string>

#include "Viewer.h"
#include "Sensor.h"
#include "Settings.h"

//#pragma comment(lib,"glew32.lib")
using namespace std;

void savedInfo();

void showWelComeImg();

queue<shared_ptr<SavedInfo>> q_info;

mutex saved_mutex;

string base_path = "D:\\Dataset_test\\";


string type[4] = { "00","01","02","03" };


bool parseArgument(char* arg)
{
	int option;
	float foption;
	char buf[1000];

    if(1==sscanf(arg,"ActionIDMin=%d",&option))
    {
        MIN_ActionID = option;
        return true;
    }
	
	if(1==sscanf(arg,"ActionIDMax=%d",&option))
    {
        MAX_ActionID = option;
        return true;
    }

	if(1==sscanf(arg,"ViewID=%d",&option))
    {
        switch (option)
		{
			case 0:
				ViewID = "0";
				break;
			
			case 1:
				ViewID = "1";
				break;

			case 2:
				ViewID = "2";
				break;
			
			default:
				cout<<"Fault view id : "<<option<<endl;
				break;
		}
        return (option>=0 && option<=2);
    }
	cout<<"Fault param"<<endl;
	return false;
}

int main(int argc, char* argv[])
{
	if(argc>1){
		cout<<argc<<endl;
		for(int i =1;i<argc;i++)
			if(!parseArgument(argv[i])){
				
				cout<<"Fault param"<<endl;
				return 0;
			}
	}else{
		cout<<"Usage : recordDataset.exe ViewID=id [ ActionIDMin=MinIndex ActionIDMax=MaxIndex ]"<<endl;
		return 0;
	}
	

	
	Kinect kinect;
	cout<<"Kinect Opened"<<endl;	
	RealSense rs_sensor;
	cout<<"Realsense Opened"<<endl;	

	Viewer viewer(kinect.colorWidth, kinect.colorHeight, rs_sensor.width, rs_sensor.height);
	thread savedThread(savedInfo);

	{
		cv::VideoCapture cap("../media/icv.mp4");

		if(!cap.isOpened())
		{
			viewer.setEndWelcome();
			std::cout<<"could not open the video"<<std::endl;
		}else
		{
			int i=0;
			while(true)
			{
				cv::Mat frame;
				cap>>frame;
				if(frame.empty()){
					usleep(3000000);
					viewer.setEndWelcome();
					break;
				}
				viewer.publishWelcome(frame);
				usleep(80000);
				i++;
					
			}
			cap.release();
		}
	}

	while (true)
	{
		shared_ptr<SavedInfo> sInfo(new SavedInfo);

		while (true)
		{
			shared_ptr<cv::Mat> rs_color, rs_depth;
			shared_ptr<cv::Mat> k_color, k_depth;
			cv::Mat skeleton;
			shared_ptr<Joint_Info> joint(new Joint_Info);

			kinect.getData(k_color, k_depth, skeleton, joint);
			rs_sensor.getFrameSet(rs_color, rs_depth);
			if (isRecord)
			{
				sInfo->kinect_ColorImgs.push_back(k_color);
				sInfo->kinect_DepthImgs.push_back(k_depth);
				sInfo->rs_ColorImgs.push_back(rs_color);
				sInfo->rs_DepthImgs.push_back(rs_depth);
				sInfo->Joints.push_back(joint);
			}

			if (isSaved)
			{
				sInfo->actionId = ActionID;
				sInfo->peopleId = PeopleID;
				LastRecordSize = sInfo->kinect_ColorImgs.size();
				saved_mutex.lock();
				q_info.push(sInfo);
				saved_mutex.unlock();
				isSaved = false;
				isRecord = false;
				break;
			}
			viewer.publishRsFrame(rs_color);
			viewer.publishKinectFrame(skeleton);
		}
		
	}

	return 0;
}

void savedInfo()
{
	cout << "Saved Thead Start" << endl;
	
	while (true) {
		{
			unique_lock<mutex> lck(saved_mutex);
			QueueSize = q_info.size();
		}
		if (QueueSize == 0) {
			usleep(10000);
			continue;
		}
		else {
			shared_ptr<SavedInfo> savedinfo;
			{
				unique_lock<mutex> lck(saved_mutex);
				savedinfo = q_info.front();
				q_info.pop();
			}

			int serialIndex;
			stringstream ssf0;
			char tmp0[200];
			sprintf_s(tmp0, "%sA%02d\\P%02d", base_path.c_str(), savedinfo->actionId, savedinfo->peopleId);
			ssf0 << tmp0;
			string folder_person = ssf0.str();
			if (!(GetFileAttributesA(folder_person.c_str()) & FILE_ATTRIBUTE_DIRECTORY) || GetFileAttributesA(folder_person.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				serialIndex = 0;
			}
			else
			{
				ifstream fin(folder_person + "\\check_serial.txt");
				fin >> serialIndex;
				fin.close();
				serialIndex++;
			}
			stringstream ssf;
			char tmp1[200];
			sprintf_s(tmp1, "A%02d\\P%02d\\S%02d\\V%s\\", savedinfo->actionId, savedinfo->peopleId, serialIndex, ViewID.c_str());
			ssf << base_path << tmp1;
			string folder = ssf.str();
			for (int i = 0; i < 4; i++)
			{
				if (!(GetFileAttributesA((folder + type[i]).c_str()) & FILE_ATTRIBUTE_DIRECTORY) || GetFileAttributesA((folder + type[i]).c_str()) == INVALID_FILE_ATTRIBUTES)
				{
					string cmd = "mkdir " + folder + type[i];
					system(cmd.c_str());
				}
			}
			for (int i = 0; i < savedinfo->kinect_ColorImgs.size(); i++)
			{
				stringstream ssi;
				ssi << setw(5) << setfill('0') << i;
				cv::imwrite(folder + type[0] + "\\" + ssi.str() + ".png", *savedinfo->kinect_ColorImgs[i]);
				cv::imwrite(folder + type[1] + "\\" + ssi.str() + ".png", *savedinfo->kinect_DepthImgs[i]);
				cv::imwrite(folder + type[2] + "\\" + ssi.str() + ".png", *savedinfo->rs_ColorImgs[i]);
				cv::imwrite(folder + type[3] + "\\" + ssi.str() + ".png", *savedinfo->rs_DepthImgs[i]);

			}

			char tmp2[200];
			
			sprintf_s(tmp2, "A%03dP%03dS%03dV%s", savedinfo->actionId, savedinfo->peopleId, serialIndex, ViewID.c_str());
			stringstream ssk;
			ssk << tmp2;
			ofstream fout(folder+ssk.str()+".skeleton");

			LastRecordFile = ssk.str();

			fout << savedinfo->Joints.size() << endl;
			for (int i = 0; i < savedinfo->Joints.size(); i++)
			{
				for (int j = 0; j < savedinfo->Joints[i]->body_num; j++)
				{
					if (savedinfo->Joints[i]->body_num > 0){
						fout << savedinfo->Joints[i]->body_num << " " << savedinfo->Joints[i]->timestamps << endl;
						for (int k = 0; k < savedinfo->Joints[i]->joint_num; k++) {
							fout << savedinfo->Joints[i]->p_worlds[k].x << " " << savedinfo->Joints[i]->p_worlds[k].y << " " << savedinfo->Joints[i]->p_worlds[k].z << " ";
							fout << savedinfo->Joints[i]->p_depths[k].x << " " << savedinfo->Joints[i]->p_depths[k].y << " ";
							fout << savedinfo->Joints[i]->p_colors[k].x << " " << savedinfo->Joints[i]->p_colors[k].y << " ";
							fout << savedinfo->Joints[i]->rotations[k].w << " " << savedinfo->Joints[i]->rotations[k].x << " " << savedinfo->Joints[i]->rotations[k].y << " " << savedinfo->Joints[i]->rotations[k].z << " ";
							fout << savedinfo->Joints[i]->state[k] << endl;
						}
					}
				}
			}
			fout.close();
			
			ofstream fout2(folder_person + "\\check_serial.txt");
			fout2 << serialIndex;
			fout2.close();

		}
	}
	
}
