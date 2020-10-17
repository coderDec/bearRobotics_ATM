# Instructions to clone the file:
git clone https://github.com/coderDec/bearRobotics_ATM.git

# Instructions to build the file:
From the directory where the repository is cloned, run the folloing command:
g++ -std=c++11 -o ATM ATM.cpp

# Instructions to Run the tests:
From the same directory run:
./ATM

# Additional information to write more tests:
1. More user's can be created by creating objects of user class. Pin number can be specified at the time of creation.
2. Each user upon creation has Checkings and Savings types of accounts.
3. Insert card by calling -> insert_card();
4. Verify Pin by calling -> verify_pin(<pin number>)
5. Select desired account to do the transaction
6. Deposit money by calling -> make_deposit(<amount>)
7. Withdraw money by calling -> withdraw_money(<amount>)
8. Remove card by calling -> remove_card()