#include "bits/stdc++.h"

#include "block.h"
#include <vector>

/*
		TODO:
                 - coinjoin ✅
                        - fragment payout ️randomly
                 - user store balance and server perform transfer

                 - *distribute users
                    - client / server binaries
                    - local client User file, eventually encrypted
                        - addressBook maps to ip:port/~/.userfile
                 - implement unique block hash
                 - hardware wallet
 */

using namespace std;

vector<string> DEMO_ADDRESSES = {"A", "B", "C", "D"};

Block setup(map<string, User *> &addressBook) {
  // Create the first block
  Block genesisBlock = Block("A", "B", 400, "0X0X");

  // Create demo users
  for (string addr : DEMO_ADDRESSES) {
    User *temp = new User(addr);
    temp->setLedgerCopy(genesisBlock.toString());
    addressBook[temp->address] = temp;
  }

  // Test random uuid address generator
  /* User randAddrTest = User(); */
  /* randAddrTest.setLedgerCopy(genesisBlock.toString()); */
  /* addressBook[randAddrTest.address] = randAddrTest; */

  return genesisBlock;
}

void printState(map<string, User *> &addressBook) {
  for (auto user : addressBook) {
    cout << user.second->toString() << endl;
  }
}

string pickRAIDMaster(map<string, User *> &addressBook) {

  // Use RAID consensus algorithm to find a random master node
  string masterProspect = "";
  string verifier1 = "";
  string verifier2 = "";
  vector<string> addresses;
  for (auto user : addressBook) {
    addresses.push_back(user.first);
  }

  while (true) {
    shuffle(begin(addresses), end(addresses), default_random_engine(time(0)));
    masterProspect = addressBook[addresses[0]]->address;
    verifier1 = addressBook[addresses[1]]->address;
    verifier2 = addressBook[addresses[2]]->address;

    // return when all keys different && all ledgers same
    if (addressBook[masterProspect]->ledgerCopy ==
            addressBook[verifier1]->ledgerCopy &&
        addressBook[masterProspect]->ledgerCopy ==
            addressBook[verifier2]->ledgerCopy &&
        addressBook[verifier1]->ledgerCopy ==
            addressBook[verifier2]->ledgerCopy) {
      return masterProspect;
    }
    cout << "RAID consensus failed: " << masterProspect << "," << verifier1
         << "," << verifier2 << "  Retrying..." << endl;
  }
}

bool updateLedger(map<string, User *> &addressBook, string &from, string &to,
                  int &amount) {

  string currentLedger;
  string updatedLedger;
  string master = "";
  Block newTransaction = Block(from, to, amount);

  if (from != "esc") {

    master = pickRAIDMaster(addressBook);

    currentLedger = addressBook[master]->ledgerCopy;
    updatedLedger = currentLedger + newTransaction.toString();
  } else {
    currentLedger = addressBook[from]->ledgerCopy;
    updatedLedger = currentLedger + newTransaction.toString();
  }

  cout << "ledger: " << currentLedger << "\n\t---> : " << updatedLedger << endl;

  int discrepancyCount = 2;
  vector<User *> corruptedNodes;
  for (auto &item : addressBook) {
    User *node = item.second;
    if (node->ledgerCopy != currentLedger) {

      corruptedNodes.push_back(node);
      cout << ">>> ERROR: IN USER " << node->address << endl
           << node->ledgerCopy << endl;
      // If more than 'discrepancyCount' nodes throw, master assumed corrupted
      if (corruptedNodes.size() >= discrepancyCount) {
        cout << "Choosing new master node..." << endl;
        return false;
      }
    } else {
      node->setLedgerCopy(updatedLedger);
    }
  }

  for (int i = 0; i < corruptedNodes.size(); i++) {
    cout << "FIXING " << corruptedNodes[i]->address << ": "
         << corruptedNodes[i]->ledgerCopy << " ---> " << updatedLedger << endl;
    corruptedNodes[i]->setLedgerCopy(updatedLedger);
  }
  return true;
}

struct EscrowUser {

  static const int MAX_SIZE = 2;
  static const int MIN_SIZE = 1;
  static const int TIME_LIMIT = 300000;
  time_t startTime;
  User user;
  vector<pair<string, int>> receivers = {};

  EscrowUser(User user) : user(user) { this->startTime = time(NULL); };

  bool isExpired() { return (time(NULL) - this->startTime) > TIME_LIMIT; };

  bool isFull() { return this->receivers.size() >= MAX_SIZE; };

  void payout(map<string, User *> &addressBook) {
    for (size_t i = 0; i < receivers.size(); i++) {
      string from = user.address;
      string to = receivers[i].first;
      int amount = receivers[i].second;
      updateLedger(addressBook, from, to, amount);
    }
  }
};

int main() {

  // Create the genesis block, add it to the ledger and store the address book
  map<string, User *> addressBook;
  EscrowUser *escrow = new EscrowUser(User("esc"));
  Block genesisBlock = setup(addressBook);

  // Adding demo error to address ledger to test discrepancy fixer
  addressBook["D"]->ledgerCopy += "boobs,";

  // Add beta escrow user (for coinjoin testing)
  /* escrow = EscrowUser(); */
  escrow->user.setLedgerCopy(genesisBlock.toString());
  addressBook[escrow->user.address] = &(escrow->user);

  // Print the state of the blockchain
  printState(addressBook);

  // Read in user input as transactions and update ledger

  string from;
  string to;
  int amount = -1;

  while (cin >> from >> to >> amount) {
    if (amount != -1) {
      while (true) {
        if (escrow->isExpired() || escrow->isFull()) {
          // do bulk transactions
          escrow->payout(addressBook);
          // remove escrow from ledger and reinitialise
          string oldLedger = escrow->user.ledgerCopy;
          delete escrow;
          escrow = new EscrowUser(User("esc"));
          escrow->user.setLedgerCopy(oldLedger);
          addressBook[escrow->user.address] = &escrow->user;
        }

        if (updateLedger(addressBook, from, escrow->user.address, amount)) {
          escrow->receivers.push_back(make_pair(to, amount));
          break;
        }
      }
    }
    printState(addressBook);
    cout << "----------------------------------------" << endl;
  }

  bool testcase = escrow->user.ledgerCopy ==
                  "A,B,400|D,esc,99|B,esc,88|esc,B,99|esc,D,88|C,esc,44|";
  cout << "Testcase: " << (testcase == true ? "PASS" : "FAIL") << endl;
  return 0;
}
