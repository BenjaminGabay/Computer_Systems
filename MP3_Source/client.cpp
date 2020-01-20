/* 
    File: client.cpp

    Author: Benjamin Gabay
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : 10/27/2019

    Modified:
    Client main process for Machine Problem 3. Also, forks off
    a process to run the dataserver.
*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "PCBuffer.hpp"
#include "reqchannel.hpp"

using namespace std;

//Request Thread Arguments
struct RTArg
{
    PCBuffer* wpc;
    string patientName;
    int nReq;
};

//Worker Thread Arguments
struct WTArg
{
    PCBuffer* wpc;
    RequestChannel* chan;
    map<string, PCBuffer*> PCMap;
    Mutex* printing;    //Ensures only one thread is printing to the system at a time
};

//Statistic Thread Arguments
struct STArg
{
    PCBuffer* pcb;
    Mutex* printing;    //Ensures only one thread is printing to the system at a time
    string name;
    int nReq;
};

//Request Thread Function
void* ReqThFunc(void* addr)
{
    RTArg* args = (RTArg*) addr;
    for(int i = 0; i < args->nReq; i++)
    {
        args->wpc->Produce("data " + args->patientName);
    }
}

//Worker Thread Function
void* WThFunc(void* addr)
{
    WTArg* args = (WTArg*) addr;
    string req = args->wpc->Consume();
    while(req.compare("done") != 0)
    {
        args->printing->Lock();
        string res = args->chan->send_request(req);
        args->printing->Unlock();
        string name = req.substr(5, req.length() - 5);
        args->PCMap[name]->Produce(res);
        req = args->wpc->Consume();
    }
    args->wpc->Produce("done"); //Send done signal for other worker threads
}

//Statistic Thread Function
void* STFunc(void* addr)
{
    STArg* args = (STArg*) addr;
    //Fill histogram
    vector<int> hist(10);
    for(int i = 0; i < args->nReq; i++)
    {
        string resString = args->pcb->Consume();
        int res = stoi(resString);
        hist[res/10]++;
    }
    args->printing->Lock();
    //Print histogram
    cout << endl << args->name << endl;
    for(int i = 0; i < 10; i++)
    {
        //print hist
        cout << i << "0-" << i << "9: " << hist[i] << endl;
    }
    cout << endl;
    args->printing->Unlock();
}

int main(int argc, char *argv[])
{
    if(argc != 4) //Valid number of command line arguments
    {
        cout << "USAGE: " << argv[0] << " -n -b -w" << endl;
        cout << "\t-n: <number of data requests per patient>" << endl;
        cout << "\t-b: <size of bounded buffer between request and worker threads>" << endl;
        cout << "\t-w: <number of worker threads>" << endl;
        return 1;
    }

    pid_t pid = fork(); //Fork off second concurrent process

    if(pid == 0) //Child process runs dataserver
		execvp("./dataserver", nullptr);
	else
	{
        struct timeval tp_start, tp_end;
        gettimeofday(&tp_start, NULL); //Start timer for process
        //Assumes command line arguments are the right type
        int numReq = stoi(argv[1]); //1000
        int bbSize = stoi(argv[2]); //50
        int numWT = stoi(argv[3]); //5
        PCBuffer wpc(bbSize);
        //Create each patient request thread
        RTArg p1RTArg;
        p1RTArg.wpc = &wpc;
        p1RTArg.nReq = numReq;
        p1RTArg.patientName = "Joe Smith";
        RTArg p2RTArg;
        p2RTArg.wpc = &wpc;
        p2RTArg.nReq = numReq;
        p2RTArg.patientName = "Jane Smith";
        RTArg p3RTArg;
        p3RTArg.wpc = &wpc;
        p3RTArg.nReq = numReq;
        p3RTArg.patientName = "John Doe";
        pthread_t P1RT;
        pthread_create(&P1RT, NULL, ReqThFunc, (void*) &p1RTArg);
        pthread_t P2RT;
        pthread_create(&P2RT, NULL, ReqThFunc, (void*) &p2RTArg);
        pthread_t P3RT;
        pthread_create(&P3RT, NULL, ReqThFunc, (void*) &p3RTArg);

        //Bounded buffer for each patient stat thread
        PCBuffer s1pc(bbSize);
        PCBuffer s2pc(bbSize);
        PCBuffer s3pc(bbSize);
        map<string, PCBuffer*> stMap;
        stMap[p1RTArg.patientName] = &s1pc;
        stMap[p2RTArg.patientName] = &s2pc;
        stMap[p3RTArg.patientName] = &s3pc;
        
        Mutex m; //Mutex used to only allow one thread to print at a time
        //Create desired number of worker threads
        vector<WTArg> wtArgs(numWT);
        vector<pthread_t> wThreads(numWT);
        vector<RequestChannel*> channels(numWT);
        RequestChannel ctrlChan("control", RequestChannel::CLIENT_SIDE);
        for(int i = 0; i < numWT; i++)
        {
            string chanName = ctrlChan.send_request("newthread");
            channels[i] = new RequestChannel(chanName, RequestChannel::CLIENT_SIDE);
            wtArgs[i].wpc = &wpc;
            wtArgs[i].chan = channels[i];
            wtArgs[i].PCMap = stMap;
            wtArgs[i].printing = &m;
            pthread_create(&wThreads[i], NULL, WThFunc, (void*) &wtArgs[i]);
        }

        //Create statistic threads for each patient
        STArg st1Args;
        st1Args.pcb = &s1pc;
        st1Args.name = p1RTArg.patientName;
        st1Args.nReq = p1RTArg.nReq;
        st1Args.printing = &m;
        STArg st2Args;
        st2Args.pcb = &s2pc;
        st2Args.name = p2RTArg.patientName;
        st2Args.nReq = p2RTArg.nReq;
        st2Args.printing = &m;
        STArg st3Args;
        st3Args.pcb = &s3pc;
        st3Args.name = p3RTArg.patientName;
        st3Args.nReq = p3RTArg.nReq;
        st3Args.printing = &m;
        pthread_t s1th;
        pthread_create(&s1th, NULL, STFunc, (void*) &st1Args);
        pthread_t s2th;
        pthread_create(&s2th, NULL, STFunc, (void*) &st2Args);
        pthread_t s3th;
        pthread_create(&s3th, NULL, STFunc, (void*) &st3Args);

        //Join all threads so the main does not terminate until alll threads terminate
        pthread_join(P1RT, NULL);
        pthread_join(P2RT, NULL);
        pthread_join(P3RT, NULL);
        wpc.Produce("done"); //Send done signal to worker threads once request threads are done
        for(int i = 0; i < numWT; i++)
        {
            pthread_join(wThreads[i], NULL);
        }
        pthread_join(s1th, NULL);
        pthread_join(s2th, NULL);
        pthread_join(s3th, NULL);
        cout << endl;
        m.Lock();
        for(int i = 0; i < numWT; i++)
        {
            channels[i]->send_request("quit");
            delete channels[i];
        }
        ctrlChan.send_request("quit");
        wpc.Consume(); //Consume extra done in buffer
        m.Unlock();

        //Output the time in miliseconds the process took
        gettimeofday(&tp_end, NULL); //End timer for process
        long sec, musec;
        sec = tp_end.tv_sec - tp_start.tv_sec;
        musec = tp_end.tv_usec - tp_start.tv_usec;
        musec += sec*1000000;
        cout << endl << "Time Taken: " << musec << endl << endl;
        
        usleep(1000000);
    }
}