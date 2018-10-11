// Copyright (c) 2018 Jared H. Hudson
// Licensed under the MIT License
//

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <sys/time.h>
#include <vector>
#include <unistd.h>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include "Frame.h"
#include "HVAC.h"

using namespace std;
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

bool verbose = false;
void handler(const boost::system::error_code& error, int signal_number) {
	if (!error) {
		// A signal occurred.
	}
}

int main(int argc, char **argv) {
	int opt;
	bool pty = false;
	u_int32_t baud_rate = 9600;
	string device = "/dev/ttyUSB0";
	string mqtthost;
	int mqttport = 1883;

	// Construct a signal set registered for process termination.
	//boost::asio::signal_set signals(boost::asio::io_service, SIGINT, SIGTERM);

	// Start an asynchronous wait for one of the signals to occur.
	//signals.async_wait(handler);

	while ((opt = getopt(argc, argv, "b:f:vh:p:")) != -1) {
		switch (opt) {
		case 'b':
			pty = false;
			baud_rate = atoi(optarg);
			break;
		case 'f':
			device = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		case 'h':
			mqtthost = optarg;
			break;
		case 'p':
			mqttport = atoi(optarg);
			break;
		default: /* '?' */
			cerr << "Usage: " << argv[0] << "[-b baud rate] [-f tty/file] [-h MQTT host] [-p MQTT port]"
					<< endl;
			exit(EXIT_FAILURE);
		}
	}

	logging::add_file_log(keywords::file_name = "hvacmgr_%N.log",
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(
					0, 0, 0), keywords::format = "[%TimeStamp%]: %Message%");

	logging::core::get()->set_filter(
			logging::trivial::severity >= logging::trivial::info);
	using namespace logging::trivial;
	src::severity_logger<severity_level> lg;

	BOOST_LOG_SEV(lg, trace)<< "A trace severity message";
	BOOST_LOG_SEV(lg, debug)<< "A debug severity message";
	BOOST_LOG_SEV(lg, info)<< "An informational severity message";
	BOOST_LOG_SEV(lg, warning)<< "A warning severity message";
	BOOST_LOG_SEV(lg, error)<< "An error severity message";
	BOOST_LOG_SEV(lg, fatal)<< "A fatal severity message";

	HVAC hvac(device, baud_rate, pty, 4, mqtthost, mqttport);
	hvac.listen();

	return 0;
}
