#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <unordered_map>
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
        return idEqual;
    }

    bool operator<(const Expression &expression) const {
        bool idLess = (id < expression.id);
        return idLess;
    }
};

struct Item {
    Expression expression; // 推导式
    int point; // 点
    mutable set<string> lookahead; // 向前看符号集

    Item(Expression expression, int point, const set<string> &lookahead) : expression(std::move(expression)),
                                                                           point(point),
                                                                           lookahead(lookahead) {}

    bool operator==(const Item &item) const {
        bool expressEqual = (expression == item.expression);
        bool pointEqual = (point == item.point);
        bool lookaheadEqual = (lookahead == item.lookahead);
        return expressEqual && pointEqual && lookaheadEqual;
    }

    bool operator<(const Item &item) const {
        bool expressLess = (expression < item.expression);
        bool pointLess = (point < item.point);
        bool lookaheadLess = (lookahead < item.lookahead);
        if (expression == item.expression) {
            if (point == item.point) {
                return lookaheadLess;
            } else {
                return pointLess;
            }
        } else {
            return expressLess;
        }
    }
};

class DFANode {
public:
    int id;                         // 项目集规范族序号
    set<Item> itemList;          // 项目集
    map<string, DFANode *> previous;  // 能推导出当前规范族的规范族
    map<string, DFANode *> next;      // 当前规范族能推导出的规范族

    DFANode() {
        id = -1;
    }

    explicit DFANode(int id) {
        this->id = id;
    }

    DFANode(int id, const map<string, DFANode *> &previous, const map<string, DFANode *> &next,
            const set<Item> &itemList) : id(id), previous(previous), next(next), itemList(itemList) {}

    bool operator==(const DFANode &d) const {

        bool itemListEqual = (itemList == d.itemList);

        return itemListEqual;
    }

    bool operator<(const DFANode &d) const {

        bool itemListEqual = (itemList < d.itemList);

        return itemListEqual;
    }

    void getClosure(const vector<string> &nonTerminal, const vector<Expression> &grammar);

    void findNextNode(vector<DFANode*> &dfa, int &nodeNum,vector<string> &nonTerminal,vector<Expression> &grammar);

    set<string> getFirstSet(Item item, const vector<string> &nonTerminal, const vector<Expression> &grammar);

    void getDeepFirstSet(set<string> &result, const string &str, const vector<string> &nonTerminal,
                         const vector<Expression> &grammar);

    bool mergeLookahead(const Item &newItem);
};

void DFANode::getClosure(const vector<string> &nonTerminal, const vector<Expression> &grammar) {
    bool isChanged = true;
    while (isChanged) {
        isChanged = false;
        auto itemListPrevious = itemList;
        for (const auto &item: itemList) {
            if (item.point < item.expression.end.size()) {
                string currentSignal = item.expression.end[item.point];
                if (std::find(nonTerminal.begin(), nonTerminal.end(),
                              currentSignal) != nonTerminal.end()) {
                    // 圆点后面是非终结符,需要扩展闭包
                    for (const auto &expression: grammar) {
                        // 对文法中每条以该非终结符为左侧的句子创建项目
                        if (expression.start == currentSignal) {
                            Item newItem(expression, 0, getFirstSet(item, nonTerminal, grammar));
                            // 检查项目集中是否存在除了向前看符号集以外其他属性全部一致的项目,如果有,合并向前看符号集
                            // 否则视为新的项目加入项目集中
                            if (!mergeLookahead(newItem)) {
                                itemList.insert(newItem);
                            }
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

/**
 * 创建新的DFA节点,也就是状态集
 * @param nonTerminal
 * @param grammar
 * @param dfa
 */
void DFANode::findNextNode(vector<DFANode*> &dfa, int &nodeNum,vector<string> &nonTerminal,vector<Expression> &grammar) {

    vector<DFANode *> dfaReady; // 候选项目集的集合,里面的项目集还未与已经创建的项目集进行比较
    for (const auto &item: itemList) {
        if (item.point < item.expression.end.size()) {
            // 不对归约项目进行构建
            Item newItem(item.expression, item.point + 1, item.lookahead);
            auto iterator = next.find(item.expression.end[item.point]);
            if (iterator == next.end()) {
                // 如果项目集对这个符号的后续项目集不存在,需要创建新的项目集
                // DFANode newDFANode((int) (dfa.size() + dfaReady.size()));
                auto *newDFANode = new DFANode((int) (dfa.size() + dfaReady.size()));
                // newDFANode.previous.insert(map<string, DFANode>::value_type(item.expression.end[item.point], *this));
                newDFANode->previous[item.expression.end[item.point]] = this;
                newDFANode->itemList.insert(newItem);
                // next.insert(map<string, DFANode>::value_type(item.expression.end[item.point], newDFANode));
                next[item.expression.end[item.point]] = newDFANode;
                dfaReady.push_back(next[item.expression.end[item.point]]);
            } else {
                // 如果项目集对这个符号的后续项目集存在,向这个后续项目集加入新的项目
                if (!dfaReady.empty()) {
                    iterator->second->itemList.insert(newItem);
                    // next.find(item.expression.end[item.point])->second.itemList.insert(newItem);
                }
            }
        }
    }
    auto selfNext = this->next;

    for (auto node: dfaReady) {
        DFANode * tempNode = nullptr;
        for(auto iterator :dfa) {
            node->getClosure(nonTerminal,grammar);
            if(*iterator == *node) {
                tempNode = iterator;
                break;
            }
        }
        // auto iterator = std::find(dfa.begin(), dfa.end(), node);
        // if (iterator == dfa.end()) {
        if (tempNode == nullptr) {
            // 当前dfa中没有这个项目集
            node->id = nodeNum;
            nodeNum++;
            dfa.push_back(node);
        } else {
            // 否则,直接将当前项目集对应符号的next指向已经存在的项目集
            // next.find(node->previous.begin()->first)->second = *iterator;
            // selfNext[node->previous.begin()->first] = *iterator;
            // this->next[node->previous.begin()->first] = *iterator;
            this->next[node->previous.begin()->first] = tempNode;
            delete(node);
        }
    }
}

/**
 * 根据当前项目生成对应的FIRST集合
 * @param item 当前项目
 * @param nonTerminal 语法分析器的非终结符号集
 * @param grammar 语法分析器的语法
 * @return 生成的FIRST集合
 */
set<string> DFANode::getFirstSet(Item item, const vector<string> &nonTerminal, const vector<Expression> &grammar) {
    if (item.point >= item.expression.end.size() - 1) {
        // 圆点在推导式最后一个符号前,直接返回已有的向前看集
        return item.lookahead;
    }
    /* 否则,需要拼接字符串求FIRST集
       题目没有空集推导,故直接分辨终结符和非终结符即可
       注意:这里的判断起始是圆点后的第二个符号!
    */
    string target = item.expression.end[item.point + 1];
    if (std::find(nonTerminal.begin(), nonTerminal.end(), target) != nonTerminal.end()) {
        // 圆点后的第二个符号是非终结符
        // set<string> result = item.lookahead;
        set<string> result;
        getDeepFirstSet(result, target, nonTerminal, grammar);
        return result;
    } else {
        // 圆点后的第二个符号是终结符
        // item.lookahead.insert(target);
        return {target};
    }
}

void DFANode::getDeepFirstSet(set<string> &result, const string &str, const vector<string> &nonTerminal,
                              const vector<Expression> &grammar) {
    if (std::find(nonTerminal.begin(), nonTerminal.end(), str) != nonTerminal.end()) {
        // str是非终结符,递归
        for (const auto &expression: grammar) {
            if (expression.start == str) {
                getDeepFirstSet(result, expression.end[0], nonTerminal, grammar);
            }
        }
    } else {
        // str是终结符
        result.insert(str);
    }
}

bool DFANode::mergeLookahead(const Item &newItem) {
    for (const auto &it: itemList) {
        if (newItem.expression == it.expression && newItem.point == it.point) {
            it.lookahead.insert(newItem.lookahead.begin(), newItem.lookahead.end());
            return true;
        }
    }
    return false;
}

struct Action {
    string action;
    int target;

    Action() {
        action = "Error";
        target = 666;
    }

    Action(string action, int target) : action(std::move(action)), target(target) {}
};

struct PredictIndex {
    int id;
    map<string, Action> move;

    explicit PredictIndex(int id1) {
        id = id1;
        vector<string> signalList = {"+", "-", "*", "/", "(", ")", "num", "$", "E", "T", "F"};
        for (const auto &signal: signalList) {
            move[signal] = Action("Error", 666);
        }
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
        generateDFA();
        generateTable();
        printTable();
        // analyse(str);
    }

    int nodeNum = 1;

private:
    vector<string> terminal;
    vector<string> nonTerminal;
    vector<Expression> grammar;
    vector<DFANode*> dfa;
    vector<PredictIndex> predictTable;

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
        auto * dfaNode = new DFANode(0);
        Expression expression = grammar[0];
        set<string> lookaheadList;
        lookaheadList.insert("$");
        Item item(expression, 0, lookaheadList);
        dfaNode->itemList.insert(item);
        dfa.reserve(50);
        dfa.push_back(dfaNode);
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

    void generateTable();

    void printTable();
};

void LR1Parser::generateDFA() {
    /*
     * 要生成DFA,需要固定执行以下步骤
     * 1.从E'->E开始,通过求闭包构建项目集规范族
     * 2.对当前项目集规范族的每一个移进/待约项目,对圆点后的符号,进行项目集推导
     *   如果推导的结果项目集已经存在,当前项目集的next集合和结果项目集的previous集要增加
     */
    bool isChanged = true;
    int dfaStartPos = 0;
    while (isChanged) {
        isChanged = false;
        auto dfaPrevious = dfa;
        auto dfaSize = (int) dfa.size();
        for (auto dfaNode: dfa) {
            dfaNode->getClosure(nonTerminal, grammar);
        }

        for (auto iterator = dfaStartPos; iterator < dfaSize; ++iterator) {
            dfa[iterator]->findNextNode(dfa, nodeNum,nonTerminal,grammar); // 求新的项目集
        }

        dfaStartPos = dfaSize;
        if (dfa != dfaPrevious) {
            isChanged = true;
        }
    }

}

void LR1Parser::generateTable() {
    for (const auto node: dfa) {
        PredictIndex predictIndex(node->id);
        for (const auto &shift: node->next) {
            predictIndex.move[shift.first].action = "Shift";
            predictIndex.move[shift.first].target = shift.second->id;
        }
        for (const auto &item: node->itemList) {
            if (item.point == item.expression.end.size()) {
                for (const auto &reduce: item.lookahead) {
                    if (item.expression.id == 0) {
                        predictIndex.move[reduce].action = "Accept";
                        predictIndex.move[reduce].target = item.expression.id;
                    } else {
                        predictIndex.move[reduce].action = "Reduce";
                        predictIndex.move[reduce].target = item.expression.id;
                    }
                }
            }
        }
        predictTable.push_back(predictIndex);
    }
}

void LR1Parser::printTable() {
    for (const auto &predictIndex: predictTable) {
        cout << predictIndex.id << "\t";
        for (const auto &index: predictIndex.move) {
            cout << index.second.action << index.second.target << "\t";
        }
        cout << endl;
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
