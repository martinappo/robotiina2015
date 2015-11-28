#include "types.h"
#include "Dialog.h"
#include "refereeCom.h"
#include <atomic>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

namespace po = boost::program_options;
enum STATE
{
	STATE_NONE = 0,
	STATE_AUTOCALIBRATE,
	STATE_CALIBRATE,
	STATE_LAUNCH,
	STATE_SELECT_GATE,
	STATE_RUN,
	STATE_SETTINGS,
	STATE_REMOTE_CONTROL,
	STATE_MANUAL_CONTROL,
	STATE_DANCE,
	STATE_TEST,
	STATE_MOUSE_VISION,
	STATE_DISTANCE_CALIBRATE,
	STATE_GIVE_COMMAND,
	STATE_END_OF_GAME /* leave this last*/
};
class Robot {
private:
	po::variables_map config;
	ICamera *m_pCamera = NULL;
	ISerial *m_pSerial = NULL;
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
	SimpleSerial *serialPort;

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
	void RunCaptureTest();
};
