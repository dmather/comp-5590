#include <iostream>
#include "redcode_fileio.h"

ostream& operator<<(ostream& outs, const memory_cell& cell)
{
  switch(cell.code){
  case DAT: outs << "DAT"; break;
  case MOV: outs << "MOV"; break;
  case ADD: outs << "ADD"; break;
  case SUB: outs << "SUB"; break;
  case MUL: outs << "MUL"; break;
  case DIV: outs << "DIV"; break;
  case MOD: outs << "MOD"; break;
  case JMP: outs << "JMP"; break;
  case JMZ: outs << "JMZ"; break;
  case DJZ: outs << "DJZ"; break;
  case CMP: outs << "CMP"; break;
  case SPL: outs << "SPL"; break;
  case NOP: outs << "NOP"; break;
  default: outs << endl << "*** ERROR: no such instruction ***" << endl;
  }
  outs << " ";
  switch(cell.mode_A){
  case IMM: outs << "#"; break;
  case DIR: outs << "$"; break;
  case IND: outs << "@"; break;
  default: outs << endl << "*** ERROR: no such addressing mode" << endl;
  }
  outs << cell.arg_A << " ";
  switch(cell.mode_B){
  case IMM: outs << "#"; break;
  case DIR: outs << "$"; break;
  case IND: outs << "@"; break;
  default: outs << endl << "*** ERROR: no such addressing mode" << endl;
  }
  outs << cell.arg_B;

  return outs;
}

istream& operator>>(istream& ins, memory_cell& cell)
{
  char instruct[4];
  char a_mode;
  int a_value;
  char b_mode;
  int b_value;
  char dummy;

  // NO ERROR CHECKING AT ALL!!!!
  ins >> instruct >> a_mode >> a_value >> b_mode >> b_value;

  if(!strcmp(instruct,"DAT")) cell.code = DAT;
  if(!strcmp(instruct,"MOV")) cell.code = MOV;
  if(!strcmp(instruct,"ADD")) cell.code = ADD;
  if(!strcmp(instruct,"SUB")) cell.code = SUB;
  if(!strcmp(instruct,"MUL")) cell.code = MUL;
  if(!strcmp(instruct,"DIV")) cell.code = DIV;
  if(!strcmp(instruct,"MOD")) cell.code = MOD;
  if(!strcmp(instruct,"JMP")) cell.code = JMP;
  if(!strcmp(instruct,"JMZ")) cell.code = JMZ;
  if(!strcmp(instruct,"DJZ")) cell.code = DJZ;
  if(!strcmp(instruct,"CMP")) cell.code = CMP;
  if(!strcmp(instruct,"SPL")) cell.code = SPL;
  if(!strcmp(instruct,"NOP")) cell.code = NOP;

  switch(a_mode){
  case '#': cell.mode_A = IMM; break;
  case '$': cell.mode_A = DIR; break;
  case '@': cell.mode_A = IND; break;
  default: cout << "*** ERROR: undefined addressing mode ***" << endl;
  }

  cell.arg_A = a_value;

  switch(b_mode){
  case '#': cell.mode_B = IMM; break;
  case '$': cell.mode_B = DIR; break;
  case '@': cell.mode_B = IND; break;
  default: cout << "*** ERROR: undefined addressing mode ***" << endl;
  }

  cell.arg_B = b_value;

  return ins;
}
