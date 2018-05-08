/**
 * Game related constants
 */

 // Game states
export const State = {
 	IDLE: 0,
 	STARTING: 1,
 	PLAY: 2,
 	ENDING: 10,
 	END: 11
 };
 export const StateName = {
 	0: 'Idle',
 	1: 'Starting',
 	2: 'Play',
 	10: 'Ending',
 	11: 'End'
};

 // Different types of game
export const Type = {
	DOMINATION: 'DOMINATION',
	// RANDOM: 'RANDOM'
 }
export const TypeInfo = {
	NONE: {
		name: 'None',
		description: 'Select a game type'
	},
 	DOMINATION: {
 		name: 'Domination',
 		description: 'Fight for control'
 	},
 	RANDOM: {
 		name: 'Whack-a-Node',
 		description: 'When it lights up, shoot it'
 	}
};
