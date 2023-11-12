#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace std;

/*
1	E -> TA
2	A -> +TA
3	A -> -TA
4	A -> ε
5	T -> FB
6	B -> *FB
7	B -> /FB
8	B -> ε
9	F -> (E)
10	F -> num
*/
class LL1Parser {
public:
    LL1Parser() = default;

    void parse() {
        // 语法解析器主函数
        init();
        // getFirstSet();
        // getFollowSet();
        // generateTable();
        // analyse();
        // printResult();
    }

private:
    struct expressionPositioner {
        int id;
        // 对应文法句子的id
        string signal;
        // 对应文法句子得到的FIRST或FOLLOW集合中的符号
    };
    vector<string> terminal = {"+", "-", "*", "/", "(", ")", "num"};
    vector<string> nonTerminal = {"E",
                                  "T",
                                  "A",
                                  "B",
                                  "F"};
    map<string, string> grammar;
    map<string, vector<expressionPositioner>> firstSet;
    map<string, vector<expressionPositioner>> followSet;

    void init() {
        terminal = {"+", "-", "*", "/", "(", ")", "num"};
        nonTerminal = {"E", "T", "A", "B", "F"};
        grammar = {
                {"E", "TA"},
                {"A", "+TA"},
                {"A", "-TA"},
                {"A", "ε"},
                {"T", "FB"},
                {"B", "*FB"},
                {"B", "/FB"},
                {"B", "ε"},
                {"F", "(E)"},
                {"F", "num"},
        };
        firstSet.insert(pair<string, vector<expressionPositioner>>("E", NULL));
        firstSet.insert(pair<string, vector<expressionPositioner>>("A", NULL));
        firstSet.insert(pair<string, vector<expressionPositioner>>("B", NULL));
        firstSet.insert(pair<string, vector<expressionPositioner>>("T", NULL));
        firstSet.insert(pair<string, vector<expressionPositioner>>("F", NULL));

        followSet.insert(pair<string, vector<expressionPositioner>>("E", NULL));
        followSet.insert(pair<string, vector<expressionPositioner>>("A", NULL));
        followSet.insert(pair<string, vector<expressionPositioner>>("B", NULL));
        followSet.insert(pair<string, vector<expressionPositioner>>("T", NULL));
        followSet.insert(pair<string, vector<expressionPositioner>>("F", NULL));
    }

    map<string, vector<string>> getFirstSet() {
        // 获得文法的First集合
        for(auto item : grammar) {
            auto position = find(terminal.begin(),terminal.end(),item.second.at(0));

        }
    }


public:


};

int main() {
    LL1Parser ll1Parser;
    ll1Parser.parse();
    getchar();
    return 0;
}
