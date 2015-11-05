#include "types.h"
#include "Dialog.h"
#include "refereeCom.h"
#include <atomic>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace po = boost::program_options;
class ObjectFinder;
class WheelController;
class CoilGun;
class ComPortScanner;
class Simulator;
class Robot {
private:
	po::variables_map config;
	Simulator *pSim = NULL;
	ICamera *camera = NULL;
	ComPortScanner *scanner = NULL;
	IWheelController * wheels = NULL;
	ICoilGun *coilBoard = NULL;
	IDisplay *m_pDisplay = NULL;
	RefereeCom *refCom = NULL;
	bool coilBoardPortsOk;
	bool wheelsPortsOk;

    //STATE state = STATE_NONE;
    std::atomic<STATE> state;
	std::atomic<STATE> last_state;
	bool ParseOptions(int argc, char* argv[]);
	void InitHardware();
	void InitSimulator(bool master, const std::string game_mode);
//	void initWheels();
//	void initCoilboard();
	void initRefCom();

	void Run();
    boost::mutex remote_mutex;
protected:
	boost::asio::io_service &io;
	OBJECT targetGate= NUMBER_OF_OBJECTS; //uselected
	bool captureFrames = false;
	std::atomic_bool autoPilotEnabled;
	std::string play_mode = "single";
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
