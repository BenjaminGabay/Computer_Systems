/* 
    File: client.cpp

    Author: Benjamin Gabay
            Department of Computer Science and Engineering
            Texas A&M University
    Date  : 11/14/2019

    Modified:
    Client main process for Machine Problem 4. Also, forks off
    a process to run the dataserver.
*/

#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "PCBuffer.hpp"
#include "reqchannel.hpp"

using namespace std;

//Request Thread Arguments
struct RTArg
{
    PCBuffer *wpc;
    string patientName;
    int nReq;
};

//Event Hnadler Argumants
struct EHTArg
{
    PCBuffer *wpc;
    vector<RequestChannel *> chans;
    map<string, PCBuffer *> PCMap;
    Mutex *printing; //Ensures only one thread is printing to the system at a time
};

//Statistic Thread Arguments
struct STArg
{
    PCBuffer *pcb;
    Mutex *printing; //Ensures only one thread is printing to the system at a time
    string name;
    int nReq;
};

//Request Thread Function
void *ReqThFunc(void *addr)
{
    RTArg *args = (RTArg *)addr;
    for (int i = 0; i < args->nReq; i++)
    {
        args->wpc->Produce("data " + args->patientName);
    }
}

//Event Handler Thread Function
void *EHThFunc(void *addr)
{
    EHTArg *args = (EHTArg *)addr;
    //args->printing->Lock();
    fd_set readset;
    vector<string> reqs(args->chans.size());
    int numDone = 0;
    for (int i = 0; i < args->chans.size(); i++)
    {
        reqs[i] = args->wpc->Consume();
        if (reqs[i].compare("done") == 0)
        {
            args->wpc->Produce("done");
            numDone++;
        }
        else
        {
            args->chans[i]->cwrite(reqs[i]);
        }
    }
    while (numDone < args->chans.size())
    {
        //zero out readset
        FD_ZERO(&readset);
        //populate readset
        int max = 0;
        for (int i = 0; i < args->chans.size(); i++)
        {
            FD_SET(args->chans[i]->read_fd(), &readset);
            if (args->chans[i]->read_fd() > max)
                max = args->chans[i]->read_fd();
        }
        //select blocking call
        int n = select(max + 1, &readset, nullptr, nullptr, nullptr);

        //read and write to available channels
        for (int i = 0; i < args->chans.size(); i++)
        {
            if (FD_ISSET(args->chans[i]->read_fd(), &readset))
            {
                if (reqs[i].compare("done") != 0)
                {
                    string res = args->chans[i]->cread();
                    string name = reqs[i].substr(5, reqs[i].length() - 5);
                    args->PCMap[name]->Produce(res);
                    reqs[i] = args->wpc->Consume();
                    if (reqs[i].compare("done") == 0)
                    {
                        args->wpc->Produce("done");
                        numDone++;
                    }
                    else
                    {
                        args->chans[i]->cwrite(reqs[i]);
                    }
                }
            }
        }
    }
    //args->printing->Unlock();
}

//Statistic Thread Function
void *STFunc(void *addr)
{
    STArg *args = (STArg *)addr;
    //Fill histogram
    vector<int> hist(10);
    for (int i = 0; i < args->nReq; i++)
    {
        string resString = args->pcb->Consume();
        int res = stoi(resString);
        hist[res / 10]++;
    }

    args->printing->Lock();
    //Print histogram
    std::cout << endl << args->name << endl;
    for (int i = 0; i < 10; i++)
    {
        //print hist
        std::cout << i << "0-" << i << "9: " << hist[i] << endl;
    }
    args->printing->Unlock();
}

int main(int argc, char *argv[])
{
    if (argc != 6) //Valid number of command line arguments
    {
        std::cout << "USAGE: " << argv[0] << " -n -b -w" << endl;
        std::cout << "\t-n: <number of data requests per person>" << endl;
        std::cout << "\t-b: <size of bounded buffer in requests>" << endl;
        std::cout << "\t-w: <number of request channels>" << endl;
        std::cout << "\t-h: <name of server host>" << endl;
        std::cout << "\t-p: <port number of server host>" << endl;
        return 1;
    }

    pid_t pid = fork(); //Fork off second concurrent process

    if (pid == 0) //Child process runs dataserver
        execvp("./dataserver", nullptr);
    else
    {
        struct timeval tp_start, tp_end;
        gettimeofday(&tp_start, NULL); //Start timer for process
        //Assumes command line arguments are the right type
        int numReq = stoi(argv[1]); //10000
        int bbSize = stoi(argv[2]); //50
        int numRC = stoi(argv[3]);  //2, 10, 20, 50
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
        pthread_create(&P1RT, NULL, ReqThFunc, (void *)&p1RTArg);
        pthread_t P2RT;
        pthread_create(&P2RT, NULL, ReqThFunc, (void *)&p2RTArg);
        pthread_t P3RT;
        pthread_create(&P3RT, NULL, ReqThFunc, (void *)&p3RTArg);

        //Bounded buffer for each patient stat thread
        PCBuffer s1pc(bbSize);
        PCBuffer s2pc(bbSize);
        PCBuffer s3pc(bbSize);
        map<string, PCBuffer *> stMap;
        stMap[p1RTArg.patientName] = &s1pc;
        stMap[p2RTArg.patientName] = &s2pc;
        stMap[p3RTArg.patientName] = &s3pc;

        Mutex m; //Mutex used to only allow one thread to print at a time
        RequestChannel ctrlChan("control", RequestChannel::Side::CLIENT);
        vector<RequestChannel *> channels(numRC);
        for (int i = 0; i < numRC; i++)
        {
            string chanName = ctrlChan.send_request("newthread");
            channels[i] = new RequestChannel(chanName, RequestChannel::Side::CLIENT);
        }
        EHTArg ehArgs;
        ehArgs.wpc = &wpc;
        ehArgs.chans = channels;
        ehArgs.PCMap = stMap;
        ehArgs.printing = &m;
        pthread_t ehThread;
        pthread_create(&ehThread, NULL, EHThFunc, (void *)&ehArgs);

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
        pthread_create(&s1th, NULL, STFunc, (void *)&st1Args);
        pthread_t s2th;
        pthread_create(&s2th, NULL, STFunc, (void *)&st2Args);
        pthread_t s3th;
        pthread_create(&s3th, NULL, STFunc, (void *)&st3Args);

        //Join all threads so the main does not terminate until alll threads terminate
        pthread_join(P1RT, NULL);
        pthread_join(P2RT, NULL);
        pthread_join(P3RT, NULL);
        wpc.Produce("done"); //Send done signal to worker threads once request threads are done
        pthread_join(ehThread, NULL);
        pthread_join(s1th, NULL);
        pthread_join(s2th, NULL);
        pthread_join(s3th, NULL);
        for (int i = 0; i < numRC; i++)
        {
            channels[i]->send_request("quit");
            delete channels[i];
        }
        ctrlChan.send_request("quit");
        wpc.Consume(); //Consume extra done in buffer

        //Output the time in seconds and microseconds the process took
        gettimeofday(&tp_end, NULL); //End timer for process
        long sec, musec;
        sec = tp_end.tv_sec - tp_start.tv_sec;
        musec = tp_end.tv_usec - tp_start.tv_usec;
        if(musec < 0)
            std::cout << endl << "Time Taken(sec): " << sec-1 << "." << musec+1000000 << endl;
        else
            std::cout << endl << "Time Taken(sec): " << sec << "." << musec << endl;
        musec += sec * 1000000;
        std::cout << "Time Taken(musec): " << musec << endl;

        usleep(1000000);
    }
}