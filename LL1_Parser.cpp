#include <iostream>
#include <vector>
#include <map>
#include <string>
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

    struct expressionPositionerFunc {
        bool operator()(const expressionPositioner &e1, const expressionPositioner &e2) const {
            return e1.id > e2.id;
        }
    };

    vector<string> terminal;
    vector<string> nonTerminal;
    map<string, string> grammar;
    map<string, set<expressionPositioner,expressionPositionerFunc>> firstSet;
    map<string, set<expressionPositioner,expressionPositionerFunc>> followSet;

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
        firstSet.insert(pair<string, set<expressionPositioner>>("E", NULL));
        firstSet.insert(pair<string, set<expressionPositioner>>("A", NULL));
        firstSet.insert(pair<string, set<expressionPositioner>>("B", NULL));
        firstSet.insert(pair<string, set<expressionPositioner>>("T", NULL));
        firstSet.insert(pair<string, set<expressionPositioner>>("F", NULL));

        followSet.insert(pair<string, set<expressionPositioner>>("E", NULL));
        followSet.insert(pair<string, set<expressionPositioner>>("A", NULL));
        followSet.insert(pair<string, set<expressionPositioner>>("B", NULL));
        followSet.insert(pair<string, set<expressionPositioner>>("T", NULL));
        followSet.insert(pair<string, set<expressionPositioner>>("F", NULL));
    }

    map<string, vector<string>> getFirstSet() {
        // 获得文法的First集合
        for (const auto &item: grammar) {
            string result = findTerminal(item.second);
            if (!result.empty()) {
                // 推导式右侧第一个符号就是终结符，直接加入first集合
                firstSet.find(item.first)->second.insert()
            }
        }
    }

    string findTerminal(const string &str) {
        for (const auto &signal: terminal) {
            size_t pos = str.find(signal, 0);
            if (pos == 0) {
                return signal;
            }
        }
        // 第一位不是非终结符
        return "";
    }


public:


};

int main() {
    LL1Parser ll1Parser;
    ll1Parser.parse();
    getchar();
    return 0;
}
