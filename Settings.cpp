#include "Settings.h"


volatile int ActionID = 0;
volatile int PeopleID = 0;
volatile int SequenceID = 0;
std::string ViewID = "";
int LastRecordSize = 0; 
std::string LastRecordFile = "";

int QueueSize = 0;

volatile bool isRecord = false;
volatile bool isSaved = false;

int MIN_ActionID = 0;
int MAX_ActionID = 1000;