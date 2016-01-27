// System headers
#include <csignal>
#include <getopt.h>
#include <string>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"

// Local headers
#include "QReplNode.hpp"
#include "lsst/log/Log.h"

using namespace std;

const struct option long_options[] = {
    {"status", required_argument, 0, 's'},
    {0, 0, 0, 0}
};

const char short_options[] = "s:";


int main(int argc, char* const argv[])
{
    LOG_CONFIG_PROP(
        "log4j.rootLogger=INFO, console\n"
        "log4j.appender.console=org.apache.log4j.ConsoleAppender\n"
        "log4j.appender.console.layout=org.apache.log4j.PatternLayout\n"
        "log4j.appender.console.layout.ConversionPattern=[%X{nodeId}] %-4r %-5p %m%n\n"
    );

    LOG_PUSHCTX("qrepl");
    LOG_PUSHCTX("node");

    int opt;
    string status;

    while((opt = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
        switch(opt) {
        case 's':
            status = optarg;
            break;
        }
    }

    boost::system::error_code ec;
    boost::asio::io_service ioService;

    boost::asio::signal_set signals{ioService, SIGINT, SIGTERM, SIGQUIT};
    signals.async_wait([&ioService](const boost::system::error_code&, int) {
        ioService.stop();
    });

    lsst::qserv::qrepl::QReplNode node{ioService};
    ec = node.init(status);
    if (ec) {
        LOGS_INFO("Exiting: " << ec.message());
        return ec.value();
    }

    ioService.run(ec);
    LOGS_INFO("Exiting: " << ec.message());
    return ec.value();
}
