#include <Executor.hpp>
#include <message.hpp>
 
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

Task* Executor::FindTask(std::string name) {
  Task *t1 = new Task(name);
  typedef std::list<Task *>::iterator it;
  it found = std::find(Tasks.begin(), 
 		                   Tasks.end(), t1);
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
  request->print();
  switch (request->msgId) {
    case MSG_DONE:
    case MSG_ACK:
    case MSG_MOVING_AVG:
    default:
      //For the moment just act as a proxy.
      char* buff = new char[1024];
      Client c(schedPort,schedName);
      c.start();
      c.send(request);
      c.getResult(buff);
      c.stop();
      return (msg_t*) buff;
  }

  //Just return a nullPtr for now so we segfault :D
  return (msg_t*)NULL;
}
