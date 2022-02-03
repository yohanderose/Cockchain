#include <string>

#include "user.h"
using namespace std;

// Logical unit that represents a transaction

struct Block {
  string from;
  string to;
  int amount;
  string hash;
  Block *prevBlock;

  Block(string from, string to, int amount, string hash = "")
      : from(from), to(to), amount(amount),
        hash(hash == "" ? create_hash() : hash){};

  // TODO: implement create unique hash function
  string create_hash() { return "temp"; }

  string toString() {
    return from + "," + to + "," + std::to_string(amount) + "|";
  }
};
