#pragma once
#include <mutex>
#include <atomic>
#include <string>

extern volatile int ActionID;
extern volatile int PeopleID;
extern volatile int SequenceID;
extern std::string ViewID;
extern int LastRecordSize; 
extern int QueueSize;
extern std::string LastRecordFile;

extern volatile bool isRecord;
extern volatile bool isSaved;

extern int MIN_ActionID;
extern int MAX_ActionID;

struct P3D{
	float x;
	float y;
	float z;
	P3D(float x_,float y_,float z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}

    P3D(){}
};
