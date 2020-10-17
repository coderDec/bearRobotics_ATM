#include <iostream>
#include <stdbool.h>
#include <stdint.h>

using namespace std;

#define MIN_ACCOUNT_BALANCE -2147483647

#define ENUM_TO_STRING(x) #x

namespace ATM{

// enum for the account types
enum class ACCOUNT_TYPE{
    CHECKING =0,
    SAVING,
    SIZE
};

// enum for error codes
enum class ATM_ERROR_CODES:int32_t {
    CARD_NOT_INSERTED =0,
    WITHDRAWAL_DENIED,
    ATM_PIN_INCORRECT,
    UNVERIFIED_PIN,
    ACCOUNT_TYPE_UNKNOWN
};

// to string function for enum for error codes to make the output more redabale.
std::string enum_to_string(::ATM::ATM_ERROR_CODES code)
{   
    std::string return_string;
    switch(code){
        case ::ATM::ATM_ERROR_CODES::CARD_NOT_INSERTED:
            return_string = "CARD_NOT_INSERTED.TRANSACTION_NOT_PERMITTED";
            break;
        case ::ATM::ATM_ERROR_CODES::WITHDRAWAL_DENIED:
            return_string = "WITHDRAWAL DENIED";
            break;
        case ::ATM::ATM_ERROR_CODES::ATM_PIN_INCORRECT:
            return_string = "ATM_PIN_INCORRECT";
            break;
        case ::ATM::ATM_ERROR_CODES::UNVERIFIED_PIN:
            return_string = "UNVERIFIED_PIN. TRANSACTION_NOT_PERMITTED";
            break;
        case ::ATM::ATM_ERROR_CODES::ACCOUNT_TYPE_UNKNOWN:
            return_string = "ACCOUNT_TYPE_UNKNOWN";
            break;
        default:
            return_string = "UNKNOWN ERROR CODE";
            break;
    }
    return return_string;
}

// user defined data type for return type
typedef struct return_type_atm{
    bool transaction_successful;
    union{
        int32_t balance;
        ::ATM::ATM_ERROR_CODES error_code;
    };
}return_type_atm;

// Base class for account
class account {
    public:

        account(const ::ATM::ACCOUNT_TYPE type):account_balance(0), account_type(type) {}
        
        void deposit(const int32_t deposit_amount) {
            account_balance += deposit_amount;

        }
        virtual return_type_atm withdraw(const int32_t withraw_amount) = 0;

        void withdraw_balance_change(const int32_t withraw_amount) {
            account_balance -= withraw_amount;
        }
        
        int32_t get_balance() const {
            return account_balance;
        }

    private:
        int32_t account_balance;
        ::ATM::ACCOUNT_TYPE account_type;
}; // class account

// Checking account class
class checking_account: public account {
    public:
        checking_account():account(::ATM::ACCOUNT_TYPE::CHECKING),overdraft_protection(false)  {}
        return_type_atm withdraw(const int32_t withraw_amount) override {
            return_type_atm return_value;
            bool withdraw_successful = true;
            if (withraw_amount > get_balance()){
                if(overdraft_protection){
                    withdraw_balance_change(withraw_amount);
                } else {
                    withdraw_successful = false;
                    return_value.error_code = ::ATM::ATM_ERROR_CODES::WITHDRAWAL_DENIED;
                }
            } else {
                withdraw_balance_change(withraw_amount);
            }
            return_value.transaction_successful = withdraw_successful;
            return return_value;
        }

        void set_overdraft_protection() {
            overdraft_protection = true;
        }

    private:
        bool overdraft_protection;
}; // class checking_account

// Savings account class
class saving_account: public account {
    public:
        saving_account():account(::ATM::ACCOUNT_TYPE::SAVING), minimum_saving_value(0) {}

        return_type_atm withdraw(const int32_t withraw_amount) override {
            return_type_atm return_value;
            bool withdraw_successful = true;
            if ((get_balance() - withraw_amount) >= minimum_saving_value) {
                withdraw_balance_change(withraw_amount);
            } else {
                withdraw_successful = false;
                return_value.error_code = ::ATM::ATM_ERROR_CODES::WITHDRAWAL_DENIED;
            }
            return_value.transaction_successful = withdraw_successful;
            return return_value;
        }
    private:
        int32_t minimum_saving_value;
}; // class saving_account

// User class
class user {
    public:
        // constructor
        user(const std::string atm_pin)
        :atm_pin(atm_pin),
        account_count(2),
        user_saving_account(),
        user_checking_account(),
        selected_account_type(::ATM::ACCOUNT_TYPE::SIZE),
        card_inserted(false),
        pin_verified(false) {}

        // Function to verify pin.
        // [in]pin - pin entered by user
        return_type_atm verify_pin(const std::string pin) {
            return_type_atm return_value;
            if(card_inserted){
                bool correct_pin = false;
                if (pin == atm_pin) {
                    correct_pin = true;
                    pin_verified = true;
                }
                return_value.transaction_successful = correct_pin;
                return_value.error_code = ::ATM::ATM_ERROR_CODES::ATM_PIN_INCORRECT;
            } else {
                return_value.transaction_successful = false;
                return_value.error_code = ::ATM::ATM_ERROR_CODES::CARD_NOT_INSERTED;
            }
            return return_value;
        }

        // Function to allow user to select the account type for transaction
        // [in]type - is the account type. Checkings or Savings
        return_type_atm select_account(::ATM::ACCOUNT_TYPE type) {
            return_type_atm return_value;
            return_value = prechecks_transaction();
            if(return_value.transaction_successful) {
                bool success_select_account = true;
                if (type != ::ATM::ACCOUNT_TYPE::SIZE) {
                    selected_account_type = type;
                    success_select_account = true;
                }
                return_value.transaction_successful = true;
            }
            return return_value;
        }

        // Function to insert card
        void insert_card() {
            card_inserted = true;
        }

        // Function to remove card
        void remove_card() {
            selected_account_type = ::ATM::ACCOUNT_TYPE::SIZE;
            pin_verified = false;
            card_inserted = false;
        }

        // Function to allow user to see balance for selected account
        return_type_atm see_balance(){
            return_type_atm return_value;
            int32_t current_account_balance = MIN_ACCOUNT_BALANCE;
            return_value = prechecks_transaction();
            if (return_value.transaction_successful) {
                bool transaction_success = true;
                switch(selected_account_type) {
                    case ::ATM::ACCOUNT_TYPE::CHECKING:
                        return_value.balance = user_checking_account.get_balance();
                        break;
                    case ::ATM::ACCOUNT_TYPE::SAVING:
                        return_value.balance = user_saving_account.get_balance();
                        break;
                    default: 
                        transaction_success = false;
                        return_value.error_code = ::ATM::ATM_ERROR_CODES::ACCOUNT_TYPE_UNKNOWN;
                }
                return_value.transaction_successful = transaction_success;
            }
            return return_value;
        }

        // Function to allow user to make deposit to selected account.
        //[in]deposit_amount - takes in the amount of money to be deposited.
        return_type_atm make_deposit(const int32_t deposit_amount){
            return_type_atm return_value;
            int32_t current_account_balance = MIN_ACCOUNT_BALANCE;
            return_value = prechecks_transaction();
            if (return_value.transaction_successful) {
                switch(selected_account_type) {
                    case ::ATM::ACCOUNT_TYPE::CHECKING:
                        user_checking_account.deposit(deposit_amount);
                        break;
                    case ::ATM::ACCOUNT_TYPE::SAVING:
                        user_saving_account.deposit(deposit_amount);
                        break;
                    default: 
                        return_value.transaction_successful = false;
                        return_value.error_code = ::ATM::ATM_ERROR_CODES::ACCOUNT_TYPE_UNKNOWN;
                }
            }
            return return_value;
        }

        // Function to allow user to withdraw from selected account.
        //[in]deposit_amount - takes in the amount of money to be withdrawn.
        return_type_atm withdraw_money(const int32_t withdraw_amount){
            return_type_atm return_value;
            int32_t current_account_balance = MIN_ACCOUNT_BALANCE;
            return_value = prechecks_transaction();
            if (return_value.transaction_successful) {
                switch(selected_account_type) {
                    case ::ATM::ACCOUNT_TYPE::CHECKING:
                        return_value = user_checking_account.withdraw(withdraw_amount);
                        break;
                    case ::ATM::ACCOUNT_TYPE::SAVING:
                        return_value = user_saving_account.withdraw(withdraw_amount);
                        break;
                    default: 
                        return_value.transaction_successful = false;
                        return_value.error_code = ::ATM::ATM_ERROR_CODES::ACCOUNT_TYPE_UNKNOWN;
                }
            }
            return return_value;
        }

    private:
        std::string atm_pin;
        int8_t account_count;
        ::ATM::saving_account user_saving_account;
        ::ATM::checking_account user_checking_account;
        ::ATM::ACCOUNT_TYPE selected_account_type;
        bool card_inserted;
        bool pin_verified;

        // Function to do prechecks for every transaction
        return_type_atm prechecks_transaction()
        {   
            return_type_atm return_value;
            return_value.transaction_successful = true;
            if (!card_inserted) {
                return_value.transaction_successful = false;
                return_value.error_code = ::ATM::ATM_ERROR_CODES::CARD_NOT_INSERTED;
            } else if (!pin_verified){
                return_value.transaction_successful = false;
                return_value.error_code = ::ATM::ATM_ERROR_CODES::UNVERIFIED_PIN;
            }
            return return_value;
        }

}; // class user
}  // namespace ATM

int main()
{   
    std::cout <<"---USER 1---"<<std::endl;
    ::ATM::user user1("*78&");
    ::ATM::return_type_atm return_value;

    // Verifying pin without inserting card
    return_value = user1.verify_pin("*88&");
    if(return_value.transaction_successful){
        std::cout << "PIN CORRECT " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // Insert card
    user1.insert_card();
    std::cout<< "CARD INSERTED" <<std::endl;

    // INCORRECT PIN
    return_value = user1.verify_pin("*88&");
    if(return_value.transaction_successful){
        std::cout << "PIN CORRECT " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }


    // SELECT ACCOUNT
    return_value = user1.select_account(::ATM::ACCOUNT_TYPE::CHECKING);
    if(return_value.transaction_successful){
        std::cout << "ACCOUNT SELECTED SUCCESSFULLY " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // CORRECT PIN
    return_value = user1.verify_pin("*78&");
    if(return_value.transaction_successful){
        std::cout << "PIN CORRECT " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    std::cout <<"---USER 1: SAVING ACCOUNT---"<<std::endl;
    // SELECT ACCOUNT
    return_value = user1.select_account(::ATM::ACCOUNT_TYPE::SAVING);
    if(return_value.transaction_successful){
        std::cout << "ACCOUNT SELECTED SUCCESSFULLY " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // DEPOSIT
    return_value = user1.make_deposit(1000);
    if(return_value.transaction_successful){
        std::cout << "DEPOSIT SUCCESSFULLY MADE" << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

     // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // WITHDRAW 
    return_value = user1.withdraw_money(100);
    if(return_value.transaction_successful){
        std::cout << "WITHDREW 100 $ " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // WITHDRAW 
    return_value = user1.withdraw_money(200);
    if(return_value.transaction_successful){
        std::cout << "WITHDREW 200 $ " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // Change account
    std::cout <<"---USER 1: CHECKING ACCOUNT---"<<std::endl;
    return_value = user1.select_account(::ATM::ACCOUNT_TYPE::CHECKING);
    if(return_value.transaction_successful){
        std::cout << "ACCOUNT SELECTED SUCCESSFULLY " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // WITHDRAW MONEY
    return_value = user1.withdraw_money(500);
    if(return_value.transaction_successful){
        std::cout << "WITHDREW 500 $ " << std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }

    // REMOVE CARD
    user1.remove_card();
    std::cout<< "CARD REMOVED" <<std::endl;
    
    // VIEW BALANCE
    return_value = user1.see_balance();
    if(return_value.transaction_successful){
        std::cout << "Balance is "<< return_value.balance<<"$" <<std::endl;
    } else {
        std::cout << enum_to_string(return_value.error_code) << std::endl;
    }
    
}