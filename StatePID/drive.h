extern int stopper;
extern float kp ;
extern float ki ;
extern float kd ;

void motorMove(unsigned long leftPower, unsigned long rightPower, int ,int);
void motorStop();
float distFront();
float distRight();
