/**
 * @file
 * Declares the BSD networking fact resolver.
 */
#ifndef FACTER_FACTS_BSD_NETWORKING_RESOLVER_HPP_
#define FACTER_FACTS_BSD_NETWORKING_RESOLVER_HPP_

#include "../posix/networking_resolver.hpp"
#include <ifaddrs.h>

namespace facter { namespace facts { namespace bsd {

    /**
     * Responsible for resolving networking facts.
     */
    struct networking_resolver : posix::networking_resolver
    {
     protected:
        /**
         * Called to resolve interface facts.
         * @param facts The fact map that is resolving facts.
         */
        virtual void resolve_interface_facts(fact_map& facts);

     private:
        void resolve_address(fact_map& facts, ifaddrs const* addr, bool primary);
        void resolve_network(fact_map& facts, ifaddrs const* addr, bool primary);
        void resolve_mtu(fact_map& facts, ifaddrs const* addr);
    };

}}}  // namespace facter::facts::bsd

#endif  // FACTER_FACTS_BSD_NETWORKING_RESOLVER_HPP_
