#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <cmath>
#include <vector>
#include <set>

using namespace std;

enum { EMPTY_VALUE, TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE_FLOAT, TOKEN_DIVIDE_INT, TOKEN_POWER, TOKEN_LEFTBRACKET, TOKEN_RIGHTBRACKET, TOKEN_MODULO, TOKEN_DOUBLE,
       TOKEN_SPACE, TOKEN_SIN, TOKEN_COS, TOKEN_TAN, TOKEN_ABS, TOKEN_FACTORIAL };
enum { NO_ERROR, EMPTY_BRACKETS, ERROR_ILLEGAL_STR, ERROR_SECOND_DECIMAL, ERROR_UNEXPECTED_DECIMAL, ERROR_FAULTY_END, ERROR_MISMATCHED_BRACKETS };

const double m_pi = 3.14159265358979323846264338327950288419716939937510;
const double m_e =  2.71828182845904523536028747135266249775724709369995;

char a_allowedChars[] = {'+','-','*','/','0','1','2','3','4','5','6','7','8','9','(',')','^','.','%','!'};
set<char> allowedCharsSet (a_allowedChars,a_allowedChars+20);

struct stTokens {
       vector<long double> values;
       vector<int> types;
       void removePos(int pos) {
           values.erase(values.begin()+pos);
           types.erase(types.begin()+pos);
       }
       void removePos(int pos, int posEnd) {
           values.erase(values.begin()+pos, values.begin()+posEnd);
           types.erase(types.begin()+pos, types.begin()+posEnd);           
       }
       void dumpData() {
           for (unsigned int i = 0; i != values.size(); ++i) {
               cout << "#" << i << ": " << types.at(i) << ", " << values.at(i) << endl;
           }
           cout << "Dump complete\n";
       }
       void addData(int tokenType, long double value = EMPTY_VALUE) {
           types.push_back(tokenType);
           values.push_back(value);
       }
       void setData(int pos, int tokenType, long double value) {
           types.at(pos) = tokenType;
           values.at(pos) = value;
       }
       void insertData(int pos, int tokenType, long double value = EMPTY_VALUE) {
           types.insert(types.begin()+pos, tokenType);
           values.insert(values.begin()+pos, value);
       }
};

/**
  Stop searching through the string for extra characters to add to the current string, and place the number as a single value to our tokenList
  */
inline void addDoubleFromString(stTokens &tokenList, bool &numString, bool &foundDec, string input) {
    if(numString) {
        tokenList.addData(TOKEN_DOUBLE,atof(input.c_str()));
        numString = false;
        foundDec = false;
    }
}

string stripSpaces(string input) {
    string newString;
    for(unsigned int i = 0; i != input.size(); ++i) {
        if(input.at(i) != ' ') {
            newString += input.at(i);
        }
    }
    return newString;
}

int factorial(int n) {
    if(n == 1)
        return 1;
    else
        return n*factorial(n-1);
}

int convertStringToTokens(string sInput, stTokens &tokenList, long double previousAnswer = 0) {

    sInput = stripSpaces(sInput);

    bool blNumberString = false;
    bool blFoundDecimal = false;
    string stNumber;

    unsigned int i = 0;
    while(i < sInput.length()) {

        if(isdigit(sInput.at(i))) { //We've found a number, keep adding the current char to a string til we find a non-digit

            if(blNumberString) {
                stNumber += sInput.at(i);
            } else {
                stNumber = sInput.at(i);
                blNumberString = true;
            }

        }
        else if(sInput.at(i) == '.') {
            if(blNumberString) {
                if(blFoundDecimal) {
                    cout << "A second decimal point was found at position " << i+1 << ", could not evaluate expression.\n";
                    return ERROR_SECOND_DECIMAL;
                } else {
                    blFoundDecimal = true;
                    stNumber += sInput.at(i);
                }
            } else {
                cout << "An unexpected decimal point was found at position " << i+1 << ", could not evaluate expression.\n";
                return ERROR_UNEXPECTED_DECIMAL;
            }
        }
        else if(isspace(sInput.at(i))) {
            //DO ABSOLUTELY NOTHING
        }
        else if(allowedCharsSet.find(sInput.at(i)) == allowedCharsSet.end()) {
            //We've found a letter, it could be part of a function. Find the whole word, and then see if it's an allowed word.
            addDoubleFromString(tokenList, blNumberString,blFoundDecimal,stNumber);

            string stWordBuffer;
            while( (i < sInput.length()) ) {
                if (allowedCharsSet.find(sInput.at(i)) == allowedCharsSet.end()) {
                    stWordBuffer += tolower(sInput.at(i));
                } else {
                    break;
                }
                ++i;
            }

            if(stWordBuffer == "div") {
                tokenList.addData(TOKEN_DIVIDE_INT);
            }
            else if(stWordBuffer == "mod") {
                tokenList.addData(TOKEN_MODULO);
            }
            else if(stWordBuffer == "sin") {
                tokenList.addData(TOKEN_SIN);
            }
            else if(stWordBuffer == "cos") {
                tokenList.addData(TOKEN_COS);
            }
            else if(stWordBuffer == "tan") {
                tokenList.addData(TOKEN_TAN);
            }
            else if(stWordBuffer == "abs") {
                tokenList.addData(TOKEN_ABS);
            }
            else if(stWordBuffer == "e") {
                tokenList.addData(TOKEN_DOUBLE, m_e);
            }
            else if(stWordBuffer == "pi") {
                tokenList.addData(TOKEN_DOUBLE, m_pi);
            }
            else if(stWordBuffer == "ans") {
                tokenList.addData(TOKEN_DOUBLE, previousAnswer);
            }
            else {
                cout << "An illegal set of characters, '" << stWordBuffer << "' was found at position " << i+1 << ", could not evaluate expression.\n";
                return ERROR_ILLEGAL_STR;
            }

            continue;
        }
        else { //Else add the appropriate token to the vector
            addDoubleFromString(tokenList, blNumberString, blFoundDecimal,stNumber);

            switch (sInput.at(i)) {
            case '+': tokenList.addData(TOKEN_PLUS); break;
            case '-': tokenList.addData(TOKEN_MINUS); break;
            case '*': tokenList.addData(TOKEN_MULTIPLY); break;
            case '/': tokenList.addData(TOKEN_DIVIDE_FLOAT); break;
            case '^': tokenList.addData(TOKEN_POWER); break;
            case '(': tokenList.addData(TOKEN_LEFTBRACKET); break;
            case ')': tokenList.addData(TOKEN_RIGHTBRACKET); break;
            case '%': tokenList.addData(TOKEN_MODULO); break;
            case '!': tokenList.addData(TOKEN_FACTORIAL); break;
            }
        }

        ++i;
    }
    if(blNumberString) {
        addDoubleFromString(tokenList, blNumberString,blFoundDecimal,stNumber);
    }

    //Remove spaces
    unsigned int count = 0, deleted = 0;
    while(count - deleted < tokenList.types.size()) {
        if(tokenList.types.at(count-deleted) == TOKEN_SPACE) {
            tokenList.removePos(count-deleted);
            ++deleted;
        }
        ++count;
    }

    //Perform various checks to ensure we don't have faulty token data that could mess up our calculation later

    //Check for mismatched brackets
    int lBrackets = 0, rBrackets = 0;
    for (unsigned int i = 0; i != tokenList.types.size(); ++i) {
        if(tokenList.types.at(i) == TOKEN_LEFTBRACKET) ++lBrackets;
        if(tokenList.types.at(i) == TOKEN_RIGHTBRACKET) ++rBrackets;
    }
    if(lBrackets != rBrackets) {
        cout << "Number of left brackets does not match the number of right brackets.\n";
        return ERROR_MISMATCHED_BRACKETS;
    }

    //Add a leading 0 to starting + or - tokens
    if( (tokenList.types.front() == TOKEN_PLUS) || (tokenList.types.front() == TOKEN_MINUS) ) {
        tokenList.addData(TOKEN_DOUBLE,0);
    }

    //Insert * between any TOKEN_DOUBLE and TOKEN_LEFTBRACKET
    for (unsigned int i = 1; i < tokenList.types.size(); ++i) {
        if( (tokenList.types.at(i-1) == TOKEN_DOUBLE) && (tokenList.types.at(i) == TOKEN_LEFTBRACKET) ) tokenList.insertData(i,TOKEN_MULTIPLY);
        if( (tokenList.types.at(i-1) == TOKEN_RIGHTBRACKET) && (tokenList.types.at(i) == TOKEN_DOUBLE) ) tokenList.insertData(i,TOKEN_MULTIPLY);
        if( (tokenList.types.at(i-1) == TOKEN_RIGHTBRACKET) && (tokenList.types.at(i) == TOKEN_LEFTBRACKET) ) tokenList.insertData(i,TOKEN_MULTIPLY);
    }

    //tokenList.dumpData();
    //cout << "Converted string '" << sInput << "' to tokens successfully.\n";
    return NO_ERROR;
}

int evaluateTokens_rc(stTokens &tokens, long double &result) {

    //tokens.dumpData();

    if(tokens.values.size() == 0) { result = 0; return EMPTY_BRACKETS; }

    if (! ((tokens.types.back() == TOKEN_DOUBLE) || (tokens.types.back() == TOKEN_RIGHTBRACKET) || (tokens.types.back() == TOKEN_FACTORIAL)) ) {
        cout << "The final character (possibly within a bracket) was not a number or closing bracket.\n";
        result = 0;
        return ERROR_FAULTY_END;
    }

    //Check for brackets, and recursively evaluate anything inside them
    int leftBracketsFound = 0;
    int rightBracketsFound = 0;
    stTokens smallerEvaluation;
    int posLeftBracket, posRightBracket;
    for (unsigned int i = 0; i < tokens.types.size(); ++i) {

        if(leftBracketsFound > 0) {
            smallerEvaluation.types.push_back(tokens.types.at(i));
            smallerEvaluation.values.push_back(tokens.values.at(i));
        }

        if(tokens.types.at(i) == TOKEN_LEFTBRACKET) {
            ++leftBracketsFound;
            if(leftBracketsFound == 1)
                posLeftBracket = i;
        }
        else if(tokens.types.at(i) == TOKEN_RIGHTBRACKET) {
            ++rightBracketsFound;
            if( (leftBracketsFound == rightBracketsFound) && (rightBracketsFound > 0) ) {
                posRightBracket = i;
                //Remove the brackets and contents
                tokens.removePos(posLeftBracket, posRightBracket);
                smallerEvaluation.removePos(smallerEvaluation.types.size()-1);
                //Evaluate the brackets and insert that into the tokens 
                long double dblSmallerEvaluate;
                int rsltSmallerEvaluate = evaluateTokens_rc(smallerEvaluation, dblSmallerEvaluate);
                if(rsltSmallerEvaluate != NO_ERROR) {
                    //There was an error found, so return that error code.
                    return rsltSmallerEvaluate;
                }

                tokens.types.at(posLeftBracket) = TOKEN_DOUBLE;
                tokens.values.at(posLeftBracket) = dblSmallerEvaluate;

                i = 0;
                smallerEvaluation.values.clear();
                smallerEvaluation.types.clear();

                leftBracketsFound = 0;
                rightBracketsFound = 0;
            }
        }

    }

    //Find carets for exponents evaluation
    for (int i = tokens.types.size() - 1; i >= 0; --i) {
        switch(tokens.types.at(i)) {
        case TOKEN_POWER:
            long double base = tokens.values.at(i-1);
            long double exp = tokens.values.at(i+1);
            tokens.setData(i,TOKEN_DOUBLE,pow(base,exp)); tokens.removePos(i-1); tokens.removePos(i); --i; break;
        }
    }

    //Now run functions, like sin, cos
    for (unsigned int i = 0; i < tokens.types.size(); ++i) {
        int changeMade = 0;
        switch(tokens.types.at(i)) {
        case TOKEN_SIN: tokens.setData(i,TOKEN_DOUBLE,sin(tokens.values.at(i+1))); changeMade = 1; break;
        case TOKEN_COS: tokens.setData(i,TOKEN_DOUBLE,cos(tokens.values.at(i+1))); changeMade = 1; break;
        case TOKEN_TAN: tokens.setData(i,TOKEN_DOUBLE,tan(tokens.values.at(i+1))); changeMade = 1; break;
        case TOKEN_ABS: tokens.setData(i,TOKEN_DOUBLE,abs(tokens.values.at(i+1))); changeMade = 1; break;
        case TOKEN_FACTORIAL: tokens.setData(i,TOKEN_DOUBLE,factorial(static_cast<int>(tokens.values.at(i-1)))); changeMade = 2; break;
        }
        if(changeMade == 1) {
            tokens.removePos(i+1);
            --i;
        }
        else if(changeMade == 2) {
            tokens.removePos(i-1);
            --i;
        }
    }

    //Find multiplications and divisions
    for (unsigned int i = 0; i < tokens.types.size(); ++i) {
        bool changeMade = false;
        switch(tokens.types.at(i)) {
        case TOKEN_MULTIPLY: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1) * tokens.values.at(i+1)); changeMade = true; break;
        case TOKEN_DIVIDE_FLOAT: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1) / tokens.values.at(i+1)); changeMade = true; break;
        case TOKEN_DIVIDE_INT: tokens.setData(i,TOKEN_DOUBLE,static_cast<int>(tokens.values.at(i-1)) / static_cast<int>(tokens.values.at(i+1))); changeMade = true; break;
        case TOKEN_MODULO: tokens.setData(i,TOKEN_DOUBLE,static_cast<int>(tokens.values.at(i-1)) % static_cast<int>(tokens.values.at(i+1))); changeMade = true; break;
        }
        if(changeMade) {
            tokens.removePos(i-1);
            tokens.removePos(i);
            --i;
        }
    }
    
    //And finally addition and subtraction
    for (unsigned int i = 0; i < tokens.types.size(); ++i) {
        bool changeMade = false;
        switch(tokens.types.at(i)) {
        case TOKEN_PLUS: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1) + tokens.values.at(i+1)); changeMade = true; break;
        case TOKEN_MINUS: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1) - tokens.values.at(i+1)); changeMade = true; break;
        }
        if(changeMade) {
            tokens.removePos(i-1);
            tokens.removePos(i);
            --i;
        }
    }

    result = tokens.values.at(0);
    return NO_ERROR;
}

int main()
{
    cout.precision(10);
    cout << ">>> ";

    string sLineIn;
    long double calculatedResult = 0;
    getline(cin, sLineIn);

    while(sLineIn != "exit") {
        stTokens tokenList;        

        if(convertStringToTokens(sLineIn, tokenList, calculatedResult) == NO_ERROR) {
            if(evaluateTokens_rc(tokenList,calculatedResult) == NO_ERROR) {
                //tokenList.dumpData();
                cout << calculatedResult << endl << endl;
            }
        }
        
        cout << ">>> ";
        getline(cin, sLineIn);
    }

    return 0;
}
