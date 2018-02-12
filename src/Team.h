/**
 * Team definition
 */
#ifndef Team_h
#define Team_h

 class Team {
 public:
	Team(int id, int code, int score = 0);
 private:
	int id;
	int code;
	int score;
};


#endif
