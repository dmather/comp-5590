/****
	WHAT's BEING PASSED IN/OUT
	input1 = tmp_player_par (struct)
	input2 = competitors[] (array[j] -- which is a struct)
	output = comp_i_score
	output2 = comp_all_score[]
****/

typedef struct {
	unsigned int strategy_first;
	unsigned int strategy_second;
	unsigned int past_behavior;
	unsigned int score;
} player;

#define TOURNAMENT_LENGTH 30

__kernel void prisoner(__global player* input1, __global player* input2,
      __global int* output, __global int* output2) {

	uint global_addr;
	//player player_i;
	//player player_j;
	player pOne;
	player pTwo;
	unsigned int pOneScoreChange;
	unsigned int pTwoScoreChange;

	/*** I MAY NEED LOCAL_MEM HERE --- DEFINED WHEN PASSED IN ***/

	global_addr = get_global_id(0); //get_global_id(0) = 0??

	//player_i
	pOne = *input1;
	//player_j 
	pTwo = input2[global_addr];
	

	/************ PLAY A MATCH HERE ************/ 
	const int payoff[2][2] = {{3,0},{5,1}};

	int play_counter;
	unsigned int pOneIndex;
	unsigned int pTwoIndex;
	unsigned int pOneMask;
	unsigned int pTwoMask;;
	unsigned int pOneMove;
	unsigned int pTwoMove;

	unsigned int past_temp;

	(pOneScoreChange) = 0;
	(pTwoScoreChange) = 0;

	pOneIndex = pOne.past_behavior;
	pTwoIndex = pTwo.past_behavior;

	for(play_counter=0;play_counter<TOURNAMENT_LENGTH;play_counter++){
	// determine moves by 'indexing' into the bit array
	pOneMask = 0x00000001;
	pTwoMask = 0x00000001;

	if(pOneIndex < 32){
		pOneMask = pOneMask << pOneIndex;
		pOneMove = pOneMask & (pOne.strategy_first);
	} else {
		pOneMask = pOneMask << (pOneIndex - 32);
		pOneMove = pOneMask & (pOne.strategy_second);
	}
	if(pOneMove) pOneMove = 1;

	if(pTwoIndex < 32){
		pTwoMask = pTwoMask << pTwoIndex;
		pTwoMove = pTwoMask & (pTwo.strategy_first);
	} else {
		pTwoMask = pTwoMask << (pTwoIndex - 32);
		pTwoMove = pTwoMask & (pTwo.strategy_second);
	}
	if(pTwoMove) pTwoMove = 1;

	/*
		// print statements for tracing / debugging
		cout << "pOneMove = " << pOneMove << endl;
		cout << "pTwoMove = " << pTwoMove << endl;
		cout << "pOneIndex = " << pOneIndex << endl;
		cout << "pTwoIndex = " << pTwoIndex << endl;
	*/

	// update scores
	(pOneScoreChange) += payoff[pOneMove][pTwoMove];
	(pTwoScoreChange) += payoff[pTwoMove][pOneMove];

	// update past behaviors
    
	// 1 hex in binary is 0001, B hex in binary is 1011
	//   so the last six bits are 011011
	//   and that means that the following line zeros out the
	//   third and sixth bits
	pOneIndex = pOneIndex & 0x0000001B;
	// shifts the bits over so the old bits 5,4,2,1 are moved 
	//   to positions 6,5,3,2
	pOneIndex = pOneIndex << 1;
	// records the player's (pOne) move in the first bit
	if(pOneMove) pOneIndex = pOneIndex | 0x00000001;
	// records the opponent's (pTwo) move in the fourth bit
	if(pTwoMove) pOneIndex = pOneIndex | 0x00000008;

	// Similarly for pTwo
	pTwoIndex = pTwoIndex & 0x0000001B;
	pTwoIndex = pTwoIndex << 1;
	if(pTwoMove) pTwoIndex = pTwoIndex | 0x00000001;
	if(pOneMove) pTwoIndex = pTwoIndex | 0x00000008;

	}
	/************ END OF PLAY A MATCH HERE ************/



	// if(get_local_id(0) == 0) { 
		
	
		/*** BOTH SCORES ***/
/*		output[get_group_id(0)] = 1;
		output2[get_group_id(0)] = 3000; */
		output[get_global_id(0)] = pOneScoreChange;
		output2[global_addr] = pTwoScoreChange;

	 //} 

}
