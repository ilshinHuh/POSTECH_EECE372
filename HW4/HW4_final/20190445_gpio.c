#include <wiringPi.h>

#define A 2
#define B 3
#define C 4
#define D 5
#define E 6
#define F 7
#define G 22
#define BTN 21

int main(void){
	if(wiringPiSetup() == -1)
		return 0;

	pinMode(A,OUTPUT);
	pinMode(B,OUTPUT);
	pinMode(C,OUTPUT);
	pinMode(D,OUTPUT);
	pinMode(E,OUTPUT);
	pinMode(F,OUTPUT);
	pinMode(G,OUTPUT);
	pinMode(BTN,INPUT);

	int count = 0;
	int btn_check = 0;
	
	while(1){
		if(digitalRead(BTN) == 1 && btn_check == 0){
			btn_check = 1;

			count++;

			if(count > 15){
				count = 0;
			}

			delay(300);
		}

		switch(count){
			case 0:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,0);
			break;
			case 1:
			digitalWrite(A,0);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,0);
			digitalWrite(E,0);
			digitalWrite(F,0);
			digitalWrite(G,0);
			break;
			case 2:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,0);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,0);
			digitalWrite(G,1);
			break;
			case 3:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,0);
			digitalWrite(F,0);
			digitalWrite(G,1);
			break;
			case 4:
			digitalWrite(A,0);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,0);
			digitalWrite(E,0);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 5:
			digitalWrite(A,1);
			digitalWrite(B,0);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,0);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 6:
			digitalWrite(A,1);
			digitalWrite(B,0);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 7:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,0);
			digitalWrite(E,0);
			digitalWrite(F,1);
			digitalWrite(G,0);
			break;
			case 8:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 9:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,0);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 10:
			digitalWrite(A,1);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,0);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 11:
			digitalWrite(A,0);
			digitalWrite(B,0);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 12:
			digitalWrite(A,0);
			digitalWrite(B,0);
			digitalWrite(C,0);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,0);
			digitalWrite(G,1);
			break;
			case 13:
			digitalWrite(A,0);
			digitalWrite(B,1);
			digitalWrite(C,1);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,0);
			digitalWrite(G,1);
			break;
			case 14:
			digitalWrite(A,1);
			digitalWrite(B,0);
			digitalWrite(C,0);
			digitalWrite(D,1);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
			case 15:
			digitalWrite(A,1);
			digitalWrite(B,0);
			digitalWrite(C,0);
			digitalWrite(D,0);
			digitalWrite(E,1);
			digitalWrite(F,1);
			digitalWrite(G,1);
			break;
		}

		if(digitalRead(BTN) == LOW){
			btn_check = 0;
		}	
	}

	return 0;
}
