/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/log/Log.h"
#include "eckit/runtime/Monitor.h"
#include "eckit/net/NetService.h"
#include "eckit/net/NetUser.h"
#include "eckit/thread/ThreadControler.h"

//-----------------------------------------------------------------------------

namespace eckit {

//-----------------------------------------------------------------------------

NetService::NetService(int port):
	server_(port)
{
}

NetService::~NetService()
{
}

void NetService::run()
{

	Monitor::show(false);
	Monitor::name(name());
	Monitor::kind(name());

	while(!stopped())
	{
		//Log::info() << "Wait for " << name() << " connection on port "<< port() << endl;
		ThreadControler t(newUser(server_.accept()));
		t.start();
	}
}


//-----------------------------------------------------------------------------

} // namespace eckit
