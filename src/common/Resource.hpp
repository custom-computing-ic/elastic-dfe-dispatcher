#ifndef _RESOURCE_H_
#define _RESOURCE_H_
#include <Client.hpp>
//typedef std::deque<Resource> ResourcePool;


class Resource : public Client {
  public:
    //TODO[mtottenh] : This is so bad.. Every time we copy a resouce
    // we end up invoking a new client connection!
    Resource(Resource const & r) :
      Client::super(r.getPort(),r.getName())
    {
        rid = r.getId();
    }
    Resource(int port_,
             const std::string& name_,
             int id) :
      Client::super(port_, name_)
    {
      rid = id;
    }
    ~Resource() {
    }

    virtual void start();
    virtual void getResult(void *out);
    virtual void stop();
    virtual int getId() const { return rid; }
    virtual int getPort() const {return port;}
    virtual std::string getName() const  {return name;}



  private:
    int rid;
    float utilization;
    float idle_time;
    std::string name;
    int port;
};

#endif
