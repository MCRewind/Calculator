#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <vector>

using namespace std;

string evaluateTinyBlock(string input) {
    //Search for an operator, and use the digits either side of it as operands
    int iOutput = -1;
    int opPosition = 0;
    int iOperand1 = 1;
    int iOperand2 = -1;

    for(unsigned int i = 0; i != input.length(); ++i) {
        if(isdigit(input.at(i)) == 0) {
            opPosition = i;

            string strOperand1;
            for(unsigned int k = 0; k != i; ++k) {
                strOperand1 += input.at(k);
            }
            iOperand1 = atoi(strOperand1.c_str());

            string strOperand2;
            for(unsigned int k = i+1; k != input.length(); ++k) {
                strOperand2 += input.at(k);
            }
            iOperand2 = atoi(strOperand2.c_str());

//            cout << "sOperand1 = " << strOperand1 << endl;
//            cout << "sOperand2 = " << strOperand2 << endl;

            break;
        }
    }

    char sOperator = input.at(opPosition);

    switch (sOperator) {
    case '+': iOutput = iOperand1 + iOperand2; break;
    case '-': iOutput = iOperand1 - iOperand2; break;
    case '*': iOutput = iOperand1 * iOperand2; break;
    case '/': iOutput = iOperand1 / iOperand2; break;
    case '^': iOutput = pow(iOperand1,iOperand2); break;
    }

//    cout << "sOperator = " << sOperator << endl;

    char * sOutput = new char;
    itoa(iOutput,sOutput,10);
    return sOutput;
}

string evaluateMultiBlockLeftToRight(string input) {
    string currentEquation = input;
    string tinyBlock = "";
    bool foundOperator = false;
    bool foundSecondOperator = false;

    do {
        tinyBlock = "";
        foundOperator = false;
        foundSecondOperator = false;
        for(unsigned int i = 0; i != currentEquation.length(); ++i) {
            if(isdigit(currentEquation.at(i)) == 0) {

                if(foundOperator) { //We've found a second operator, so we need to condense the first part into a single value
                    foundSecondOperator = true;
                    currentEquation.erase(0,i);
                    currentEquation.insert(0,evaluateTinyBlock(tinyBlock));
                    //cout << currentEquation << endl << "---" << endl;
                    break;
                } else {
                    foundOperator = true;
                }

            }
            tinyBlock += currentEquation.at(i);
        }
        //cout << tinyBlock << endl;
    } while(foundSecondOperator == true);

    return evaluateTinyBlock(currentEquation);
}

enum { TOKEN_PLUS = 1, TOKEN_MINUS = 2, TOKEN_MULTIPLY, TOKEN_DIVIDE };

string evaluateBlockRecursive(string sInput) {
    vector<int> tokenList;
    bool blNumberString = false;
    string stNumber;

    int i = 0;
    while(i < sInput.length()) {

        //if(blNumberString) {
            cout << i << "; " << sInput.at(i) << endl;
        //}

        if(isdigit(sInput.at(i))) { //We've found a number, keep adding the current char to a string til we find a non-digit

            if(blNumberString) {
                stNumber += sInput.at(i);
            } else {
                stNumber = sInput.at(i);
                blNumberString = true;
            }

        } else { //Else add the appropriate token to the vector
            if(blNumberString) {
                tokenList.push_back(-atoi(stNumber.c_str()));
                blNumberString = false;
            }

            switch (sInput.at(i)) {
            case '+': tokenList.push_back(TOKEN_PLUS);
            case '-': tokenList.push_back(TOKEN_MINUS);
            case '*': tokenList.push_back(TOKEN_MULTIPLY);
            case '/': tokenList.push_back(TOKEN_DIVIDE);
            }
        }

        ++i;
    }
    if(blNumberString) {
        tokenList.push_back(-atoi(stNumber.c_str()));
        blNumberString = false;
    }

    cout << "------\n";

    for( i = 0; i != tokenList.size(); ++i ) {
        char * tempChar;
        itoa(tokenList.at(i), tempChar, 10);
        cout << i << ": " << tokenList.at(i) << endl;
    }


    return "Done";
}

string evaluateBlock(string sInput) {
    string input = sInput;

    //Order of BIDMAS:
    //Level 0: +, -
    //Level 1: *, /, %
    //Level 2: ^
    //Level 3: (, )
    bool tokens [4] = {false,false,false,false};

    while(input.find_first_of(' ') != string::npos) { //Clear all whitespaces
        input.erase(input.find_first_of(' '));
        cout << "h\n";
    }

    for (int i = 0; i != input.length(); ++i) { //Do a scan first, if it only contains level 1 xor level 2 items, then it can be evaluated left to right
        switch (input.at(i)) {
        case '+':
        case '-': tokens[0] = true; break;
        case '*':
        case '/':
        case '%': tokens[1] = true; break;
        case '^': tokens[2] = true; break;
        }
    }

    int numTrues = 0;
    for (int i = 0; i != 4; ++i) {
        if(tokens[i] == true) {
            ++numTrues;
        }
    }


    return evaluateBlockRecursive(input);

    if(numTrues == 1) {
        return evaluateMultiBlockLeftToRight(input);
    } else if(numTrues > 1) {
        return evaluateBlockRecursive(input);
    } else {
        return input;
    }

    return "Not able to perform calculation.";
}

int main()
{
    cout << "Hello World!" << endl << ">>> ";

    string sLineIn;
    cin >> sLineIn;
    while(sLineIn != "exit") {
        cout << evaluateBlockRecursive(sLineIn) << endl << ">>> ";
        cin >> sLineIn;
    }

    return 0;
}

