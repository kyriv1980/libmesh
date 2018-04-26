// The libMesh Finite Element Library.
// Copyright (C) 2002-2018 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#ifndef LIBMESH_REQUEST_H
#define LIBMESH_REQUEST_H

// Parallel includes
#include "libmesh/libmesh_call_mpi.h"
#include "libmesh/post_wait_work.h"
#include "libmesh/status.h"

// libMesh Includes
#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh_logging.h"

// C++ includes
#include <memory>
#include <vector>
#include <utility>

namespace libMesh
{

namespace Parallel
{

#ifdef LIBMESH_HAVE_MPI

//-------------------------------------------------------------------
/**
 * Request object for non-blocking I/O
 */
typedef MPI_Request request;

#else

// This shouldn't actually be needed, but must be
// a unique type for function overloading to work
// properly.
struct request      { /* unsigned int r; */ };
#endif // LIBMESH_HAVE_MPI


//-------------------------------------------------------------------
/**
 * Encapsulates the MPI_Request
 */
class Request
{
public:
  Request ();

  Request (const request & r);

  Request (const Request & other);

  void cleanup();

  Request & operator = (const Request & other);

  Request & operator = (const request & r);

  ~Request ();

  request * get() { return &_request; }

  const request * get() const { return &_request; }

  Status wait ();

  bool test ();

  bool test (status & status);

  void add_prior_request(const Request & req);

  void add_post_wait_work(PostWaitWork * work);

private:
  request _request;

  // Breaking non-blocking sends into multiple requests can require chaining
  // multiple requests into a single Request
  std::unique_ptr<Request> _prior_request;

  // post_wait_work->first is a vector of work to do after a wait
  // finishes; post_wait_work->second is a reference count so that
  // Request objects will behave roughly like a shared_ptr and be
  // usable in STL containers
  std::pair<std::vector <PostWaitWork * >, unsigned int> * post_wait_work;
};

/**
 * Wait for a non-blocking send or receive to finish
 */
inline Status wait (Request & r) { return r.wait(); }

/**
 * Wait for a non-blocking send or receive to finish
 */
inline void wait (std::vector<Request> & r)
{ for (std::size_t i=0; i<r.size(); i++) r[i].wait(); }



// ------------------------------------------------------------
// Request member functions
inline Request::Request () :
#ifdef LIBMESH_HAVE_MPI
  _request(MPI_REQUEST_NULL),
#else
  _request(),
#endif
  post_wait_work(libmesh_nullptr)
{}

inline Request::Request (const request & r) :
  _request(r),
  post_wait_work(libmesh_nullptr)
{}

inline Request::Request (const Request & other) :
  _request(other._request),
  post_wait_work(other.post_wait_work)
{
  if (other._prior_request.get())
    _prior_request = std::unique_ptr<Request>
      (new Request(*other._prior_request.get()));

  // operator= should behave like a shared pointer
  if (post_wait_work)
    post_wait_work->second++;
}

inline void Request::cleanup()
{
  if (post_wait_work)
    {
      // Decrement the use count
      post_wait_work->second--;

      if (!post_wait_work->second)
        {
#ifdef DEBUG
          // If we're done using this request, then we'd better have
          // done the work we waited for
          for (const auto & item : post_wait_work->first)
            libmesh_assert(!item);
#endif
          delete post_wait_work;
          post_wait_work = libmesh_nullptr;
        }
    }
}

inline Request & Request::operator = (const Request & other)
{
  this->cleanup();
  _request = other._request;
  post_wait_work = other.post_wait_work;

  if (other._prior_request.get())
    _prior_request = std::unique_ptr<Request>
      (new Request(*other._prior_request.get()));

  // operator= should behave like a shared pointer
  if (post_wait_work)
    post_wait_work->second++;

  return *this;
}

inline Request & Request::operator = (const request & r)
{
  this->cleanup();
  _request = r;
  post_wait_work = libmesh_nullptr;
  return *this;
}

inline Request::~Request () {
  this->cleanup();
}

inline Status Request::wait ()
{
  LOG_SCOPE("wait()", "Parallel::Request");

  if (_prior_request.get())
    _prior_request->wait();

  Status stat;
#ifdef LIBMESH_HAVE_MPI
  libmesh_call_mpi
    (MPI_Wait (&_request, stat.get()));
#endif
  if (post_wait_work)
    for (auto & item : post_wait_work->first)
      {
        // The user should never try to give us NULL work or try
        // to wait() twice.
        libmesh_assert (item);
        item->run();
        delete item;
        item = libmesh_nullptr;
      }

  return stat;
}

inline bool Request::test ()
{
#ifdef LIBMESH_HAVE_MPI
  int val=0;

  // MPI_STATUS_IGNORE is from MPI-2; using it with some versions of
  // MPICH may cause a crash:
  // https://bugzilla.mcs.anl.gov/globus/show_bug.cgi?id=1798
#if MPI_VERSION > 1
  libmesh_call_mpi
    (MPI_Test (&_request, &val, MPI_STATUS_IGNORE));
#else
  MPI_Status stat;
  libmesh_call_mpi
    (MPI_Test (&_request, &val, &stat));
#endif

  if (val)
    {
      libmesh_assert          (_request == MPI_REQUEST_NULL);
      libmesh_assert_equal_to (val, 1);
    }

  return val;
#else
  return true;
#endif
}

#ifdef LIBMESH_HAVE_MPI
inline bool Request::test (status & stat)
{
  int val=0;

  libmesh_call_mpi
    (MPI_Test (&_request, &val, &stat));

  return val;
}
#else
inline bool Request::test (status &)
{
  return true;
}
#endif

inline void Request::add_prior_request(const Request & req)
{
  // We're making a chain of prior requests, not a tree
  libmesh_assert(!req._prior_request.get());

  Request * new_prior_req = new Request(req);

  // new_prior_req takes ownership of our existing _prior_request
  new_prior_req->_prior_request.reset(this->_prior_request.release());

  // Our _prior_request now manages the new resource we just set up
  this->_prior_request.reset(new_prior_req);
}

inline void Request::add_post_wait_work(PostWaitWork * work)
{
  if (!post_wait_work)
    post_wait_work = new
      std::pair<std::vector <PostWaitWork * >, unsigned int>
      (std::vector <PostWaitWork * >(), 1);
  post_wait_work->first.push_back(work);
}



} // namespace Parallel

} // namespace libMesh

#endif // LIBMESH_REQUEST_H
