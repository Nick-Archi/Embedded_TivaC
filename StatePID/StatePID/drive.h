extern int stopper;
extern float kp ;
extern float ki ;
extern float kd ;
//extern int limit ;

void motorMove(unsigned long leftPower, unsigned long rightPower, int ,int);
void motorStop();
float ReadFrontWall_US_W();
float ReadWall_IR();
