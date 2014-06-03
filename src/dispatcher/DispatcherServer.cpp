#include <DispatcherServer.hpp>

using namespace std;

void DispatcherServer::movingAverage_cpu(size_t n, size_t size, int *data, int *out) {
  cout << "Dispatcher::MovingAverageCPU" << endl;
  cout << " n:    " << n << endl;
  cout << " size: " << size << endl;

  for (size_t i = 0; i <= n - size; i++) {
    out[i] = 0;
    for (size_t j = 0; j < size; j++)
      out[i] += data[i + j];
    out[i] /= size;
  }

  for (size_t i = n - size + 1; i < n; i++) {
    out[i] = 0;
  }
}

void DispatcherServer::movingAverage_dfe(int n, int size, int *data, int *out) {
  cout << "Dispatcher::MovingAverageDFE" << endl;
  char* dfeIds[] = {"1", "2", "3", "4"};
#ifdef USEDFE
  MovingAverageDFE(size, n, data, out, 2, dfeIds, false);
#endif
}

msg_t* DispatcherServer::handle_request(msg_t* request) {
#ifdef DEBUG
  std::cout << "Dispatcher::handle_request" << std::endl;
#endif

  if (request->msgId == MSG_MOVING_AVG) {
    // unpack data
    size_t n = request->dataSize;
    size_t nBytes = sizeof(int) * n;
    int* out = (int *)calloc(n, sizeof(int));
    int* data_in = (int*)malloc(nBytes);
    memcpy(data_in, request->data, nBytes);

    // do computation
    // TODO check resource type
    if (/*request->resourceType == "DFE" &&*/ useDfe) {
      // TODO pass in other arguments (e.g. nDfes, dfeIDs)
      movingAverage_dfe(n, request->firstParam(), data_in, out);
    } else
      movingAverage_cpu(n, (size_t)request->firstParam(), data_in, out);

#ifdef DEBUG
    cout << "Data out " << endl;
    for (int i = 0; i < n; i++)
      cout << out[i] << endl;
#endif

    // write the response
    size_t sizeBytes = sizeof(msg_t) + request->dataSize * sizeof(int);
    msg_t* response = (msg_t*)calloc(sizeBytes, 1);
    response->msgId = MSG_RESULT;
    response->dataSize = n;
    response->paramsSize = 0;
    memcpy(response->data, out, nBytes);
    free(out);
    free(data_in);

    return response;
  }

  return msg_empty();
}
