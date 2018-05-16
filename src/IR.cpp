
#ifndef IR_h
#include <IR.h>
#endif

IRrecv irrecv(IR_PIN);
decode_results  results;        // Somewhere to store the results

void setupIR() {

  // Start the receiver
  irrecv.enableIRIn();
}

void loopIR() {
	if (irrecv.decode(&results)) {  // Grab an IR code

		Serial.printf("IR: %x Addr: %d Command: %d\n", (long)results.value, results.address, results.command);
		irrecv.resume();              // Prepare for the next value
		int t = results.address;
		if(t < 1 || t > TEAM_COUNT_MAX)
		{
			return;
		}
		// Teams are 0 indexed everywhere else
		hit(t-1, results.command);
	}
}
