// Third-party headers
#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"

// Local headers
#include "QReplMon.hpp"
#include "lsst/log/Log.h"


int main(int argc, char * const argv[])
{
    LOG_CONFIG_PROP(
        "log4j.rootLogger=INFO, console\n"
        "log4j.appender.console=org.apache.log4j.ConsoleAppender\n"
        "log4j.appender.console.layout=org.apache.log4j.PatternLayout\n"
        "log4j.appender.console.layout.ConversionPattern=[qreplmon] %-4r %-5p %m%n\n"
    );

    LOG_PUSHCTX("qrepl");
    LOG_PUSHCTX("node");

    boost::system::error_code ec;
    boost::asio::io_service ioService;

    boost::asio::signal_set signals{ioService, SIGINT, SIGTERM, SIGQUIT};
    signals.async_wait([&ioService](boost::system::error_code const&, int) {
        ioService.stop();
    });

    lsst::qserv::qrepl::QReplMon qReplMon(ioService, 8080);
    ec = qReplMon.init();
    if (!ec) {
        ioService.run(ec);
    }

    LOGS_INFO("Exiting: " << ec.message());
    return ec.value();
}
