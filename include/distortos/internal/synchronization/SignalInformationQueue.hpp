/**
 * \file
 * \brief SignalInformationQueue class header
 *
 * \author Copyright (C) 2015 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \date 2015-11-27
 */

#ifndef INCLUDE_DISTORTOS_INTERNAL_SYNCHRONIZATION_SIGNALINFORMATIONQUEUE_HPP_
#define INCLUDE_DISTORTOS_INTERNAL_SYNCHRONIZATION_SIGNALINFORMATIONQUEUE_HPP_

#include "distortos/internal/allocators/PoolAllocator.hpp"
#include "distortos/internal/allocators/FeedablePool.hpp"

#include "distortos/SignalInformation.hpp"

#include <forward_list>
#include <memory>

namespace distortos
{

class SignalSet;

namespace internal
{

/// SignalInformationQueue class can be used for queuing of SignalInformation objects
class SignalInformationQueue
{
public:

	/// link and SignalInformation
	using LinkAndSignalInformation = std::pair<void*, SignalInformation>;

	/// type of uninitialized storage for SignalInformation with link
	using Storage =
			typename std::aligned_storage<sizeof(LinkAndSignalInformation), alignof(LinkAndSignalInformation)>::type;

	/// unique_ptr (with deleter) to Storage[]
	using StorageUniquePointer = std::unique_ptr<Storage[], void(&)(Storage*)>;

	/**
	 * \brief SignalInformationQueue's constructor
	 *
	 * \param [in] storageUniquePointer is a rvalue reference to StorageUniquePointer with storage for queue elements
	 * (sufficiently large for \a maxElements Storage objects) and appropriate deleter
	 * \param [in] maxElements is the number of elements in \a storage array
	 */

	SignalInformationQueue(StorageUniquePointer&& storageUniquePointer, size_t maxElements);

	/**
	 * \brief SignalInformationQueue's destructor
	 */

	~SignalInformationQueue();

	/**
	 * \brief Accepts (dequeues) one of signals that are queued.
	 *
	 * This should be called when the signal is "accepted".
	 *
	 * \param [in] signalNumber is the signal that will be accepted, [0; 31]
	 *
	 * \return pair with return code (0 on success, error code otherwise) and dequeued SignalInformation object;
	 * error codes:
	 * - EAGAIN - no SignalInformation object with signal number equal to \a signalNumber was queued;
	 */

	std::pair<int, SignalInformation> acceptQueuedSignal(uint8_t signalNumber);

	/**
	 * \return set of currently queued signals
	 */

	SignalSet getQueuedSignalSet() const;

	/**
	 * \brief Adds the signalNumber and signal value (sigval union) to list of queued SignalInformation objects.
	 *
	 * \param [in] signalNumber is the signal that will be queued, [0; 31]
	 * \param [in] value is the signal value
	 *
	 * \return 0 on success, error code otherwise:
	 * - EAGAIN - no resources are available to queue the signal, \a maxElements signals are already queued;
	 * - EINVAL - \a signalNumber value is invalid;
	 */

	int queueSignal(uint8_t signalNumber, sigval value);

	SignalInformationQueue(const SignalInformationQueue&) = delete;
	SignalInformationQueue(SignalInformationQueue&&) = default;
	const SignalInformationQueue& operator=(const SignalInformationQueue&) = delete;
	SignalInformationQueue& operator=(SignalInformationQueue&&) = delete;

private:

	/// type of allocator used by \a List
	using PoolAllocatorType = PoolAllocator<SignalInformation, FeedablePool>;

	/// type of container with SignalInformation objects
	using List = std::forward_list<SignalInformation, PoolAllocatorType>;

	/// storage for queue elements
	StorageUniquePointer storageUniquePointer_;

	/// PoolAllocator::Pool used by \a poolAllocator_
	PoolAllocatorType::Pool pool_;

	/// PoolAllocator used by \a signalInformationList_ and \a freeSignalInformationList_
	PoolAllocatorType poolAllocator_;

	/// list of queued SignalInformation objects
	List signalInformationList_;

	/// list of "free" SignalInformation objects
	List freeSignalInformationList_;
};

}	// namespace internal

}	// namespace distortos

#endif	// INCLUDE_DISTORTOS_INTERNAL_SYNCHRONIZATION_SIGNALINFORMATIONQUEUE_HPP_
