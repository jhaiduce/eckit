/*
 * (C) Copyright 2020- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef eckit_utils_Rsync_H
#define eckit_utils_Rsync_H

namespace eckit {

class PathName;

class Rsync {

public:  // methods

  Rsync();

  ~Rsync();

  void syncData(const PathName& source, const PathName& target);
  void syncRecursive(const PathName& source, const PathName& target);

private: // members

  size_t block_len_;
  size_t strong_len_;

};

}  // end namespace eckit

#endif