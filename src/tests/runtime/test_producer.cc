/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/runtime/Tool.h"
#include "eckit/runtime/ProducerConsumer.h"
#include "eckit/parser/Translator.h"


using namespace eckit;

class Test : public Tool {
    virtual void run();

public:

    Test(int argc, char** argv): Tool(argc,argv) {}

};


struct C : public Consumer<string> {

    virtual void consume(string& s) {
        Log::info() << "Consume " << s << endl;
        ::sleep(5);
    }

};

struct P : public Producer<string> {

    int count_;

    virtual bool done() {
        return count_ <= 0;
    }

    virtual void produce(string& s) {
        Log::info() << "Produce " << count_ << endl;
        ::sleep(count_);
        ASSERT(count_); s = string("Hello, world! ") + Translator<int,string>()(count_); count_--;
    }

    P() : count_(5) {}
};


void Test::run()
{
    P p;
    C c;
    ProducerConsumer<string> pc;
    pc.execute(p, c);
}

//-----------------------------------------------------------------------------

int main(int argc,char **argv)
{
    Test app(argc,argv);
    app.start();
    return 0;
}