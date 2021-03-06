#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <MultiThreadedTCPServer.hpp>
#include <string>
#include <boost/lexical_cast.hpp>

#include "DispatcherServer.hpp"

class Dispatcher {

  MultiThreadedTCPServer *server;

public:

  Dispatcher(int port, const std::string &name) {
    server = new DispatcherServer(name, boost::lexical_cast<std::string>(port), 4);
    server->run();
  }

  ~Dispatcher() {
    delete server;
  }

};


#endif /* _DISPATCHER_H_ */
