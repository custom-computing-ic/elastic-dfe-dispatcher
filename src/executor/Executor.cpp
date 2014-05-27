#include <Executor.hpp>
#include <message.hpp>


Executor::~Executor() {
    size_t s = Tasks.size();
    for (size_t i = 0; i < s; i++) {
       Task* tsk = Tasks.front();
       Tasks.pop_front();
       delete tsk;
    }
    //TODO: delete implementaitons for a task
    s = AvailableRes.size();
    for (size_t i = 0; i < s; i++) {
       Resource* res = AvailableRes.front();
       AvailableRes.pop_front();
       delete res;
    }
   //TODO delete performance models for a task.
}


    /* 
     * Manipulate the Resources list
     */
int Executor::SetResources(std::list<Resource*> res) {
  return 0;
}


int Executor::AddResource(Resource *res) {
  return 0;
}

int Executor::DelResource(Resource *res) {
  return 0;
}

/* 
 * Manipulate the Task list
 */
int Executor::SetTasks(std::list<Task*> tsk) {
  this->Tasks = tsk;
  return 0;
}

int Executor::AddTask(Task *tsk) {
  this->Tasks.push_back(tsk);
  return 0;
}

int Executor::DelTask(Task *tsk) {
  this->Tasks.remove(tsk);
  delete(tsk);
  return 0;
}

bool task_ptr_eq(Task* t, Task* t2) {
 return *t == *t2;
}

Task* Executor::FindTask(std::string name) {
  using namespace std::placeholders;
  Task *t1 = new Task(name);
  typedef std::list<Task *>::iterator it;
  it found = std::find_if(Tasks.begin(), 
 		                      Tasks.end(), 
                          std::bind(task_ptr_eq,t1,_1)
                       );
	//	          [&] (Task *t) { return *t == *t1; } );
  delete(t1);
  return Tasks.end() != found ? *found : NULL;  
	
}
/* 
 * Add/Remove Implementations
 */
int Executor::AddImp(Task *tsk,Implementation *imp) {
  tsk->AddImp(imp);	
  return 0;
}

int Executor::DelImp(Task *tsk,Implementation *imp) {
  tsk->DelImp(imp);
  return 0;
}

int Executor::CreatePerfomanceModels() {
  return 0;
}

Implementation Executor::SelectImplementation(Task *tsk, void* input){
        /* TODO: Remove this asap. real quick hack to make it compile.
         * This function passes the address of a null pointer
         * D:
         */
  return *((Implementation*)input);
}


std::list<Implementation *> Executor::GetImplementations(Task *tsk)
{
        return tsk->GetImplementations();
}

std::list<Resource *> Executor::GetResources()
{
        return this->AvailableRes;
}

std::list<Task *> Executor::GetTasks()
{
        return this->Tasks;
}

msg_t* Executor::handle_request(msg_t* request) {
  std::cout << "Executor::handle_request()\n";
  if (request->clientId != atoi(cid.c_str())) {
    std::cout << "Error: Got a request from incorrect clientId[" 
              << request->clientId << "]\n";
      return msg_ack();
  }
  request->print();
  switch (request->msgId) {
    case MSG_DONE:
    case MSG_ACK:
    case MSG_MOVING_AVG:
    default:
      //For the moment just act as a proxy.
      int sizeBytes = sizeof(msg_t) + sizeof(int) * (request->dataSize);
      char* buff = (char *)calloc(sizeBytes, 1);
      msg_t* rsp = (msg_t *) buff;
      std::cout << "Sending request to client\n";
      Client c(schedPort,schedName);
      c.start();
      c.send(request);

      std::cout << "Getting result\n";
      do {
        c.read(buff,sizeBytes);
        rsp->print();
      } while ( rsp->msgId != MSG_RESULT);
      std::cout << "Shutting down\n";
      c.stop();
      return rsp;
  }

  //Just return a nullPtr for now so we segfault :D
  return (msg_t*)NULL;
}
