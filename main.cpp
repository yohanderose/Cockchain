#include "bits/stdc++.h"

#include "block.h"
#include <vector>

// TODO: - implement a unique hash for blocks
//		 - make ledger copies files with a list of hashes instead of
//		   a string in memory
//		 - refactor / recode so application is distributed
//		 - coinjoin
//		 - hardware wallet

using namespace std;

vector<string> DEMO_ADDRESSES = {"A", "B", "C", "D"};

void setup(map<string, User> &addressBook) {
  // Create the first block
  Block genesisBlock = Block("A", "B", 400, "0X0X");

  // Create demo users
  for (string addr : DEMO_ADDRESSES) {
    User temp = User(addr);
    temp.setLedgerCopy(genesisBlock.toString());
    addressBook[temp.address] = temp;
  }

  // Check random uuid address generator
  User randAddrTest = User();
  randAddrTest.setLedgerCopy(genesisBlock.toString());
  addressBook[randAddrTest.address] = randAddrTest;
}

void printState(map<string, User> &addressBook) {
  for (auto &item : addressBook) {
    cout << item.second.toString() << endl;
  }
}

string pickRAIDMaster(map<string, User> &addressBook) {

  // Use RAID consensus algorithm to find a random master node
  std::random_device rd;  // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(0,
                                        addressBook.size() -
                                            1); // define the range

  auto it = addressBook.begin();
  advance(it, distr(gen));
  string masterProspect = it->first;
  it = addressBook.begin();
  advance(it, distr(gen));
  string verifier1 = it->first;
  it = addressBook.begin();
  advance(it, distr(gen));
  string verifier2 = it->first;
  while (masterProspect == verifier1) {
    it = addressBook.begin();
    advance(it, distr(gen));
    verifier1 = it->first;
  };
  while (masterProspect == verifier2 || verifier1 == verifier2) {
    it = addressBook.begin();
    advance(it, distr(gen));
    verifier2 = it->first;
  };
  /* cout << masterProspect << " " << verifier1 << " " << verifier2 << endl; */

  return masterProspect;
}

bool updateLedger(map<string, User> &addressBook, string &from, string &to,
                  int &amount) {

  Block newTransaction = Block(from, to, amount);

  string master = pickRAIDMaster(addressBook);
  cout << "MASTER: " << master << endl;

  string currentLedger = addressBook[master].ledgerCopy;
  string updatedLedger = currentLedger + newTransaction.toString();
  cout << "current: " << currentLedger << "\n---> updated:" << updatedLedger
       << endl;

  int discrepancyCount = 2;
  vector<User *> corruptedNodes;
  for (auto &item : addressBook) {
    User *node = &item.second;
    if (node->ledgerCopy != currentLedger) {

      corruptedNodes.push_back(node);
      cout << "ERROR FOUND IN USER " << node->address << endl;
      // If more than 'discrepancyCount' nodes throw, master assumed corrupted
      if (corruptedNodes.size() >= discrepancyCount) {
        cout << "Choosing new master node..." << endl;
        return false;
      }
    } else {
      node->setLedgerCopy(updatedLedger);
    }
  }

  if (corruptedNodes.size() < 1) {
    return true;
  } else {
    for (int i = 0; i < corruptedNodes.size(); i++) {
      cout << "FIXING: " << corruptedNodes[i]->address << endl;
      corruptedNodes[i]->setLedgerCopy(updatedLedger);
    }
    return true;
  }
}

int main() {

  // Create the genesis block, and add it to the ledger
  // Then store all known addresses in a map
  map<string, User> addressBook;
  setup(addressBook);

  // Adding demo error to address ledger to test discrepancy fixer
  /* addressBook["D"].ledgerCopy += "boobs;"; */

  // Print the state of the blockchain
  printState(addressBook);

  // Read in user input as transactions and update ledger
  string from;
  string to;
  int amount = -1;
  while (cin >> from >> to >> amount) {
    // cout << "from:" << from << ", to:" << to << ", amount:" << amount <<
    // endl;
    if (amount != -1) {
      while (true) {
        if (updateLedger(addressBook, from, to, amount)) {
          break;
        }
      }
    }
    printState(addressBook);
  }

  return 0;
}
