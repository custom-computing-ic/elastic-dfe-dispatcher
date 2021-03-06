#ifndef _DISPATCHERSERVER_H_
#define _DISPATCHERSERVER_H_

#include <MultiThreadedTCPServer.hpp>
#include <message.hpp>
#include <string>
#include <Logging.hpp>

#ifdef USEDFE
#include <DfeLib.hpp>
#endif

class DispatcherServer : public MultiThreadedTCPServer {

  bool useDfe;

public:

  DispatcherServer(const std::string& address,
                   const std::string& name,
                   std::size_t thread_pool_size) :
    super(address, name, thread_pool_size)
  {
#ifdef USEDFE
    LOG(debug) << "Using DFE implementations";
    useDfe = true;
#else
    LOG(debug) << "Using CPU Only";
    useDfe = false;
#endif
  }

  void movingAverage_cpu(size_t n, size_t size, int *data, int *out);
  void movingAverage_dfe(int n, int size, int *data, int *out);
  void movingAverage_dfe(int n, int size, int *data, int *out,
                         char** dfeIds, int nDfes);
  void optionPricing_dfe(double strike,
			 double sigma,
			 double timestep,
			 int numMaturity,
			 int paraNode,
			 int numPathGroup,
			 double T,
			 double *out,
			 int nDFEs
			 );

  msg_t* handle_request(msg_t* request);
};


#endif /* _DISPATCHERSERVER_H_ */
