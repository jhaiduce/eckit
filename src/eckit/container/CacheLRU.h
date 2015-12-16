/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date   Dec 2015

#ifndef eckit_container_CacheLRU_h
#define eckit_container_CacheLRU_h

#include <iosfwd>
#include <map>
#include <list>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/memory/NonCopyable.h"

namespace eckit {

//----------------------------------------------------------------------------------------------------------------------

template< typename K, typename V >
class CacheLRU : private NonCopyable {

public: // types

    typedef K key_type;
    typedef V value_type;

    struct Entry {

        key_type   key_;
        value_type value_;

        Entry(const key_type& k, const value_type& v) : key_(k), value_(v) {}
    };

    typedef Entry entry_type;

    typedef std::list<entry_type> storage_type;
    typedef typename storage_type::iterator storage_iterator;
    typedef std::map<key_type,storage_iterator> map_type;

public: // methods

    CacheLRU(const size_t& maxSize);

    ~CacheLRU();

    /// Inserts an entry to the cache, overwrites if already exists
    /// @returns true if a key aklready existed
    bool insert(const key_type& key, const value_type& value);

    /// Accesses a key that must already exist
    /// @throws NotFound exception is key not present
    const value_type& access(const key_type& key);

    /// Remove a key-value pair from the cache
    /// No effect if key is not present
    /// @return true if removed
    bool remove(const key_type& key);

    /// @returns true if the key exists in the cache
    bool exists(const key_type& key);

    /// Clears all entries in the cache
    void clear();

    /// @returns the size of the cache
    size_t size() const { return storage_.size(); }

    void print(std::ostream& os) const { os << "CacheLRU(maxSize=" << maxSize_ << ")"; }

    friend std::ostream& operator<<(std::ostream& s,const CacheLRU& p) { p.print(s); return s; }

private: // methods

    void erase(typename map_type::iterator itr) {
        storage_.erase( itr->second );
        map_.erase( itr );
    }

    void trim() {
        while(map_.size() > maxSize_) {
            map_.erase( storage_.back().key_ );
            storage_.pop_back();
        }
    }

    void moveToFront(typename map_type::iterator itr) {
      storage_.splice( storage_.begin(), storage_, itr->second );
    }

private: // members

    storage_type      storage_;

    map_type        map_;

    size_t          maxSize_;

};

//----------------------------------------------------------------------------------------------------------------------

template< typename K, typename V>
CacheLRU<K,V>::CacheLRU(const size_t& maxSize) :
    maxSize_( maxSize )
{
}

template< typename K, typename V>
CacheLRU<K,V>::~CacheLRU()
{
    clear();
}

template< typename K, typename V>
bool CacheLRU<K,V>::insert(const key_type& key, const value_type& value)
{
    bool existed = false;

    typename map_type::iterator itr = map_.find(key);
    if( itr != map_.end() )
    {
        existed = true;

        // remove the key from where it is
        // we'll reinsert it again so it comes on top

        erase(itr);
    }

    storage_.push_front( Entry(key,value) );
    map_[key] = storage_.begin();

    trim();

    return existed;
}

template< typename K, typename V>
const V& CacheLRU<K,V>::access(const key_type& key)
{
    // check first the front() since it is the most popular/recent entry

    if( size() && storage_.front().key_ == key )
        return storage_.front().value_;

    typename map_type::iterator itr = map_.find(key);

    if(itr != map_.end())
    {
        // move entry of list to front
        // this keeps the most popular in front

        moveToFront(itr);

        return itr->second->value_;
    }
    else
    {
        throw eckit::OutOfRange( "key not present in CacheLRU", Here() );
    }
}

template< typename K, typename V>
bool CacheLRU<K,V>::remove(const key_type& key)
{
    bool existed = false;

    typename map_type::iterator itr = map_.find(key);
    if( itr != map_.end() )
    {
        existed = true;
        erase(itr);
    }

    return existed;
}

template< typename K, typename V>
bool CacheLRU<K,V>::exists(const key_type& key)
{
    return ( map_.find(key) != map_.end() );
}

template< typename K, typename V>
void CacheLRU<K,V>::clear()
{
    storage_.clear();
    map_.clear();
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace eckit

#endif
