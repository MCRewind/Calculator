#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <cmath>
#include <vector>
#include <set>
#include <map>

#define ALLOW_FOR_IMPLIED_MULTIPLICATION
//#define ALLOW_ZERO_TO_POWER_ZERO

using namespace std;

enum { EMPTY_VALUE, TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE_FLOAT, TOKEN_DIVIDE_INT, TOKEN_POWER, TOKEN_LEFTBRACKET, TOKEN_RIGHTBRACKET, TOKEN_MODULO, TOKEN_DOUBLE,
       TOKEN_STRING, TOKEN_SPACE, TOKEN_SIN, TOKEN_COS, TOKEN_TAN, TOKEN_ABS, TOKEN_FACTORIAL, TOKEN_TO_TYPE, TOKEN_LET, TOKEN_EQUALS, TOKEN_VARIABLE_ESTABLISHED, TOKEN_VARIABLE_NEW,
       TOKEN_IS, TOKEN_GREATER, TOKEN_GREATER_EQ, TOKEN_LESS, TOKEN_LESS_EQ, TOKEN_BECOMES, TOKEN_EQUIVALENT };

enum { NO_ERROR, NO_ERROR_MATH_FUNCTION, NO_ERROR_VAR_ASSIGNMENT, NO_ERROR_COMPARISON, EMPTY_BRACKETS, ERROR_ILLEGAL_STR, ERROR_SECOND_DECIMAL, ERROR_UNEXPECTED_DECIMAL, ERROR_FAULTY_START, ERROR_FAULTY_END,
       ERROR_MISMATCHED_BRACKETS, ERROR_IMPLIED_MULTIPLICATION, ERROR_ZERO_TO_POWER_ZERO, ERROR_MOD_BY_ZERO, ERROR_INVALID_DATA_TO_FUNCTION, ERROR_FACTORIAL_NON_INTEGER,
       ERROR_UNKNOWN_VARIABLE, ERROR_BAD_LET_USAGE, ERROR_BAD_ASSIGNMENT, ERROR_EQUALS_WITHOUT_LET, ERROR_BAD_IS_USAGE, ERROR_9000, FINAL_ERROR };

string errorMessages[FINAL_ERROR];

const double m_pi = 3.14159265358979323846264338327950288419716939937510;
const double m_e =  2.71828182845904523536028747135266249775724709369995;

char a_allowedChars[] = {'+','-','*','/','0','1','2','3','4','5','6','7','8','9','(',')','^','.','%','!'};
set<char> allowedCharsSet (a_allowedChars,a_allowedChars+50);

map<string,int> tokenMap;
map<char,int> operatorMap;
std::map<string,long double> variableMap;
map<string,long double> constMap;
std::vector<string> vecVariablesNames;

union stTokensValues{
    long double v_dbl;
    int v_int;
    string * v_varname;
};

struct stTokens {
       vector<stTokensValues> values;
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
               cout << "#" << i << ": " << types.at(i) << ", " << values.at(i).v_dbl << endl;
           }
           cout << "Dump complete\n";
       }
       void addData(int tokenType, long double value = EMPTY_VALUE) {
           types.push_back(tokenType);
           values.push_back(*(new stTokensValues));
           values.back().v_dbl = value;
       }
       void setData(int pos, int tokenType, long double value) {
           types.at(pos) = tokenType;
           values.at(pos).v_dbl = value;
       }
       void insertData(int pos, int tokenType, long double value = EMPTY_VALUE) {
           types.insert(types.begin()+pos, tokenType);
           values.insert(values.begin()+pos, *(new stTokensValues));
       }
};

inline void setupErrorDictionary() {
    for(unsigned int i = NO_ERROR; i != FINAL_ERROR; ++i) {
        char tempNum [3];
        itoa(i,tempNum,10);
        string stTempNum = tempNum;
        errorMessages[i] = "Error #"+stTempNum+" found, but no message specified yet. Sorry!\n";
    }
    errorMessages[NO_ERROR] = "NO_ERROR: No error found - you shouldn't be able to see this message!\n";
    errorMessages[NO_ERROR_MATH_FUNCTION] = "NO_ERROR_MATH_FUNCTION: No error found - you shouldn't be able to see this message!\n";
    errorMessages[NO_ERROR_VAR_ASSIGNMENT] = "NO_ERROR_VAR_ASSIGNMENT: No error found - you shouldn't be able to see this message!\n";
    errorMessages[EMPTY_BRACKETS] = "There is nothing inside your brackets.\n";
    errorMessages[ERROR_SECOND_DECIMAL] = "A second decimal point in a number was found (like 5.25.6), could not evaluate expression.\n;";
    errorMessages[ERROR_UNEXPECTED_DECIMAL] = "An unexpected decimal point was found, could not evaluate expression.\n";
    errorMessages[ERROR_ILLEGAL_STR] = "An illegal set of characters was found, could not evaluate expression. This could be a random letter or invalid character in your equation.\n";
    errorMessages[ERROR_MISMATCHED_BRACKETS] = "Number of left brackets does not match the number of right brackets.\n";
    errorMessages[ERROR_FAULTY_END] = "The final character (possibly within a bracket) was not a number or closing bracket.\n";
    errorMessages[ERROR_FAULTY_START] = "The first character (possibly within a bracket) was not a number or appropriate function.\n";
    errorMessages[ERROR_ZERO_TO_POWER_ZERO] = "0^0 can not be evaluated, somewhere within your equation.";
    errorMessages[ERROR_FACTORIAL_NON_INTEGER] = "Cannot factorialise a non-integer number.\n";
    errorMessages[ERROR_INVALID_DATA_TO_FUNCTION] = "Invalid data given to a function. (Maybe you need to add brackets?)\n";
    errorMessages[ERROR_MOD_BY_ZERO] = "Cannot modulo by 0.\n";
    errorMessages[ERROR_IMPLIED_MULTIPLICATION] = "Implied multiplication is disallowed, ie 4(5)\n";
    errorMessages[ERROR_UNKNOWN_VARIABLE] = "An unknown variable was entered into the equation.\n";
    errorMessages[ERROR_BAD_LET_USAGE] = "The keyword 'let' was used incorrectly. The form let x = n is required, where x is not a variable (not a constant like pi) and n is a number, constant or variable.\n";
    errorMessages[ERROR_BAD_ASSIGNMENT] = "Bad variable assignment - did you try using a non-existent variable?\n";
    errorMessages[ERROR_EQUALS_WITHOUT_LET] = "You cannot use '=' without a starting 'let' or 'is'.\n";
}

inline void setupTokenMap() {
    operatorMap['+'] = TOKEN_PLUS;
    operatorMap['-'] = TOKEN_MINUS;
    operatorMap['*'] = TOKEN_MULTIPLY;
    operatorMap['/'] = TOKEN_DIVIDE_FLOAT;
    operatorMap['^'] = TOKEN_POWER;
    operatorMap['('] = TOKEN_LEFTBRACKET;
    operatorMap[')'] = TOKEN_RIGHTBRACKET;
    operatorMap['%'] = TOKEN_MODULO;
    operatorMap['!'] = TOKEN_FACTORIAL;
    operatorMap['`'] = TOKEN_SPACE;
    //operatorMap['='] = TOKEN_EQUALS;
    tokenMap["div"] = TOKEN_DIVIDE_INT;
    tokenMap["mod"] = TOKEN_MODULO;
    tokenMap["sin"] = TOKEN_SIN;
    tokenMap["cos"] = TOKEN_COS;
    tokenMap["tan"] = TOKEN_TAN;
    tokenMap["abs"] = TOKEN_ABS;
    tokenMap["let"] = TOKEN_LET;
    tokenMap["is"] = TOKEN_IS;
    tokenMap[":="] = TOKEN_BECOMES;
    tokenMap["=="] = TOKEN_EQUIVALENT;
    tokenMap["<"] = TOKEN_LESS;
    tokenMap["<="] = TOKEN_LESS_EQ;
    tokenMap[">"] = TOKEN_GREATER;
    tokenMap[">="] = TOKEN_GREATER_EQ;
}

inline void setupVariableMap() {
    constMap["e"] = m_e;
    constMap["pi"] = m_pi;
    constMap["todegrees"] = 180/m_pi;
    constMap["toradians"] = m_pi/180;
}

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

string stripSpaces(string input) { //Well, we're not so much stripping them as we are replacing with ` to stop weird code ambiguities.
    string newString;
    for(unsigned int i = 0; i != input.size(); ++i) {
        if(input.at(i) != ' ') {
            newString += input.at(i);
        } else {
            newString += '`';
        }
    }
    return newString;
}

int factorial(int n) { //Hey look, a recursive factorial function, who was expecting that?
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
        else if(sInput.at(i) == '.') { //Account for decimal places in numbers
            if(blNumberString) {
                if(blFoundDecimal) {
                    return ERROR_SECOND_DECIMAL;
                } else {
                    blFoundDecimal = true;
                    stNumber += sInput.at(i);
                }
            } else {
                return ERROR_UNEXPECTED_DECIMAL;
            }
        }
        else {
            //We've found a letter, it could be part of a function. Find the whole word, and then see if it's an allowed word.
            addDoubleFromString(tokenList, blNumberString,blFoundDecimal,stNumber);

            string stWordBuffer;
            while( (i < sInput.length()) ) {
                if( operatorMap.find(sInput.at(i)) != operatorMap.end() ) {
                    if(stWordBuffer.empty() == true){ //If we find a single operator, then it's definitely not part of a function or variable
                        stWordBuffer = sInput.at(i); //So set the wordBuffer to that operator, then perform the standard checks
                        ++i;
                    }
                    break;
                }

                if( !isdigit(sInput.at(i)) ) { //Otherwise, if it's still not a number, continue adding characters to the buffer
                    stWordBuffer += tolower(sInput.at(i));
                    ++i;
                } else {
                    break;
                }

            }
            //cout << stWordBuffer << endl;

            if(stWordBuffer == "ans") { //Special override keywords first
                tokenList.addData(TOKEN_DOUBLE, previousAnswer);
            }
            else if(operatorMap.find(stWordBuffer.at(0)) != operatorMap.end()) { //Check if it's an operator
                tokenList.addData(operatorMap[stWordBuffer.at(0)]); //Use the first character of the string
            }
            else if(tokenMap.find(stWordBuffer) != tokenMap.end()) { //Now see if it's a predefined function
                tokenList.addData(tokenMap[stWordBuffer]);
            }
            else if(constMap.find(stWordBuffer) != constMap.end()) { //Or a constant value
                tokenList.addData(TOKEN_DOUBLE,constMap[stWordBuffer]);
            }
            else if(variableMap.find(stWordBuffer) != variableMap.end()) { //And then check if it's a variable
                unsigned int i = 0;//distance(variableMap.begin(), variableMap.find(stWordBuffer));
                for(; i != vecVariablesNames.size(); ++i) {
                    if(vecVariablesNames.at(i) == stWordBuffer) {
                        break;
                    }
                }
                tokenList.addData(TOKEN_VARIABLE_ESTABLISHED,i);
            }
            else {
                tokenList.addData(TOKEN_VARIABLE_NEW,vecVariablesNames.size());
                vecVariablesNames.push_back(stWordBuffer);
                variableMap[stWordBuffer] = 0;
            }

            continue;
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
            //cout << "DELETING SPACE\n";
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
        return ERROR_MISMATCHED_BRACKETS;
    }

#ifdef ALLOW_FOR_IMPLIED_MULTIPLICATION
    //Insert * between brackets and numbers (or brackets)
    for (unsigned int i = 1; i < tokenList.types.size(); ++i) {
        if( (tokenList.types.at(i-1) == TOKEN_DOUBLE) && (tokenList.types.at(i) == TOKEN_LEFTBRACKET) ) tokenList.insertData(i,TOKEN_MULTIPLY);
        if( (tokenList.types.at(i-1) == TOKEN_RIGHTBRACKET) && (tokenList.types.at(i) == TOKEN_DOUBLE) ) tokenList.insertData(i,TOKEN_MULTIPLY);
        if( (tokenList.types.at(i-1) == TOKEN_RIGHTBRACKET) && (tokenList.types.at(i) == TOKEN_LEFTBRACKET) ) tokenList.insertData(i,TOKEN_MULTIPLY);
    }
#else
    //Throw an error
    for (unsigned int i = 1; i < tokenList.types.size(); ++i) {
        if( (tokenList.types.at(i-1) == TOKEN_DOUBLE) && (tokenList.types.at(i) == TOKEN_LEFTBRACKET) ) { cout << "Implied multiplication of brackets is disallowed.\n"; return ERROR_IMPLIED_MULTIPLICATION; }
        if( (tokenList.types.at(i-1) == TOKEN_RIGHTBRACKET) && (tokenList.types.at(i) == TOKEN_DOUBLE) ) { cout << "Implied multiplication of brackets is disallowed.\n"; return ERROR_IMPLIED_MULTIPLICATION; }
        if( (tokenList.types.at(i-1) == TOKEN_RIGHTBRACKET) && (tokenList.types.at(i) == TOKEN_LEFTBRACKET) ) { cout << "Implied multiplication of brackets is disallowed.\n"; return ERROR_IMPLIED_MULTIPLICATION; }
    }
#endif

    //tokenList.dumpData();
    //cout << "Converted string '" << sInput << "' to tokens successfully.\n";
    return NO_ERROR;
}

long double* getVariable(string variableName) {
    return &variableMap[variableName];
}

long double* getVariable(long double variablePos) {
    return &variableMap[vecVariablesNames.at(variablePos)];
}

int evaluateTokens_rc(stTokens &tokens, long double &result) {

    if(tokens.values.size() == 0) { result = 0; return EMPTY_BRACKETS; }

    //First let's see whether we're going to be performing a maths equation or a programming construct - like assigning a variable
    if( tokens.types.front() == TOKEN_LET) {
        //Assigning a variable, tokens should be in the form <assignment> ::= <variable> <equals> <number>|<variable>
        if (tokens.types.size() == 4) {

            if( ( (tokens.types.at(1) == TOKEN_VARIABLE_NEW) || (tokens.types.at(1) == TOKEN_VARIABLE_ESTABLISHED) ) && (tokens.types.at(2) == TOKEN_BECOMES) ) {

                if(tokens.types.at(3) == TOKEN_DOUBLE) {
                    *getVariable(tokens.values.at(1).v_dbl) = tokens.values.at(3).v_dbl;
                    return NO_ERROR_VAR_ASSIGNMENT;
                }
                else if(tokens.types.at(3) == TOKEN_VARIABLE_ESTABLISHED) {
                    *getVariable(tokens.values.at(1).v_dbl) = *getVariable(tokens.values.at(3).v_dbl);
                    return NO_ERROR_VAR_ASSIGNMENT;
                }
                else {
                    return ERROR_BAD_ASSIGNMENT;
                }

            } else {
                cout << "a\n";
                return ERROR_BAD_LET_USAGE;
            }
        }
        else {
            cout << "b\n";
            return ERROR_BAD_LET_USAGE;
        }
    }
    else if( tokens.types.front() == TOKEN_IS) {
        //Comparing values, tokens should be in the form <comparison> ::= <number>|<variable> <equals> <number>|<variable>
        if (tokens.types.size() == 4) {
            if( tokens.types.at(1) == TOKEN_VARIABLE_ESTABLISHED ) { tokens.setData(1,TOKEN_DOUBLE,*getVariable(tokens.values.at(1).v_dbl)); }
            if( tokens.types.at(3) == TOKEN_VARIABLE_ESTABLISHED ) { tokens.setData(3,TOKEN_DOUBLE,*getVariable(tokens.values.at(1).v_dbl)); }

            if (!( (tokens.types.at(1) == TOKEN_DOUBLE) && (tokens.types.at(3) == TOKEN_DOUBLE) )) {
                return ERROR_BAD_IS_USAGE;
            }

            switch(tokens.types.at(2)) {
            case TOKEN_EQUIVALENT: if(tokens.values.at(1).v_dbl == tokens.values.at(3).v_dbl) { result = 1; } else { result = 0; } break;
            case TOKEN_GREATER: if(tokens.values.at(1).v_dbl > tokens.values.at(3).v_dbl) { result = 1; } else { result = 0; } break;
            case TOKEN_LESS: if(tokens.values.at(1).v_dbl < tokens.values.at(3).v_dbl) { result = 1; } else { result = 0; } break;
            case TOKEN_GREATER_EQ: if(tokens.values.at(1).v_dbl >= tokens.values.at(3).v_dbl) { result = 1; } else { result = 0; } break;
            case TOKEN_LESS_EQ: if(tokens.values.at(1).v_dbl <= tokens.values.at(3).v_dbl) { result = 1; } else { result = 0; } break;
            }

            return NO_ERROR_COMPARISON;
        }
        else {
            return ERROR_BAD_IS_USAGE;
        }
    }
    else {

        //Add a leading 0 to starting + or - tokens
        if( (tokens.types.front() == TOKEN_PLUS) || (tokens.types.front() == TOKEN_MINUS) ) {
            tokens.insertData(0,TOKEN_DOUBLE,0);
        }

        //Replace all variables with their values
        int unknownCount = 0;
        for (unsigned int i = 0; i < tokens.types.size(); ++i) {
            if(tokens.types.at(i) == TOKEN_VARIABLE_ESTABLISHED) {
                if(variableMap.find(vecVariablesNames.at(tokens.values.at(i).v_dbl)) != variableMap.end()) {
                    tokens.setData(i,TOKEN_DOUBLE,variableMap[vecVariablesNames.at(tokens.values.at(i).v_dbl)]);
                }
            }
            else if(tokens.types.at(i) == TOKEN_VARIABLE_NEW) {
                //Just clear out this variable's place in the variableNames and variableMap lists
                string tempName = vecVariablesNames.at(tokens.values.at(i-unknownCount).v_dbl);
                vecVariablesNames.erase(vecVariablesNames.begin()+ tokens.values.at(i-unknownCount).v_dbl);
                variableMap.erase(tempName);
                ++unknownCount;
            }
        }
        if(unknownCount != 0) return ERROR_UNKNOWN_VARIABLE;

        //Check we don't have any TOKEN_EQUALS without a starting TOKEN_LET
        if(tokens.types.front() != TOKEN_LET) {
            for (unsigned int i = 0; i != tokens.types.size(); ++i) {
                if(tokens.types.at(i) == TOKEN_EQUALS) return ERROR_EQUALS_WITHOUT_LET;
            }
        }

        if (! ((tokens.types.back() == TOKEN_DOUBLE) || (tokens.types.back() == TOKEN_RIGHTBRACKET) || (tokens.types.back() == TOKEN_FACTORIAL)) ) {
            result = 0;
            return ERROR_FAULTY_END;
        }
        if (! ((tokens.types.front() == TOKEN_DOUBLE) || (tokens.types.front() == TOKEN_LEFTBRACKET) || (tokens.types.front() == TOKEN_SIN) || (tokens.types.front() == TOKEN_COS)
               || (tokens.types.front() == TOKEN_TAN) || (tokens.types.front() == TOKEN_ABS)
               ) ) {
            result = 0;
            return ERROR_FAULTY_START;
        }

        //Check for brackets, and recursively evaluate anything inside them
        int leftBracketsFound = 0;
        int rightBracketsFound = 0;
        stTokens smallerEvaluation;
        int posLeftBracket = 0, posRightBracket = 0;
        for (unsigned int i = 0; i < tokens.types.size(); ++i) {

            if(leftBracketsFound > 0) {
                smallerEvaluation.types.push_back(tokens.types.at(i));
                smallerEvaluation.values.push_back(tokens.values.at(i));
            }

            if(tokens.types.at(i) == TOKEN_LEFTBRACKET) {
                ++leftBracketsFound;
                cout << leftBracketsFound << " left brackets found.\n";
                if(leftBracketsFound == 1)
                    posLeftBracket = i;
            }
            else if(tokens.types.at(i) == TOKEN_RIGHTBRACKET) {
                ++rightBracketsFound;
                cout << rightBracketsFound << " right brackets found.\n";
                if( (leftBracketsFound == rightBracketsFound) && (rightBracketsFound > 0) ) {
                    posRightBracket = i;
                    //Remove the brackets and contents
                    tokens.removePos(posLeftBracket, posRightBracket);
                    smallerEvaluation.removePos(smallerEvaluation.types.size()-1);
                    //Evaluate the brackets and insert that into the tokens
                    long double dblSmallerEvaluate;
                    int rsltSmallerEvaluate = evaluateTokens_rc(smallerEvaluation, dblSmallerEvaluate);
                    if (!( (rsltSmallerEvaluate == NO_ERROR_MATH_FUNCTION) || (rsltSmallerEvaluate == NO_ERROR) )) {
                        //There was an error found, so return that error code.
                        return rsltSmallerEvaluate;
                    }

                    tokens.types.at(posLeftBracket) = TOKEN_DOUBLE;
                    tokens.values.at(posLeftBracket).v_dbl = dblSmallerEvaluate;

                    tokens.dumpData();

                    i = 0;
                    smallerEvaluation.values.clear();
                    smallerEvaluation.types.clear();

                    leftBracketsFound = 0;
                    rightBracketsFound = 0;
                }
            }

        }

        //Now run functions, like sin, cos
        for (unsigned int i = 0; i < tokens.types.size(); ++i) {
            int changeMade = 0;
            switch(tokens.types.at(i)) {
            case TOKEN_SIN: tokens.setData(i,TOKEN_DOUBLE,sin(tokens.values.at(i+1).v_dbl)); changeMade = 1; break;
            case TOKEN_COS: tokens.setData(i,TOKEN_DOUBLE,cos(tokens.values.at(i+1).v_dbl)); changeMade = 1; break;
            case TOKEN_TAN: tokens.setData(i,TOKEN_DOUBLE,tan(tokens.values.at(i+1).v_dbl)); changeMade = 1; break;
            case TOKEN_ABS: tokens.setData(i,TOKEN_DOUBLE,abs(tokens.values.at(i+1).v_dbl)); changeMade = 1; break;
            case TOKEN_FACTORIAL:
                if( (static_cast<int>(tokens.values.at(i-1).v_dbl)) != tokens.values.at(i-1).v_dbl ) {
                    return ERROR_FACTORIAL_NON_INTEGER;
                }
                tokens.setData(i,TOKEN_DOUBLE,factorial(static_cast<int>(tokens.values.at(i-1).v_dbl))); changeMade = 2; break;
            }
            if(changeMade == 1) {
                if(tokens.types.at(i+1) != TOKEN_DOUBLE) {
                    return ERROR_INVALID_DATA_TO_FUNCTION;
                }
                tokens.removePos(i+1);
                --i;
            }
            else if(changeMade == 2) {
                if(tokens.types.at(i-1) != TOKEN_DOUBLE) {
                    return ERROR_INVALID_DATA_TO_FUNCTION;
                }
                tokens.removePos(i-1);
                --i;
            }
        }

        //Find carets for exponents evaluation
        for (int i = tokens.types.size() - 1; i >= 0; --i) {
            switch(tokens.types.at(i)) {
            case TOKEN_POWER:
                long double base = tokens.values.at(i-1).v_dbl;
                long double exp = tokens.values.at(i+1).v_dbl;
#ifndef ALLOW_ZERO_TO_POWER_ZERO
                if ((base == exp) && (base == 0)) {
                    return ERROR_ZERO_TO_POWER_ZERO;
                }
#endif
                tokens.setData(i,TOKEN_DOUBLE,pow(base,exp)); tokens.removePos(i-1); tokens.removePos(i); --i; break;
            }
        }

        //Find multiplications and divisions
        for (unsigned int i = 0; i < tokens.types.size(); ++i) {
            bool changeMade = false;
            switch(tokens.types.at(i)) {
            case TOKEN_MULTIPLY: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1).v_dbl * tokens.values.at(i+1).v_dbl); changeMade = true; break;
            case TOKEN_DIVIDE_FLOAT: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1).v_dbl / tokens.values.at(i+1).v_dbl); changeMade = true; break;
            case TOKEN_DIVIDE_INT: tokens.setData(i,TOKEN_DOUBLE,static_cast<int>(tokens.values.at(i-1).v_dbl) / static_cast<int>(tokens.values.at(i+1).v_dbl)); changeMade = true; break;
            case TOKEN_MODULO:
                if( (static_cast<int>(tokens.values.at(i+1).v_dbl)) == 0) {
                    return ERROR_MOD_BY_ZERO;
                }
                tokens.setData(i,TOKEN_DOUBLE,static_cast<int>(tokens.values.at(i-1).v_dbl) % static_cast<int>(tokens.values.at(i+1).v_dbl)); changeMade = true;
                break;
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
            case TOKEN_PLUS: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1).v_dbl + tokens.values.at(i+1).v_dbl); changeMade = true; break;
            case TOKEN_MINUS: tokens.setData(i,TOKEN_DOUBLE,tokens.values.at(i-1).v_dbl - tokens.values.at(i+1).v_dbl); changeMade = true; break;
            }
            if(changeMade) {
                tokens.removePos(i-1);
                tokens.removePos(i);
                --i;
            }
        }

        result = tokens.values.at(0).v_dbl;
        return NO_ERROR_MATH_FUNCTION;
    }

}

int main()
{
    cout.precision(10);
    setupErrorDictionary();
    setupVariableMap();
    setupTokenMap();
    cout << "Calculator program by Chris Winward. Type 'help' for instructions.\n";
    cout << ">>> ";

    string sLineIn;
    long double calculatedResult = 0;
    getline(cin, sLineIn);

    while(sLineIn != "exit") {
        if(sLineIn == "help") {
            cout << "This calculator uses BFIDMAS to calculate results - Brackets, Functions, Indices, Division & Multiplication, Addition & Subtraction.\n\n";
            cout << "The following single digit operators are available:\n";
            cout << "* / + - ^ % ( ) !\n";
            cout << "/ refers to real division, ^ is raise to power, % is modulo operando, and ! is factorial.\n\n";
            cout << "The following functions are available:\n";
            cout << "div mod sin cos tan abs\n";
            cout << "div refers to integer division. All trigonometric functions take input in radians.\n\n";
            cout << "The following constants are available:\n\n";
            cout << "e = 2.718... pi = 3.141... todegrees = 57.296... toradians = 0.0174...\n";
            cout << "The word 'ans' can be used to access the last equations answer.\n";
            cout << "Type 'exit' to close the program.\n";
            cout << endl;
        } else {
            stTokens tokenList;
            int errorMessage = convertStringToTokens(sLineIn, tokenList, calculatedResult);
            if(errorMessage == NO_ERROR) {
                errorMessage = evaluateTokens_rc(tokenList,calculatedResult);
                switch(errorMessage) {
                case NO_ERROR_VAR_ASSIGNMENT: break;
                case NO_ERROR_MATH_FUNCTION: cout << endl << ">>>>>> " << calculatedResult << endl; break;
                case NO_ERROR_COMPARISON: cout << endl << ">>>>>> " << (bool)calculatedResult << endl; break;
                default: cout << errorMessages[errorMessage]; break;
                }

            } else {
                cout << errorMessages[errorMessage];
            }
        }

        cout << ">>> ";
        getline(cin, sLineIn);
    }

    return 0;
}
