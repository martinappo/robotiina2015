#define HEADING(angle) (angle > 180 ? angle - 360 : angle) 
#define BALL_IN_TRIBBLER m_pCom->BallInTribbler()
#define START_TRIBBLER m_pCom->ToggleTribbler(100);
#define STOP_TRIBBLER m_pCom->ToggleTribbler(0);
#define SIGHT_OBSTRUCTED false //TODO:Fix this
/*

void DriveToTagetFromNear(int speed, ICommunicationModule*pCom, const ObjectPosition &target) {
	pCom->Drive(speed, 0, HEADING(target.getAngle()));
}
void DriveToTagetFromFar(ICommunicationModule*pCom, const ObjectPosition &target) {
	double speed = 0;
	double angle = HEADING(target.getAngle());
	if (target.getDistance() > 100){
		speed = 60;
	}
	else{
		speed = target.getDistance(); // TODO: ilmselt veidi väiksemaks
	}
	double angleConst = abs(angle) < 13 ? 0 : std::max(std::min(50.0 / target.getDistance(), 1.0), 0.5);
	std::cout << angleConst << std::endl;
	pCom->Drive(speed, angle, angleConst * angle); // TODO: mingi väikese kaarega sõita
}*/
#define FIND_TARGET_BALL const BallPosition & target = getClosestBall();
#define FIND_TARGET_GATE const GatePosition & target = m_pFieldState->GetTargetGate();
#define TARGET_BALL_NOT_FOUND target.getDistance() > 500
#define TARGET_BALL_TOO_FAR target.getDistance() > 250
#define TARGET_BALL_IS_VERY_CLOSE target.getDistance() < 20
#define TARGET_BALL_IS_IN_CENTER HEADING(target.getAngle()) < 20
#define ROTATE_TOWARD_TO_TARGET DriveToTagetFromNear(0, m_pCom, target);
#define ROTATE_AND_DRIVE_TOWARD_TO_TARGET DriveToTagetFromFar(m_pCom, target);
#define ROTATE_AND_DRIVE_TOWARD_TO_TARGET_SLOWLY  DriveToTagetFromNear(30, m_pCom, target);
#define ROTATE_AND_DRIVE_TOWARD_TO_TARGET_GATE ROTATE_TOWARD_TO_TARGET
#define DRIVE_SIDEWAYS m_pCom->Drive(45, 90, 0);
#define TARGET_GATE_IS_IN_CENTER HEADING(target.getAngle()) < 2

#define STOP_DRIVING m_pCom->Drive(0,0,0);
#define STUCK_IN_STATE(delay) (boost::posix_time::microsec_clock::local_time() - actionStart).total_milliseconds() > delay
