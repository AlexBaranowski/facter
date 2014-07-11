#include <facter/facts/linux/virtualization_resolver.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/fact.hpp>
#include <facter/facts/virtual_machine.hpp>
#include <facter/execution/execution.hpp>
#include <facter/util/string.hpp>
#include <facter/util/file.hpp>
#include <boost/filesystem.hpp>
#include <vector>
#include <tuple>

using namespace std;
using namespace facter::facts;
using namespace facter::util;
using namespace facter::execution;
using namespace boost::filesystem;
namespace bs = boost::system;

namespace facter { namespace facts { namespace linux {

    string virtualization_resolver::get_hypervisor(collection& facts)
    {
        // First check for Docker/LXC
        string value = get_cgroup_vm();

        // Next check for Google Compute Engine
        if (value.empty()) {
            value = get_gce_vm(facts);
        }

        // Next check based on the virt-what command
        if (value.empty()) {
            value = get_what_vm();
        }

        // Next check the vmware tool output
        if (value.empty()) {
            value = get_vmware_vm();
        }

        // Next check for OpenVZ
        if (value.empty()) {
            value = get_openvz_vm();
        }

        // Next check for VServer
        if (value.empty()) {
            value = get_vserver_vm();
        }

        // Next check for Xen
        if (value.empty()) {
            value = get_xen_vm();
        }

        // Next check the DMI product name for the VM
        if (value.empty()) {
            value = get_product_name_vm(facts);
        }

        return value;
    }

    string virtualization_resolver::get_cgroup_vm()
    {
        string value;
        file::each_line("/proc/1/cgroup", [&](string& line) {
            auto parts = split(line, ':');
            if (parts.size() < 3) {
                return true;
            }
            auto& root = parts[2];
            if (starts_with(root, "/docker/")) {
                value = vm::docker;
                return false;
            }
            if (starts_with(root, "/lxc/")) {
                value = vm::lxc;
                return false;
            }
            return true;
        });
        return value;
    }

    string virtualization_resolver::get_gce_vm(collection& facts)
    {
        auto vendor = facts.get<string_value>(fact::bios_vendor);
        if (vendor && vendor->value().find("Google") != string::npos) {
            return vm::gce;
        }
        return {};
    }

    string virtualization_resolver::get_what_vm()
    {
        string value;
        execution::each_line("virt-what", [&](string& line) {
            // Some versions of virt-what dump error/warning messages to stdout
            if (starts_with(line, "virt-what:")) {
                return true;
            }
            // Take the first line that isn't an error/warning
            value = move(line);
            return false;
        });

        // Do some normalization of virt-what's output
        if (!value.empty()) {
            to_lower(value);
            if (value == "linux_vserver") {
                return get_vserver_vm();
            }
            if (value == "xen-hvm") {
                return vm::xen_hardware;
            }
            if (value == "xen-dom0") {
                return vm::xen_privileged;
            }
            if (value == "xen-domu") {
                return vm::xen_unprivileged;
            }
            if (value == "ibm_systemz") {
                return vm::zlinux;
            }
        }
        return value;
    }

    string virtualization_resolver::get_vserver_vm()
    {
        string value;
        file::each_line("/proc/self/status", [&](string& line) {
            auto parts = split(line, ':');
            if (parts.size() != 2) {
                return true;
            }
            auto& key = trim(parts[0]);
            auto& value = trim(parts[1]);
            if (key == "s_context" || key == "VxID") {
                if (value == "0") {
                    value = vm::vserver_host;
                } else {
                    value = vm::vserver;
                }
                return false;
            }
            return true;
        });
        return value;
    }

    string virtualization_resolver::get_vmware_vm()
    {
        auto parts = split(execute("vmware -v"));
        if (parts.size() < 2) {
            return {};
        }
        return to_lower(parts[0] + '_' + parts[1]);
    }

    string virtualization_resolver::get_openvz_vm()
    {
        // Detect if it's a OpenVZ without being CloudLinux
        bs::error_code ec;
        if (!is_directory("/proc/vz", ec) ||
            is_regular_file("/proc/lve/list", ec) ||
            boost::filesystem::is_empty("/proc/vz", ec)) {
            return {};
        }
        string value;
        file::each_line("/proc/self/status", [&](string& line) {
            auto parts = split(line, ':');
            if (parts.size() != 2) {
                return true;
            }
            auto& key = trim(parts[0]);
            auto& value = trim(parts[1]);
            if (key == "envID") {
                if (value == "0") {
                    value = vm::openvz_hn;
                } else {
                    value = vm::openvz_ve;
                }
                return false;
            }
            return true;
        });
        return {};
    }

    string virtualization_resolver::get_xen_vm()
    {
        // Check for a required Xen file
        bs::error_code ec;
        if (!is_regular_file("/proc/sys/xen", ec) &&
            !is_regular_file("/sys/bus/xen", ec) &&
            !is_regular_file("/proc/xen", ec)) {
            return {};
        }

        if (is_regular_file("/dev/xen/evtchn", ec)) {
            return vm::xen_privileged;
        }
        if (is_regular_file("/proc/xen", ec)) {
            return vm::xen_unprivileged;
        }
        return {};
    }

    string virtualization_resolver::get_product_name_vm(collection& facts)
    {
        static vector<tuple<string, string>> vms = {
            make_tuple("VMware",            string(vm::vmware)),
            make_tuple("VirtualBox",        string(vm::virtualbox)),
            make_tuple("Parallels",         string(vm::parallels)),
            make_tuple("KVM",               string(vm::kvm)),
            make_tuple("Virtual Machine",   string(vm::hyperv)),
            make_tuple("RHEV Hypervisor",   string(vm::redhat_ev)),
            make_tuple("oVirt Node",        string(vm::ovirt)),
            make_tuple("HVM domU",          string(vm::xen_hardware)),
        };

        auto product_name = facts.get<string_value>(fact::product_name);
        if (!product_name) {
            return {};
        }

        auto const& value = product_name->value();

        for (auto const& vm : vms) {
            if (value.find(get<0>(vm)) != string::npos) {
                return get<1>(vm);
            }
        }
        return {};
    }

}}}  // namespace facter::facts::linux
