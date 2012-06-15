/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef eclib_DHSBehaviorLogFile_H
#define eclib_DHSBehaviorLogFile_H

#include "eclib/DHSBehavior.h"

class DHSBehaviorLogFile : public DHSBehavior {
public:

    // -- Contructors

    DHSBehaviorLogFile();

    // -- Destructor

    ~DHSBehaviorLogFile();
    
private:

    virtual DHSLogger* getLogger();

};

#endif // eclib_DHSBehaviorLogFile_h
