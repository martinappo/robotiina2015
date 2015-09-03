#include "types.h"
#include "Dialog.h"
#include <atomic>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace po = boost::program_options;
class ObjectFinder;
class WheelController;
class CoilGun;
class ComPortScanner;

class Robot {
private:
	po::variables_map config;

    ICamera *camera;
	ComPortScanner *scanner;
    WheelController * wheels;
	CoilGun *coilBoard;
	IDisplay *m_pDisplay;
	bool coilBoardPortsOk;
	bool wheelsPortsOk;

    //STATE state = STATE_NONE;
    std::atomic<STATE> state;
	std::atomic<STATE> last_state;
	bool ParseOptions(int argc, char* argv[]);
	void initCamera();
	void initPorts();
	void initWheels();
	void initCoilboard();

	void Run();
    boost::mutex remote_mutex;
protected:
	boost::asio::io_service &io;
	OBJECT targetGate= NUMBER_OF_OBJECTS; //uselected
	bool captureFrames = false;
	std::atomic_bool autoPilotEnabled;

public:
    Robot(boost::asio::io_service &io);
	bool Launch(int argc, char* argv[]);
	~Robot();

    int GetState() {
        return state;
    }
	int GetLastState() {
		return last_state;
	}
    void SetState(STATE new_state) {
		last_state = (STATE)state;
        state = new_state;
    }

};
