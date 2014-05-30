#include <Scheduler.hpp>
#include <iostream>
#include <boost/make_shared.hpp>
#include <cstring>
#include <boost/chrono.hpp>
#include <unistd.h>
#define MODE_MANAGED 4
using namespace std;
//Removes resources allocated to a job from the res pool....
bool resPtrEQ(const Resource& lhs, const std::unique_ptr<Resource>& rhs) {
  return lhs.getId() == rhs->getId();
}
void Scheduler::claimResources(JobResPair &elem) {
  using namespace std::placeholders;
  std::cout << "Scheduler::claimResources():\t";
  std::cout << "resPool[" << resPool->size() << "]\t";

  ResourceList& r = std::get<1>(elem);
  for (auto rit = r.begin(); rit != r.end(); rit++) {
    auto remove = std::find_if(resPool->begin(), resPool->end(),std::bind(resPtrEQ,*rit,std::placeholders::_1));
    resPool->erase(remove);
  }
//  auto it = resPool->begin();
//  ResourcePoolPtr preserveList = ResourcePoolPtr(new ResourcePool());

 /* for (auto it = resPool->begin(); it != resPool->end(); it++) {
    ResourceList::iterator rit = r.begin();
    for (;rit != r.end(); rit++) {
      if ( **it != *rit) {
        std::cout << "Got: " << **it;
//        preserveList->push_back(std::move(*it));
        it = resPool->erase(it);
      }
    }
  }
//  resPool = preserveList;*/
  std::cout << "resPool[" << resPool->size() << "]\n";
}

void Scheduler::returnResources(ResourceList& res) {
  std::cout << "Scheduler::returnResources(): ";
  std::cout << "\tresPool[" << resPool->size() << "]\n";

  std::cout << "{ ";
  ResourceList::iterator r = res.begin();
  for (; r != res.end(); r++) {
    std::cout << *r << ",";
    addResource(*r);
  }
  std::cout << "}\n";
  std::cout << "resPool[" << resPool->size() << "]\n";
}

//TODO[mtottenh] Finish implementing the rest of the scheduler class
void Scheduler::defaultHandler(msg_t& request,
                               msg_t& response,
                               int responseSize) {
  /*
  resPool->front()->send(&request);
  // wait for reply back
  char buffer[responseSize];
  memset(buffer, 0, responseSize);
  int n = resPool->front()->read(buffer, responseSize);

  cout << "Default handler " << endl;
  request.print();
  response.print();

  msg_t* rsp = (msg_t*)buffer;
  rsp->print();
  response.msgId = rsp->msgId;
  response.dataSize = rsp->dataSize;
  response.paramsSize = 0;
  memcpy(response.data, rsp->data, rsp->dataBytes());

  cout << "DefHandler Response" << endl;
  response.print();*/

  cout << "Default Handler: Not implemented\n" << responseSize;
#ifdef DEBUG
  request.print();
  response.print();
#endif
}

JobPtr Scheduler::estimateFinishTime(JobPtr j) {
  //TODO[mtottenh]:Add code to estimate a job finish time
  std::cout << "Scheduler::estimateFinishTime() : Not implemented\n";
  return j;
}


/*
 * TODO[mtottenh]: Check implementations of getDispatchTime/IssueTime
 *  with timevals in the minites/seconds range not just random
 *  floats.. Also add Q locks
 */

int Scheduler::numLateJobs() {
  int sum = 0;
  JobQueue::iterator it = finishedQ->begin();
  for(;it != finishedQ->end(); it++) {
    boost::chrono::system_clock::time_point disp_t = (std::get<0>(**it))->getDispatchTime();
    boost::chrono::system_clock::time_point issue_t = (std::get<0>(**it))->getIssueTime();
    sum = (disp_t - issue_t)  > boost::chrono::seconds(1) ? sum+1 : sum;
  }
  return sum;
}

void Scheduler::updateState() {

}

void Scheduler::dumpInfo() {

}

void Scheduler::notifyClientsOfResults() {
//  std::cout << "NotifyClients Of Results: Needs implementing\n";
  auto j = finishedQ->front();
  finishedQ->pop_front();
  std::cout << "Mean waiting time: " << meanWaitTime << "\n";
  std::get<2>(*j).notify_all();
}

void Scheduler::printQInfo(const char*, JobQueuePtr, bool) {

}
//TODO[mtottenh]: Add this to the header file. This could be
// dangerous.. it seems to remove all the resources in the runQ and
// return them to the pool... use with extreme caution
void Scheduler::reclaimResources() {
  cout << "Scheduler::reclaimResources(): Not Implemented\n";
  /*
  auto j = runQ->begin();
  for (;j != runQ->end(); j++) {
    auto a = (std::get<0>(**j))->getAllocdRes()->begin();
    for (;a != (std::get<0>(**j))->getAllocdRes()->end(); a++) {
      resPool->push_back(   move(*a)   );
    }
  }
*/
}

//TODO[mtottenh]: Collapse the below back into 1 function.
// needless code duplication here.
Allocations* Scheduler::schedule(size_t choice, bool flag) {

  flag = false;
  Allocations *a = nullptr;
  if (resPool->size() > 0) {
    a = this->algVec[choice](*this);
  }
 return a;
}

ResourceList Scheduler::allocate(Job &j, size_t max_res, size_t min_res) {
  ResourceList allocatedResources;

  if (resPool->size() >= min_res) {
    while (max_res-- > 0 && resPool->size() > 0) {
      allocatedResources.push_back(*resPool->back());
      resPool->pop_back();
    }
#ifdef DEBUG
    std::cout << "Allocated " << allocatedResources.size()
              << " Resources to " << j << "\n";
#endif
  }
  return allocatedResources;
}

//TODO[mtottenh]:Check this functions as intended.
//I'm not sure about getAllocdRes()->***/
JobPtr Scheduler::deallocate(JobPtr j) {
  std::cout << "De-allocating (NOT IMPLEMENTED) " << *j << "\n";
/*            << "{" << j->noAllocdRes() << "}"
            << "|" << resPool->size() << "|";
  auto r = j->getAllocdRes()->begin();
  for (; r != j->getAllocdRes()->end(); r++) {
    resPool->push_back( move(*r ));
  }
  j->getAllocdRes()->clear();
  std::cout << "(" << resPool->size() << ")\n";*/
  return j;
}


//TODO[mtottenh]: Remove this
msg_t Scheduler::getJobResponse(int jobID) {
  std::cout << "Scheduler::getJobResponse: Not Implmented" << jobID << "\n";
  msg_t rsp;
  return rsp;
}

/* Server Handling */
msg_t* Scheduler::handle_request(msg_t* request) {
#ifdef DEBUG
  cout << "Scheduler recieved request msgID[" <<  request->msgId << "]" << endl;
  request->print();
#endif
  //TODO: Lookup requestID/Implementation ID in a map and return error if not
  //found
  msg_t* response;
  response = msg_ack();
  switch(request->msgId) {
    case MSG_DONE:
      return msg_ack();
    case MSG_MOVING_AVG:
    default:
      // XXX TODO[paul-g]: need to actually determine data size here
      cout << "Request data size " << request->dataSize << endl;
      unsigned long sizeBytes = sizeof(msg_t) + sizeof(int) * request->dataSize;
      response = (msg_t*)calloc(sizeBytes, 1);
      concurrentHandler(*request, *response, sizeBytes);
#ifdef DEBUG
      response->print();
      cout << "Returning from Scheduler::handle_request()\n";
#endif
      return response;
  }
}


void Scheduler::schedLoop() {
  try {
    while (true) {
      boost::unique_lock<boost::mutex> lock(qMutex);
      //If we have no results or nothing to try and schedule wait
      while ( !QStatus.getReadyQStatus() && !QStatus.getFinishedQStatus() ) {
        QCondVar.wait(lock);
      //  std::cout << "Scheduler Thread woke up\n";
      }
      lock.unlock();
   
      //A job was deposited in the readyQ
      if (QStatus.getReadyQStatus() == true) {
        QStatus.setReadyQStatus(false);
        auto start = boost::chrono::system_clock::now();

        std::cout << "Event on readyQ\n";
        boost::unique_lock<boost::mutex> rqLk(readyQMtx);                  
        Allocations* a = schedule(MODE_MANAGED,true);
        rqLk.unlock();
        if (a == nullptr) {
          /* No free resources.. Just block for some more time :)   */
//          std::cout << "Allocation returned nullptr, something horible happened\n";
//
        } else {
          /* managed to get some kind of schedule. */
          size_t numJobsScheduled = a->noJobs();
          if ( numJobsScheduled > 0) {
            std::cout << numJobsScheduled <<" Jobs Scheduled to run.\n";

            a->serviceAllocations(*this);

            QStatus.setRunQStatus(true);

//            if (readyQ->size() == 0)
         //TODO[mtottenh]: fix this leak   delete a;
            auto end = boost::chrono::system_clock::now();
            auto duration = boost::chrono::duration_cast<boost::chrono::microseconds>(end - start);
            std::cout << "Scheduling took: " << duration.count() << "us" << std::endl;
            //end timer
            QCondVar.notify_all();
          } else {
            std::cout << "Could not allocate any resources...\n";
          }
        }
      }

      //A result was returned on the finishedQ -- This could be delegated to
      //a different thread
      if (QStatus.getFinishedQStatus() == true) {
        std::cout << "Event on finishedQ\n";
        while (finishedQ->size() > 0) {
          notifyClientsOfResults();
        }
        QStatus.setFinishedQStatus(false);
      }
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    this->stop();
  }
}
void Scheduler::runJobs() {
    std::cout << "Scheduler::runJobs()\n";
    JobResPairQ::iterator it = runQ->begin();
    for (;it != runQ->end(); it++) {
      JobTuplePtr jobTuplePtr = std::get<0>(*it);
      if (! std::get<1>(*jobTuplePtr).isStarted()){
        jobThreads.create_thread(boost::bind(&Scheduler::runJob,this,*it));
        
      }
    }

   /* 
    * it = runQ->begin();
    JobResPairQPtr preserve_list(new JobResPairQ());
    for (;it != runQ->end(); it++) {
      //for each job in the runQ check if it is finished.
      JobTuplePtr jobTuplePtr = std::get<0>(*it);
      if (!std::get<1>(*jobTuplePtr).isFinished()){
        //remove from runQ.
        preserve_list->push_back(*it);
      } else {
        std::get<2>(*jobTuplePtr).notify_all();
      }
    }
    runQ = preserve_list;
    */
}
void Scheduler::runJob(JobResPair& j) {
  std::cout << "Scheduler::runJob()\n";
  ResourceList resourceList = std::get<1>(j);
  JobTuplePtr jobTuplePtr = std::get<0>(j);
  JobPtr jobPtr = std::get<0>(*jobTuplePtr);

 //TODO[mtottenh]: How do we invoke a job on more than one DFE? :O
  Resource r = resourceList.front();
  const string& name = r.getName().c_str();
  int portNumber = r.getPort();
  Client c(portNumber,name);
  c.start();
  msg_t& req = jobPtr->getReq();

#ifdef DEBUG
  req.print();
#endif

  c.send(&req);

  int sizeBytes = sizeof(msg_t) + sizeof(int) * req.dataSize;
  char* buff = (char *)calloc(sizeBytes, 1);

  if (buff == NULL) {
    std::cout << "ERROR: Unable to allocate result buffer\n";
    c.stop();
    return;
  }

  msg_t* rsp = (msg_t*)buff;

  do {
    c.read(buff,sizeBytes);
#ifdef DEBUG
    rsp->print();
#endif
  }  while ( rsp->msgId != MSG_RESULT);
  c.stop();

//  std::cout << "client::read() finished\n";

#ifdef DEBUG
  rsp->print();
#endif
  
  jobPtr->setRsp(rsp);
  returnResources(resourceList);
//  std::get<0>(*jobTuplePtr)->setFinishTime(boost::chrono::system_clock::now());
  
  updateMeanWaitTime(std::get<0>(*jobTuplePtr));


  //LOCK runQ
  boost::unique_lock<boost::mutex> lk(runQMtx);
//  lk.lock();
  auto it = runQ->begin();
  while (it != runQ->end() &&  std::get<0>(*std::get<0>(*it))->getId() != jobPtr->getId()) {
   it++; 
  }
  runQ->erase(it);
  std::get<1>(*jobTuplePtr).setFinished(true);
  lk.unlock();
  //UNLOCK runQ
  enqueue(finishedQ,jobTuplePtr,finishedQMtx,"finishedQ");
}

void Scheduler::dispatcherLoop() {
  try {
    while (true) {
      //wait on readyQ or FinishedQ condvar
      boost::unique_lock<boost::mutex> lock(qMutex);
      //TODO[mtottenh]: change finishedQStatus to somethign that indicates a dfe
      //has a result. Might not be able to have this as a condvar wait
      //situation....
      while ( !QStatus.getRunQStatus() ) {
        QCondVar.wait(lock);
        std::cout << "Dispatcher thread woke up\n";
      }
      lock.unlock();
      if (QStatus.getRunQStatus()) {
        std::cout << "Event on Run Q\n";
//        runQ contains [" << runQ->size()
//                  << "] Jobs \n";
        runJobs();
        QStatus.setRunQStatus(false);
      }
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    this->stop();
  }
}

msg_t*  Scheduler::concurrentHandler( msg_t &request,
                                      msg_t &response,
                                      unsigned long sizeBytes) {
  //creat (job,condvar) pair
  //
  boost::condition_variable jCondVar;
  boost::mutex jobMutex;
  boost::unique_lock<boost::mutex> lock(jobMutex);
  struct JobInfo jInfo;

  jInfo.setFinished(false);
  jInfo.setStarted(false);
  std::cout << "Scheduler::concurrentHandler()\n";

#ifdef DEBUG
  request.print();
#endif
  JobTuple t = std::make_tuple( new Job(request,getNextId()),
                                std::ref(jInfo),std::ref(jCondVar));
  enqueue(readyQ, &t, readyQMtx,"readyQ");


  while (!jInfo.isFinished()) {
    jCondVar.wait(lock);
  }
  std::cout << "Job finished\n" << std::endl;
  msg_t* rsp = std::get<0>(t)->getRsp();
  response.msgId = rsp->msgId;
  response.dataSize = rsp->dataSize;
  response.paramsSize = 0;
//  size_t dataBytes  = rsp->dataSize*sizeof(int);
  memcpy(&response.data,rsp->data ,rsp->dataBytes());

#ifdef DEBUG
  response.print();
#endif
//  std::cout << "returning from concurrentHandler\n";
  //dequeue(finishedQ,(job,condvar));
  return &response;
}




void Scheduler::start() {
  //Fire up the Scheduling and dispatch threads.
  QStatus.setReadyQStatus(false);
  QStatus.setRunQStatus(false);
  QStatus.setFinishedQStatus(false);

  schedulerThread = new boost::thread(&Scheduler::schedLoop, this);
  dispatcherThread = new boost::thread(&Scheduler::dispatcherLoop, this);
  run();
//  schedulerThread->join();
//  dispatcherThread->join();
}
