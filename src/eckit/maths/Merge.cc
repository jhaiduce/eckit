/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/maths/Merge.h"
#include "eckit/maths/List.h"

//#include <algorithm>

namespace eckit {
namespace maths {

//--------------------------------------------------------------------------------------------

Merge::Merge( ExpPtr list0, ExpPtr list1 ) : Function()
{
    args_.push_back(list0);
    args_.push_back(list1);
}

string Merge::returnSignature() const
{
    return List::sig();
}

ExpPtr Merge::evaluate( Scope &ctx ) const
{
    ExpPtr f = param(0, ctx);

    const List::value_t& l0 = List::extract( param(0, ctx) );
    const List::value_t& l1 = List::extract( param(1, ctx) );

    List::value_t res;
    res.reserve(l0.size() + l1.size());

    for( size_t i = 0; i < l0.size(); ++i )
    {
        ExpPtr e = l0[i];
        res.push_back( e );
    }

    for( size_t i = 0; i < l1.size(); ++i )
    {
        ExpPtr e = l1[i];
        res.push_back( e );
    }

    return ExpPtr(new List( res, List::Swap()));
}

void Merge::asCode(std::ostream&o) const
{
    o << "maths::merge("; printArgs(o); o <<")";
}

ExpPtr Merge::cloneWith(args_t &a) const {
    NOTIMP; // Should not be called
}

// Support for count()
bool Merge::countable() const
{
    return args_[0]->countable() && args_[1]->countable();
}

size_t Merge::count() const
{
    return args_[0]->count() + args_[1]->count();
}

//--------------------------------------------------------------------------------------------

ExpPtr merge(ExpPtr l0, ExpPtr l1 )
{
    return ExpPtr( new Merge(l0,l1) );
}

//--------------------------------------------------------------------------------------------

} // namespace maths
} // namespace eckit
