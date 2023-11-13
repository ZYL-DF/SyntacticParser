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
        getFirstSet();
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
            if (e1.id == e2.id) {
                return e1.signal < e2.signal;
            } else {
                return e1.id < e2.id;
            }
        }
    };

    vector<string> terminal;
    vector<string> nonTerminal;
    map<int, map<string, string>> grammar;
    map<string, set<expressionPositioner, expressionPositionerFunc>> firstSet;
    map<string, set<expressionPositioner, expressionPositionerFunc>> followSet;

    /**
     * 初始化FirstSet和FollowSet的pair
     * @param str
     * @return pair
     */
    auto generatePair(const string &str) {
        return pair(str, set<expressionPositioner, expressionPositionerFunc>());
    }

    /**
     * 初始化函数
     */
    void init() {
        terminal = {"+", "-", "*", "/", "(", ")", "num"};
        nonTerminal = {"E", "T", "A", "B", "F"};

        grammar = {
                {1,  map<string, string>{{"E", "TA"}}},
                {2,  map<string, string>{{"A", "+TA"}}},
                {3,  map<string, string>{{"A", "-TA"}}},
                {4,  map<string, string>{{"A", "ε"}}},
                {5,  map<string, string>{{"T", "FB"}}},
                {6,  map<string, string>{{"B", "*FB"}}},
                {7,  map<string, string>{{"B", "/FB"}}},
                {8,  map<string, string>{{"B", "ε"}}},
                {9,  map<string, string>{{"F", "(E)"}}},
                {10, map<string, string>{{"F", "num"}}},
        };
        firstSet.insert(generatePair("E"));
        firstSet.insert(generatePair("A"));
        firstSet.insert(generatePair("B"));
        firstSet.insert(generatePair("T"));
        firstSet.insert(generatePair("F"));

        followSet.insert(generatePair("E"));
        followSet.insert(generatePair("A"));
        followSet.insert(generatePair("B"));
        followSet.insert(generatePair("T"));
        followSet.insert(generatePair("F"));
    }

    /**
     * 获得文法的First集合
     */
    void getFirstSet() {

        for (const auto &item: grammar) {
            getDeepFirstSet(item, item);
        }

        for (const auto &item: firstSet) {
            cout << item.first << ":" << endl;
            for (const auto &index: item.second) {
                cout << "(" << index.id << "," << index.signal << ")" << endl;
            }
            cout << endl;
        }
    }

    /**
     * 获得文法的First集合的递归函数
     */
    void getDeepFirstSet(const pair<int, map<string, string>> &item, const pair<int, map<string, string>> &target) {
        auto grammarExpr = item.second;
        string result = findTerminal(grammarExpr.begin()->second);
        if (!result.empty()) {
            // 推导式右侧第一个符号就是终结符，直接加入first集合
            expressionPositioner e{.id = target.first, .signal = result};
            firstSet.find(target.second.begin()->first)->second.insert(e);
            // cout << "add (" << e.id << "," << e.signal << ") into " << target.second.begin()->first << endl;
        } else if (grammarExpr.begin()->second == "ε") {
            // 推导式右侧以ε结尾
            expressionPositioner e{.id = target.first, .signal = "ε"};
            firstSet.find(target.second.begin()->first)->second.insert(e);
        } else {
            // 推导式右侧第一个符号是非终结符，递归以这个非终结符为推导式左侧的所有推导式来建立first集合
            for (const auto &expr: grammar) {
                if (expr.second.begin()->first[0] == grammarExpr.begin()->second[0]) {
                    // cout << "recursive " << grammarExpr.begin()->first << " into " << grammarExpr.begin()->second[0]<< endl;
                    getDeepFirstSet(expr, target);
                }
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

    /**
     * 获得文法的Follow集合
     */
    void getFollowSet() {

    }

};

int main() {
    LL1Parser ll1Parser;
    ll1Parser.parse();
    getchar();
    return 0;
}
