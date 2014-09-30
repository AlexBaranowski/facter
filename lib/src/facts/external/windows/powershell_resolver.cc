#include <facter/facts/external/windows/powershell_resolver.hpp>
#include <facter/facts/collection.hpp>
#include <facter/facts/scalar_value.hpp>
#include <facter/logging/logging.hpp>
#include <facter/execution/execution.hpp>
#include <facter/util/windows/scoped_error.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <windows.h>
#include <Shlobj.h>

using namespace std;
using namespace facter::execution;
using namespace facter::util;
using namespace boost::filesystem;

LOG_DECLARE_NAMESPACE("facts.external.powershell");

namespace facter { namespace facts { namespace external {

    bool powershell_resolver::can_resolve(string const& file) const
    {
        try {
            path p = file;
            return boost::iends_with(file, ".ps1") && is_regular_file(p);
        } catch (filesystem_error &e) {
            LOG_TRACE("error reading status of path %1%: %2%", file, e.what());
            return false;
        }
    }

    void powershell_resolver::resolve(string const& file, collection& facts) const
    {
        LOG_DEBUG("resolving facts from powershell script \"%1%\".", file);

        try
        {
            // When facter is a 32-bit process running on 64-bit windows (such as in a 32-bit puppet installation that includes
            // native facter), this executes the 32-bit powershell and leads to problems. For example, if using powershell to read
            // values from the registry, it will read the 32-bit view of the registry. Also 32 and 64-bit versions have different
            // modules available (since PSModulePath is in system32). In those circumstances, Windows invisibly redirects to
            // 32-bit executables; it also provides a link at %SYSTEMROOT%\sysnative for the 64-bit versions. So explicitly look
            // for Powershell at sysnative before trying PATH lookup.
            TCHAR szPath[MAX_PATH];
            if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, 0, szPath))) {
                auto err = GetLastError();
                LOG_DEBUG("error finding SYSTEMROOT: %1% (%2%)", scoped_error(err), err);
            }
            auto p = path(szPath) / "sysnative" / "WindowsPowerShell" / "v1.0" / "powershell.exe";
            auto pwrshell = which(p.string());
            if (pwrshell.empty()) {
                pwrshell = "powershell";
            }

            execution::each_line(pwrshell, {"-NoProfile -NonInteractive -NoLogo -ExecutionPolicy Bypass -File ", "\"" + file + "\""},
            [&facts](string const& line) {
                auto pos = line.find('=');
                if (pos == string::npos) {
                    LOG_DEBUG("ignoring line in output: %1%", line);
                    return true;
                }
                // Add as a string fact
                string fact = line.substr(0, pos);
                boost::to_lower(fact);
                facts.add(move(fact), make_value<string_value>(line.substr(pos+1)));
                return true;
            }, { execution_options::defaults, execution_options::throw_on_failure });
        }
        catch (execution_exception& ex) {
            throw external_fact_exception(ex.what());
        }

        LOG_DEBUG("completed resolving facts from powershell script \"%1%\".", file);
    }

}}}  // namespace facter::facts::external
