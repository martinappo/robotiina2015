#include <boost/asio/serial_port.hpp> 
#include <boost/bind.hpp>
#include <boost/asio.hpp>

class blockingreader
{
	boost::asio::serial_port& port;
	size_t timeout;
	char c;
	boost::asio::deadline_timer timer;
	bool read_error;

	// Called when an async read completes or has been cancelled
	void read_complete(const boost::system::error_code& error,
		size_t bytes_transferred) {

		read_error = (error || bytes_transferred == 0);

		// Read has finished, so cancel the
		// timer.
		timer.cancel();
	}

	// Called when the timer's deadline expires.
	void time_out(const boost::system::error_code& error) {

		// Was the timeout was cancelled?
		if (error) {
			// yes
			return;
		}

		// no, we have timed out, so kill
		// the read operation
		// The read callback will be called
		// with an error
		port.cancel();
	}

public:

	// Constructs a blocking reader, pass in an open serial_port and
	// a timeout in milliseconds.
	blockingreader(boost::asio::serial_port& port, size_t timeout) :
		port(port), timeout(timeout),
		timer(port.get_io_service()),
		read_error(true) {
	}

	// Reads a character or times out
	// returns false if the read times out
	bool read_char(char& val) {

		val = c = '\0';

		// After a timeout & cancel it seems we need
		// to do a reset for subsequent reads to work.
		port.get_io_service().reset();

		// Asynchronously read 1 character.
		boost::asio::async_read(port, boost::asio::buffer(&c, 1),
			boost::bind(&blockingreader::read_complete,
			this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

		// Setup a deadline time to implement our timeout.
		timer.expires_from_now(boost::posix_time::milliseconds(timeout));
		timer.async_wait(boost::bind(&blockingreader::time_out,
			this, boost::asio::placeholders::error));

		// This will block until a character is read
		// or until the it is cancelled.
		port.get_io_service().run();

		if (!read_error)
			val = c;

		return !read_error;
	}
};
