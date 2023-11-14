#include <iostream>
#include <utility>
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
    map<int, map<string, string>> unsetGrammar;

    explicit LL1Parser(map<int, map<string, string>> grammar) {
        unsetGrammar = std::move(grammar);
    };

    void parse() {
        // 语法解析器主函数
        init();
        getFirstSet();
        getFollowSet();
        // generateTable();
        // analyse();
        // printResult();
    }

private:
    struct ExpressionPositioner {

        int id;
        // 对应文法句子的id
        string signal;
        // 对应文法句子得到的FIRST或FOLLOW集合中的符号
    };

    struct expressionPositionerFunc {
        bool operator()(const ExpressionPositioner &e1, const ExpressionPositioner &e2) const {
            if (e1.id == e2.id) {
                return e1.signal < e2.signal;
            } else {
                return e1.id < e2.id;
            }
        }
    };

    struct Expression {

        Expression(int id, string start, const vector<string> &anEnd) : id(id), start(std::move(start)), end(anEnd) {}

        int id;
        // 语法推导式id
        string start;
        // 语法推导式左边的符号
        vector<string> end;
        // 语法推导式右边的符号集
    };

    vector<string> terminal;
    vector<string> nonTerminal;
    // map<int, map<string, string>> grammar;
    vector<Expression> grammar;
    map<string, set<ExpressionPositioner, expressionPositionerFunc>> firstSet;
    map<string, set<ExpressionPositioner, expressionPositionerFunc>> followSet;

    /**
     * 初始化FirstSet和FollowSet的pair
     * @param str
     * @return pair
     */
    auto generatePair(const string &str) {
        return pair(str, set<ExpressionPositioner, expressionPositionerFunc>());
    }

    auto generateSignalSet(const string& sentence,const vector<string>& wordList) {
        string sentenceCopy = sentence;
        vector<string> result;
        while(!sentenceCopy.empty()) {
            for(const auto& word:wordList) {
                if(sentenceCopy.find(word) == 0){
                    result.push_back(word);
                    sentenceCopy = sentenceCopy.substr(word.length());
                }
            }
        }
        return result;
    }

    /**
     * 初始化函数
     */
    void init() {
        terminal = {"+", "-", "*", "/", "(", ")", "num"};
        nonTerminal = {"E", "T", "A", "B", "F"};

        vector<string> signalSet;// 全部符号集
        merge(terminal.begin(), terminal.end(), nonTerminal.begin(), nonTerminal.end(), std::back_inserter(signalSet));
        signalSet.emplace_back("ε");
        for (auto sentence: unsetGrammar) {
            Expression expression(sentence.first, sentence.second.begin()->first, generateSignalSet(sentence.second.begin()->second,signalSet));
            grammar.push_back(expression);
        }

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

        ExpressionPositioner e{.id = 1, .signal = "$"};
        followSet.find("E")->second.insert(e);
    }

    /**
     * 获得文法的First集合
     */
    void getFirstSet() {

        for (const auto &item: grammar) {
            getDeepFirstSet(item, item, 0);
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
    void getDeepFirstSet(const Expression &item, const Expression &father,
                         int itemPos) {
        auto grammarExpr = item.end;
        string result = findTerminal(item.end);
        if (!result.empty()) {
            // 推导式右侧第一个符号就是终结符，直接加入first集合
            addFirstSet(father.id, father.start, result);

        } else if (item.end[0] == "ε") {
            // 推导式右侧以ε结尾
            // 只能给自己的推导式的左侧First集加入,不能给父推导式First集加入
            addFirstSet(item.id, item.start, "ε");

        } else {
            // 推导式右侧第一个符号是非终结符，递归以这个非终结符为推导式左侧的所有推导式来建立first集合
            for (const auto &expr: grammar) {
                if (expr.start == item.end[itemPos]) {

                    getDeepFirstSet(expr, item, itemPos);
                    for (const auto &expr_first: firstSet.find(expr.start)->second) {
                        // 将子非终结符已经构建的First集合,加入到父非终结符的First集合
                        if (expr_first.signal != "ε") {
                            // 如果有子推导式没有空串的推导,直接将子推导式的First集合加入到父推导式的First集合
                            addFirstSet(item.id, item.start, expr_first.signal);
                        }/* else {
                            // 如果有子推导式有空串的推导
                            if (itemPos == grammarExpr.begin()->second.length() - 1) {
                                // 已经是最后一个子推导式还是有空串,那么父推导式也有空串推导
                                addFirstSet(item.first, item.second.begin()->first, "ε");
                            } else {
                                // 否则,进行后面一个符号的判断
                                string next = findTerminal(&(grammarExpr.begin()->second[itemPos + 1]));
                                if(next.empty()) {
                                    // 非终结符,需要递归

                                    const pair<int, map<string, string>> tempPair();
                                    getDeepFirstSet(tempPair, item, itemPos + 1);
                                } else {
                                    // 终结符,父推导式FIRST集合加入该终结符
                                    addFirstSet(item.first, item.second.begin()->first, next);
                                }
                            }
                        }*/
                    }
                }
            }
        }
    }


    string findTerminal(const vector<string>& end) {
        for (const auto &signal: terminal) {
            if (signal == end[0]) {
                return signal;
            }
        }
        // 第一位不是非终结符
        return "";
    }

    void findNonTerminal(const string &str, int &nonTerminalPos, string &result) {
        map<int, string> findPos;
        for (const auto &signal: nonTerminal) {
            size_t pos = str.find(signal, 0);
            if (pos != -1) {
                findPos.insert(pair<int, string>(pos, signal));
            }
        }
        if (!findPos.empty()) {
            nonTerminalPos = findPos.begin()->first;
            result = findPos.begin()->second;
        } else {
            result = "";
        }
    }

    /**
     * 获得文法的Follow集合
     */
    void getFollowSet() {
        for (const auto &item: grammar) {

        }
    }

    void addFirstSet(int id, const string &target, string result) {
        ExpressionPositioner e{.id = id, .signal = std::move(result)};
        firstSet.find(target)->second.insert(e);
    }
};

int main() {
    map<int, map<string, string>> grammar = {
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
    LL1Parser ll1Parser(grammar);
    ll1Parser.parse();
    getchar();
    return 0;
}
