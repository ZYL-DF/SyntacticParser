#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <stack>
#include <algorithm>

using namespace std;

/*
0	E' -> E
1	E -> E+T
2	E -> E-T
3	E -> T
4	T -> T*F
5	T -> T/F
6	T -> F
7	F -> (E)
8	F -> num
*/
struct Expression {

    Expression(int id, string start, const vector<string> &anEnd) : id(id), start(std::move(start)), end(anEnd) {}

    int id;
    // 语法推导式id
    string start;
    // 语法推导式左边的符号
    vector<string> end;
    // 语法推导式右边的符号集

    bool operator==(const Expression &expression) const {
        bool idEqual = (id == expression.id);
        bool startEqual = (start == expression.start);
        bool endEqual = (end == expression.end);
        return idEqual && startEqual && endEqual;
    }
};

class LR1Parser {
public:
    map<int, map<string, string>> unsetGrammar;

    explicit LR1Parser(map<int, map<string, string>> inputGrammar) {
        unsetGrammar = std::move(inputGrammar);
    };

    void parse(const string &str) {
        // 语法解析器主函数
        init();
        // analyse(str);
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

    class DFANode {
    public:
        explicit DFANode(int id) {
            this->id = id;
        }

        struct Item {
            Expression expression; // 推导式
            int point; // 点
            vector<string> lookahead; // 向前看符号集

            bool operator==(const Item &item) const {
                bool expressEqual = (expression == item.expression);
                bool pointEqual = (point == item.point);
                bool lookaheadEqual = (lookahead == item.lookahead);
                return expressEqual && pointEqual && lookaheadEqual;
            }
        };

        int id;                         // 项目集规范族序号
        map<string, DFANode *> previous;  // 能推导出当前规范族的规范族
        map<string, DFANode *> next;      // 当前规范族能推导出的规范族
        set<Item> itemList;          // 项目集

        bool operator==(const DFANode &d) const {
            bool idEqual = (id == d.id);
            bool itemListEqual = (itemList == d.itemList);
            return idEqual && itemListEqual;
        }


        void getClosure();
    };

    static vector<string> terminal;
    static vector<string> nonTerminal;
    static vector<Expression> grammar;
    static set<DFANode> dfa;

    vector<string> generateSignalSet(const string &sentence, const vector<string> &wordList) {
        string sentenceCopy = sentence;
        vector<string> result;
        while (!sentenceCopy.empty()) {
            for (const auto &word: wordList) {
                if (sentenceCopy.find(word) == 0) {
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
        nonTerminal = {"E'", "E", "T", "F"};

        vector<string> signalSet(terminal.size() + nonTerminal.size());// 全部符号集
        merge(terminal.begin(), terminal.end(), nonTerminal.begin(), nonTerminal.end(), signalSet.begin());

        for (auto sentence: unsetGrammar) {
            Expression expression(sentence.first, sentence.second.begin()->first,
                                  generateSignalSet(sentence.second.begin()->second, signalSet));
            grammar.push_back(expression);
        }
        DFANode dfaNode(0);
        Expression expression = grammar[0];
        vector<string> lookaheadList;
        lookaheadList.emplace_back("$");
        DFANode::Item item(expression, 0, lookaheadList);
        dfaNode.itemList.insert(item);
        dfa.insert(dfaNode, [](DFANode &dfa1, DFANode &dfa2) { return dfa1 == dfa2; });
    }

    Expression getExpression(int id) {
        for (auto expression: grammar) {
            if (expression.id == id) {
                return expression;
            }
        }
        cout << "Expression id is out of range" << endl;
        exit(-1);
    }

    void generateDFA();
};

void LR1Parser::DFANode::getClosure() {
    bool isChanged = true;
    while (isChanged) {
        isChanged = false;
        auto itemListPrevious = itemList;
        for (const auto &item: itemList) {
            if (item.point < itemList.size()) {
                string currentSignal = item.expression.end[item.point];
                if (std::find(nonTerminal.begin(), nonTerminal.end(),
                              currentSignal) != nonTerminal.end()) {
                    // 圆点后面是非终结符,需要扩展闭包
                    for (const auto &expression: grammar) {
                        // 对文法中每条以该非终结符为左侧的句子创建项目
                        if (expression.start == currentSignal) {
                            Item newItem(expression, 0, item.lookahead);
                            itemList.insert(newItem);
                            // todo:newItem的lookahead应为FIRST(item的后缀式+item.lookahead)
                        }
                    }
                }
            }
        }
        if (itemListPrevious != itemList) {
            isChanged = true;
        }
    }
}

void LR1Parser::generateDFA() {
    /*
     * 要生成DFA,需要固定执行以下步骤
     * 1.从E'->E开始,通过求闭包构建项目集规范族
     * 2.对当前项目集规范族的每一个移进/待约项目,对圆点后的符号,进行项目集推导
     *   如果推导的结果项目集已经存在,当前项目集的next集合和结果项目集的previous集要增加
     */
    for (DFANode dfaNode: dfa) {
        dfaNode.getClosure();
    }
}


int main() {
    map<int, map<string, string>> grammar = {
            {0, map<string, string>{{"E'", "E"}}},
            {1, map<string, string>{{"E", "E+T"}}},
            {2, map<string, string>{{"E", "E-T"}}},
            {3, map<string, string>{{"E", "T"}}},
            {4, map<string, string>{{"T", "T*F"}}},
            {5, map<string, string>{{"T", "T/F"}}},
            {6, map<string, string>{{"T", "F"}}},
            {7, map<string, string>{{"F", "(E)"}}},
            {8, map<string, string>{{"F", "num"}}},
    };
    LR1Parser lr1Parser(grammar);
    string str;
    // cin >> str;
    lr1Parser.parse(str);
    getchar();
    getchar();
    return 0;
}
