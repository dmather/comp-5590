#ifndef REDCODE_FILEIO_H
#define REDCODE_FILEIO_H

#include <fstream>

ostream& operator<<(ostream& outs, const memory_cell& cell);
istream& operator>>(istream& ins, memory_cell& cell);

#endif // REDCODE_FILEIO_H
