/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/net/TCPClient.h"
#include "eckit/config/Resource.h"
#include "eckit/net/Endpoint.h"


namespace eckit {
namespace net {


TCPClient::TCPClient(int port) : TCPSocket(), port_(port) {}

TCPClient::~TCPClient() {}

void TCPClient::bind() {
    if (socket_ == -1) {
        SocketOptions socketOptions;
        socketOptions.reuseAddr(false);
        socket_ = createSocket(port_, socketOptions);
    }
}

std::string TCPClient::bindingAddress() const {
    return Resource<std::string>("localBindingAddr", ""); /* "127.0.0.1" */
}

void TCPClient::print(std::ostream& s) const {
    s << "TCPClient["
      << "port=" << port_ << ",addr=" << bindingAddress() << ",";
    TCPSocket::print(s);
    s << "]";
}

/// @note TCPClient::connect(host, port, retries, timeout) is found in TCPSocket.cc
TCPSocket& TCPClient::connect(const net::Endpoint& endpoint, int retries, int timeout) {
    return connect(endpoint.hostname(), endpoint.port(), retries, timeout);
}

}  // namespace net
}  // namespace eckit
