#include "MultiThreadedTCPServer.hpp"
#include <boost/bind.hpp>
#include <signal.h>
#include <vector>
#include <Logging.hpp>

#include "message.hpp"

using namespace std;
namespace ba = boost::asio;

MultiThreadedTCPServer::~MultiThreadedTCPServer() {
  LOG(debug)<< "Deconstructed";
}

MultiThreadedTCPServer::MultiThreadedTCPServer(const string &address,
                                               const string &port,
                                               size_t thread_pool_size)
    : thread_pool_size_(thread_pool_size), signals_(io_service_),
      acceptor_(io_service_), new_connection_() {
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
  signals_.async_wait(boost::bind(&MultiThreadedTCPServer::handle_stop, this));

  namespace bip = boost::asio::ip;
  bip::tcp::resolver resolver(io_service_);
  bip::tcp::resolver::query query(address, port);
  bip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(bip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();
  start_accept();
}

void MultiThreadedTCPServer::run() {
  LOG(debug) << "Creating thread pool size " << thread_pool_size_;
  for (size_t i = 0; i < thread_pool_size_; ++i) {
    worker_threads.create_thread(
        boost::bind(&boost::asio::io_service::run, &io_service_));
  }
  LOG(debug) << "Calling join_all()";
  worker_threads.join_all();
}

void MultiThreadedTCPServer::start_accept() {
  LOG(debug) << "Start accept";
  new_connection_.reset(new connection(io_service_, *this));
  acceptor_.async_accept(
      new_connection_->socket(),
      boost::bind(&MultiThreadedTCPServer::handle_accept, this));
}

void MultiThreadedTCPServer::handle_accept() {
  LOG(debug) << "handle_accept";
  new_connection_->start();
  start_accept();
}

void MultiThreadedTCPServer::handle_stop() {
  LOG(debug) << "Stoping TCP Server";
  io_service_.stop();
  LOG(debug) << "interrupting worker_threads";
  worker_threads.interrupt_all();
}

connection::connection(ba::io_service &io_service,
                       MultiThreadedTCPServer &server)
    : strand_(io_service), socket_(io_service), server_(server) {}

ba::ip::tcp::socket &connection::socket() { return socket_; }

void connection::start() {
  // XXX This doesn't wait for a specific number of bytes; a simple
  // solution is to wait for sizeof(msg_t) so that we are sure we read
  // all the fields of the struct correctly
  socket_.async_read_some(
      ba::buffer(buffer_),
      strand_.wrap(boost::bind(&connection::handle_read, shared_from_this(),
                               ba::placeholders::error,
                               ba::placeholders::bytes_transferred)));
}

void connection::handle_read(const boost::system::error_code &e,
                             std::size_t bytes_transferred) {
  LOGF(debug, " Bytes transferred %1%") % bytes_transferred;

  // XXX TODO[paul-g]: Need to do this async
  msg_t *request = NULL;
  msg_t *reply = NULL;
  do {
    // unpack request
    request = (msg_t *)buffer_.data();

    // first check we read all data; we have to do this since we don't
    // know the size a priori and read_some does not guarantee to read
    // all bytes before it returns. NOTE that we rely on reading at
    // least the first few bytes storing the msg size (a bit of a hack
    // for now)

    int expectedBytes = request->totalBytes;

    msg_t *fullRequest;
    bool resized = false;
    if (expectedBytes > bytes_transferred) {
      int size = expectedBytes - bytes_transferred;
      fullRequest = (msg_t *)malloc(expectedBytes);
      memcpy((char *)fullRequest, (char *)request, bytes_transferred);
      char *buff = (char *)calloc(size, 1);
      ba::read(socket_, ba::buffer(buff, size));
      memcpy(((char *)fullRequest) + bytes_transferred, buff, size);
      free(buff);
      resized = true;
    } else {
      fullRequest = request;
    }

    // do work and generate reply
    reply = server_.handle_request(fullRequest);

    if (resized)
      free(fullRequest);

    // write reply back
    if (reply != NULL) {
      try {
	LOGF(info, "Writing reply %1% bytes ") % reply->totalBytes;
        ba::write(socket_, ba::buffer((char *)reply, reply->totalBytes));
        socket_.read_some(ba::buffer(buffer_));
      }
      catch (std::exception &e) {
	LOGF(error, " %1% ") % e.what();
      }
    }
  } while (request != NULL && request->msgId != MSG_DONE &&
           request->msgId != MSG_TERM);
  if (reply != NULL)
    free(reply);
  if (request->msgId == MSG_TERM) {
    LOG(debug) << "Terminating (msgId == MSG_TERM)";
    raise(SIGINT);
  }
  boost::system::error_code ignored_ec;
  socket_.shutdown(ba::ip::tcp::socket::shutdown_both, ignored_ec);
}

void connection::handle_write(const boost::system::error_code &e) {
  if (!e) {
    boost::system::error_code ignored_ec;
    socket_.shutdown(ba::ip::tcp::socket::shutdown_both, ignored_ec);
  }
}
