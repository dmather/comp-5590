/*
  This file uses four unsigned ints to store the strategy and score of a player.
  The format is as follows:
     the first two ints (holding 64 bits) are used to store the strategy of the player
       the 64 bits can be thought of as a array that is indexed by the past behavior
       of the player and of the opponent
     the third int holds the initial "past behavior" that the player will use at the
       beginning of all matches -- only uses the lowest six bits
     the fourth int holds the score of the player

  I've set the POPULATION_SIZE to 80.
  I'm thinking that the thing to do is to parallelize the tournaments. I put running
  a tournament of length TOURNAMENT_LENGTH between to players (passed in by reference)
  in one function. My hope is that one function can be removed from here and put
  into the kernal fairly easily.
  
  The only thing that makes me nervous about it is the possibility that scores getting
  updated in parallel causes a clash that messes stuff up.
 */

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <chrono>

#define NUM_GENERATIONS 1000
#define POPULATION_SIZE 80

// This is used in the function tournament(...) and nowhere else.
// I think it should go into the GPU file
#define TOURNAMENT_LENGTH 30

using namespace std;

struct player{
  unsigned int strategy_first;
  unsigned int strategy_second;
  unsigned int past_behavior;
  unsigned int score;
};

// I think this function should be run in parallel.
// I've used "pass by pointer" instead of "pass by reference" because I thik that's what the kernel function
// calls used.
// Input: pOne and pTwo
// Output: pOneScoreChange and pTwoScoreChange
// was void tournament(player& pOne, player& pTwo, unsigned int& pOneScoreChange, unsigned int& pTwoScoreChange)
void tournament(player* pOne, player* pTwo, unsigned int* pOneScoreChange, unsigned int* pTwoScoreChange)
{
  const int payoff[2][2] = {{3,0},{5,1}};

  int play_counter;
  unsigned int pOneIndex;
  unsigned int pTwoIndex;
  unsigned int pOneMask;
  unsigned int pTwoMask;;
  unsigned int pOneMove;
  unsigned int pTwoMove;

  unsigned int past_temp;

  (*pOneScoreChange) = 0;
  (*pTwoScoreChange) = 0;

  pOneIndex = pOne->past_behavior;
  pTwoIndex = pTwo->past_behavior;

  for(play_counter=0;play_counter<TOURNAMENT_LENGTH;play_counter++){
    // determine moves by 'indexing' into the bit array
    pOneMask = 0x00000001;
    pTwoMask = 0x00000001;

    if(pOneIndex < 32){
      pOneMask = pOneMask << pOneIndex;
      pOneMove = pOneMask & (pOne->strategy_first);
    } else {
      pOneMask = pOneMask << (pOneIndex - 32);
      pOneMove = pOneMask & (pOne->strategy_second);
    }
    if(pOneMove) pOneMove = 1;

    if(pTwoIndex < 32){
      pTwoMask = pTwoMask << pTwoIndex;
      pTwoMove = pTwoMask & (pTwo->strategy_first);
    } else {
      pTwoMask = pTwoMask << (pTwoIndex - 32);
      pTwoMove = pTwoMask & (pTwo->strategy_second);
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
    (*pOneScoreChange) += payoff[pOneMove][pTwoMove];
    (*pTwoScoreChange) += payoff[pTwoMove][pOneMove];

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
}

void breed(const player& first_parent, const player& second_parent, player& child)
{
  int bit_index;
  unsigned int mask;

  child.strategy_first = 0x00000000;
  child.strategy_second = 0x00000000;
  child.past_behavior = 0x00000000;
  child.score = 0;

  // deal with strategy
  mask = 0x00000001;
  for(bit_index=0;bit_index<32;bit_index++){
    // first 32 bits of strategy
    if( rand() % 2 ){ // 50-50 chances comes from first parent vs second parent
      child.strategy_first = child.strategy_first | (first_parent.strategy_first & mask);
    } else {
      child.strategy_first = child.strategy_first | (second_parent.strategy_first & mask);
    }      
    // second 32 bits of strategy
    if( rand() % 2 ){ 
      child.strategy_second = child.strategy_second | (first_parent.strategy_second & mask);
    } else {
      child.strategy_second = child.strategy_second | (second_parent.strategy_second & mask);
    } 
    mask = mask << 1;
  }
  // deal with past behavior
  mask = 0x00000001;
  for(bit_index=0;bit_index<6;bit_index++){
    if( rand() % 2 ){ 
      child.past_behavior = child.past_behavior | (first_parent.past_behavior & mask);
    } else {
      child.past_behavior = child.past_behavior | (second_parent.past_behavior & mask);
    } 
    mask = mask << 1;
  }
}

void mutate(player& p)
{
  int where;
  unsigned int mask;

  where = rand() % 70;
  mask = 0x00000001;

  if(where<32){
    mask = mask << where;
    p.strategy_first = p.strategy_first ^ mask;
  } else if(where<64){
    mask = mask << (where-32);
    p.strategy_second = p.strategy_second ^ mask;
  } else {
    mask = mask << (where-64);
    p.past_behavior = p.past_behavior ^ mask;
  }
}

void randomize_player(player& p)
{
  p.strategy_first = rand();
  p.strategy_second = rand();
  p.past_behavior = rand() & 0x0000003F;
  p.score = 0;
}

void display_player(const player& p)
{
  unsigned int mask;
  int bit_counter;

  cout << "Strategy:";
  mask = 0x00000001;
  for(bit_counter = 0; bit_counter < 32; bit_counter++){
    if(bit_counter % 8 == 0) cout << ' ';
    if(mask & p.strategy_first){
      cout << 'd';
    } else {
      cout << 'c';
    }
    mask = mask << 1;
  }

  mask = 0x00000001;
  for(bit_counter = 0; bit_counter < 32; bit_counter++){
    if(bit_counter % 8 == 0) cout << ' ';
    if(mask & p.strategy_second){
      cout << 'd';
    } else {
      cout << 'c';
    }
    mask = mask << 1;
  }
  cout << endl;

  cout << "Starting History: ";
  mask = 0x00000001;
  for(bit_counter = 0; bit_counter < 6; bit_counter++){
    if(mask & p.past_behavior){
      cout << 'd';
    } else {
      cout << 'c';
    }
    mask = mask << 1;
  }
  cout << endl;
  cout << "Score: " << p.score << endl;
}

void genetic_algorithm()
{
  int i,j;
  int generation;

  player competetors[POPULATION_SIZE];
  player next_generation[POPULATION_SIZE];
  unsigned int score_i[POPULATION_SIZE], score_j[POPULATION_SIZE];
  player tmp_player;

  srand(time(NULL));

  // Start with a random population
  for(i=0;i<POPULATION_SIZE;i++){
    randomize_player(competetors[i]);
  }

  // print out starting generation
  cout << "Starting population: " << endl;
  for(i=0;i<POPULATION_SIZE;i++){
    cout << "Player " << i << endl;
    display_player(competetors[i]);
  }

  // Genetic algorithm
  for(generation=0;generation<NUM_GENERATIONS;generation++){

    // Run Competition
    for(i=0;i<POPULATION_SIZE;i++){
      
      for(j=0;j<POPULATION_SIZE;j++){ // I THINK THAT THIS IS THE LOOP TO MAKE PARALLEL
	tournament( &(competetors[i]),
                    &(competetors[j]),
                    &(score_i[j]),             // This,
                    &(score_j[j]) );           // this,
      }
      for(j=0;j<POPULATION_SIZE;j++){ // AND THIS LOOP SHOULD BE DONE AFTER THE PARALLEL LOOP COMES BACK
	competetors[i].score += (score_i[j]);  // this,
	competetors[j].score += (score_j[j]);  // and this really are supposed to be j. I think this
	                                       // will make it easier to convert to parallel.
                                               // score_i and score_j don't need to be arrays if it isn't done
                                               // in parallel
      }
    }

    // Sort Breeders
    for(i=0;i<POPULATION_SIZE;i++){
      for(j=i+1;j<POPULATION_SIZE;j++){
	if(competetors[i].score < competetors[j].score){
	  tmp_player = competetors[i];
	  competetors[i] = competetors[j];
	  competetors[j] = tmp_player;
	}
      }
    }

    /*
    // print statement for tracing / debugging
    for(i=0;i<POPULATION_SIZE;i++){
      cout << competetors[i].score << ' ';
    }
    cout << endl;
    */

    // Breeding
    for(i=0;i<POPULATION_SIZE;i++){
      breed(competetors[i%(POPULATION_SIZE/2)],competetors[rand() & POPULATION_SIZE],next_generation[i]);
    }
    /*
    // Old way -- top square root all interbreed
    for(i=0;i<BREED_SQUARE;i++){
      for(j=0;j<BREED_SQUARE;j++){
	breed(competetors[i],competetors[j],next_generation[(i*BREED_SQUARE)+j]);
      }
    }
    // Survivors
    for(i=0;i<(POPULATION_SIZE-(BREED_SQUARE*BREED_SQUARE));i++){
      next_generation[i+(BREED_SQUARE*BREED_SQUARE)] = competetors[i+BREED_SQUARE];
      next_generation[i+(BREED_SQUARE*BREED_SQUARE)].score = 0;
    }
    */

    // Set next generation to current generation
    for(i=0;i<POPULATION_SIZE;i++){
      competetors[i] = next_generation[i];
    }

    // Mutate
    for(i=0;i<POPULATION_SIZE;i++){
      mutate(competetors[i]);
    }

  }

  // print out final generation
  cout << endl << endl << "Results after " << NUM_GENERATIONS << " generations:" << endl;
  for(i=0;i<POPULATION_SIZE;i++){
    cout << "Player " << i << endl;
    display_player(next_generation[i]);
  }

  cout << "Done" << endl;
}

void trace_one_play()
{
  player p;
  player q;

  unsigned int ps, qs;

  srand(time(NULL));

  randomize_player(p);
  randomize_player(q);

  display_player(p);
  display_player(q);

  tournament(&p,&q,&ps,&qs);

  p.score += ps;
  q.score += qs;

  display_player(p);
  display_player(q);
}

int main()
{
  typedef std::chrono::high_resolution_clock Clock;
  auto t1 = Clock::now();
  genetic_algorithm();
  auto t2= Clock::now();
  std::chrono::duration<float> elapsed_seconds = t2-t1;
  cout << elapsed_seconds.count() << endl;

  // trace_one_play();

  return 0;
}
