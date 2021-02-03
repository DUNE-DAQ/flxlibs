/**
 * @file FelixIssues.hpp FELIX card related ERS issues
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef FLXLIBS_SRC_FELIXISSUES_HPP_
#define FLXLIBS_SRC_FELIXISSUES_HPP_

#include <ers/Issue.h>

#include <string>

namespace dunedaq {

    ERS_DECLARE_ISSUE(flxlibs, CardError,
                      " FELIX Card Internal Error: " << intererror,
                      ((std::string)intererror))

    ERS_DECLARE_ISSUE(flxlibs, InitializationError,
                      " FELIX Initialization Error: " << initerror,
                      ((std::string)initerror)) 

    ERS_DECLARE_ISSUE(flxlibs, ConfigurationError,
                      " FELIX Configuration Error: " << conferror,
                      ((std::string)conferror)) 

    ERS_DECLARE_ISSUE(flxlibs, QueueTimeoutError,
                      " FELIX queue timed out: " << queuename,
                      ((std::string)queuename))

    ERS_DECLARE_ISSUE_BASE(flxlibs,
                           ResourceQueueError,
                           flxlibs::ConfigurationError,
                           " The " << queueType << " queue was not successfully created. ",
                           ((std::string)name),
                           ((std::string)queueType))

} // namespace dunedaq

#endif // FLXLIBS_SRC_FELIXISSUES_HPP_
