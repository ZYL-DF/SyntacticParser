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
class LR1Parser {
public:
    map<int, map<string, string>> unsetGrammar;

    explicit LR1Parser(map<int, map<string, string>> grammar) {
        unsetGrammar = std::move(grammar);
    };


    void parse(const string &str) {
        // 语法解析器主函数
        init();
        analyse(str);
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

    struct PredictIndex {
        PredictIndex(string signal, const map<string, Expression> &predictor) : signal(std::move(signal)),
                                                                                predictor(predictor) {}

        string signal;
        map<string, Expression> predictor;
    };

    vector<string> terminal;
    vector<string> nonTerminal;
    vector<Expression> grammar;
    map<string, set<ExpressionPositioner, expressionPositionerFunc>> firstSet;
    map<string, set<ExpressionPositioner, expressionPositionerFunc>> followSet;
    vector<PredictIndex> predictTable;

    /**
     * 初始化FirstSet和FollowSet的pair
     * @param str
     * @return pair
     */
    auto generatePair(const string &str) {
        return pair<string ,set<ExpressionPositioner,expressionPositionerFunc>> (str, set<ExpressionPositioner, expressionPositionerFunc>());
    }

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
        nonTerminal = {"E", "T", "A", "B", "F"};

        vector<string> signalSet(terminal.size()+nonTerminal.size());// 全部符号集
        merge(terminal.begin(), terminal.end(), nonTerminal.begin(), nonTerminal.end(), signalSet.begin());
        signalSet.emplace_back("ε");
        for (auto sentence: unsetGrammar) {
            Expression expression(sentence.first, sentence.second.begin()->first,
                                  generateSignalSet(sentence.second.begin()->second, signalSet));
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

        getFirstSet();
        getFollowSet();
        generateTable();
    }

    /**
     * 获得文法的First集合
     */
    void getFirstSet() {

        for (const auto &item: grammar) {
            getDeepFirstSet(item, item, 0);
        }

        /*for (const auto &item: firstSet) {
            cout << item.first << ":" << endl;
            for (const auto &index: item.second) {
                cout << "(" << index.id << "," << index.signal << ")" << endl;
            }
            cout << endl;
        }*/
    }

    /**
     * 获得文法的First集合的递归函数
     */
    void getDeepFirstSet(const Expression &item, const Expression &father,
                         int itemPos) {

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
                    bool isEpsilon = false;
                    for (const auto &expr_first: firstSet[expr.start]) {
                        // 将子非终结符已经构建的First集合,加入到父非终结符的First集合
                        if (expr_first.signal != "ε") {
                            // 如果有子推导式没有空串的推导,直接将子推导式的First集合加入到父推导式的First集合
                            addFirstSet(item.id, item.start, expr_first.signal);
                        } else {
                            isEpsilon = true;
                        }
                    }
                    if (isEpsilon) {
                        // 如果子推导式出现空串推导
                        if (itemPos == item.end.size() - 1) {
                            // 已经是最后一个符号仍为空串,说明父推导式First需要加入空串
                            addFirstSet(item.id, item.start, "ε");
                        } else {
                            // 否则,检查推导式的下一个符号
                            getDeepFirstSet(expr, item, itemPos + 1);
                        }
                    }
                }
            }
        }
    }


    string findTerminal(const vector<string> &end) {
        for (const auto &signal: terminal) {
            if (signal == end[0]) {
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
        bool isChanged = true;
        while (isChanged) {
            isChanged = false;
            auto followSetPre = followSet;
            for (const auto &item: grammar) {
                for (int pos = 0; pos < item.end.size(); pos++) {
                    if (std::find(nonTerminal.begin(), nonTerminal.end(), item.end[pos]) != nonTerminal.end()) {
                        // 当前符号为非终结符
                        if (pos == item.end.size() - 1) {
                            // 为推导式最后一个符号,A->αB,让follow(A)->follow(B)
                            for (const auto &followSignal: followSet[item.start]) {
                                addFollowSet(1, item.end[pos], followSignal.signal);
                            }
                        } else {
                            if (isEmpty(item.end[pos + 1]) && isNotEmpty(item.end[pos + 1])) {
                                // A->αBε,让follow(A)->follow(B)
                                for (const auto &followSignal: followSet[item.start]) {
                                    addFollowSet(1, item.end[pos], followSignal.signal);
                                }
                            }
                            // 不为推导式最后一个符号,A->αBb,follow(B) = first(b)
                            if (std::find(nonTerminal.begin(), nonTerminal.end(), item.end[pos + 1]) !=
                                nonTerminal.end()) {

                                // b为非终结符开头
                                for (const auto &firstSignal: firstSet[item.end[pos + 1]]) {
                                    if (firstSignal.signal != "ε") {
                                        addFollowSet(1, item.end[pos], firstSignal.signal);
                                    }
                                }

                            } else {
                                // b为终结符开头且不为ε,直接加入
                                if (item.end[pos + 1] != "ε") {
                                    addFollowSet(1, item.end[pos], item.end[pos + 1]);
                                }
                            }
                        }
                    }
                }
            }
            for (const auto &item: followSetPre) {
                if (followSet[item.first].size() != item.second.size()) {
                    isChanged = true;
                }
            }
        }

        /*for (const auto &item: followSet) {
            cout << item.first << ":" << endl;
            for (const auto &index: item.second) {
                cout << "(" << index.id << "," << index.signal << ")" << endl;
            }
            cout << endl;
        }*/
    }

    void addFirstSet(int id, const string &target, string result) {
        ExpressionPositioner e{.id = id, .signal = std::move(result)};
        firstSet[target].insert(e);
    }

    void addFollowSet(int id, const string &target, string result) {
        ExpressionPositioner e{.id = id, .signal = std::move(result)};
        followSet[target].insert(e);
    }

    bool isEmpty(const string &str) {
        for (auto express: grammar) {
            if (express.start == str && (std::find(express.end.begin(), express.end.end(), "ε") != express.end.end())) {
                return true;
            }
        }
        return false;
    }

    bool isNotEmpty(const string &str) {
        for (const auto &express: grammar) {
            if (express.start == str) {
                for (const auto &item: firstSet[express.start]) {
                    if (item.signal != "ε") {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    Expression getExpression(int id) {
        for (auto expression: grammar) {
            if (expression.id == id) {
                return expression;
            }
        }
        // 这里默认id是合法的
    }

    // 根据first集合和follow集合构造预测分析表
    void generateTable() {
        for (const auto &signal: nonTerminal) {

            map<string, Expression> predictor;
            Expression errorExpr(-1, "error", vector<string>(1, "error"));
            for (const auto &item: terminal) {
                predictor.insert(map<string, Expression>::value_type(item, errorExpr));
            }
            predictor.insert(map<string, Expression>::value_type("$", errorExpr));

            for (const auto &firstItem: firstSet[signal]) {
                if (firstItem.signal != "ε") {
                    Expression expression = getExpression(firstItem.id);
                    predictor.find(firstItem.signal)->second = expression;
                } else {
                    for (const auto &followItem: followSet[signal]) {
                        Expression expression = getExpression(firstItem.id);
                        predictor.find(followItem.signal)->second = expression;
                    }
                }
            }
            PredictIndex index(signal, predictor);
            predictTable.push_back(index);
        }

        // printTable();
    }

    PredictIndex getPrediction(const string &signal) {
        for (const auto &item: predictTable) {
            if (item.signal == signal) {
                return item;
            }
        }
    }

    void printTable() {
        cout << "\t";
        for (const auto &item: terminal) {
            cout << item << "\t\t";
        }
        cout << "$" << endl;
        for (const auto &item: predictTable) {
            cout << item.signal << "\t";
            for (const auto &signal: terminal) {
                cout << item.predictor.find(signal)->second.start << "->";
                for (const auto &character: item.predictor.find(signal)->second.end) {
                    cout << character;
                }
                cout << "\t";
            }
            cout << item.predictor.find("$")->second.start << "->";
            for (const auto &character: item.predictor.find("$")->second.end) {
                cout << character;
            }
            cout << endl;
        }
    }

    string getStackContent(stack<string> stackIndex, bool isReverse) {

        string result;
        while (!stackIndex.empty()) {
            if (stackIndex.top() == "num") {
                result += "n";
            } else {
                result += stackIndex.top();
            }
            stackIndex.pop();
        }
        if (isReverse) {
            // 反转默认是$E
            reverse(result.begin(), result.end());
        }
        return result;
    }

    /**
     * 分析函数
     */
    void analyse(const string &input) {
        string str;
        for (auto character: input) {
            if (character == 'n') {
                str += "num";
            } else {
                str += character;
            }
        }
        stack<string> analyticalStack;
        // 分析栈
        stack<string> inputStack;
        // 输入栈

        analyticalStack.emplace("$");
        analyticalStack.emplace("E");
        inputStack.emplace("$");
        vector<string> signalSet;// 全部符号集
        merge(terminal.begin(), terminal.end(),
              nonTerminal.begin(), nonTerminal.end(),
              std::back_inserter(signalSet));
        vector<string> inputSet = generateSignalSet(str, signalSet);
        for_each(inputSet.rbegin(), inputSet.rend(),
                 [&inputStack](const string &input) { inputStack.emplace(input); });

        // 符号栈和输入栈初始化完成

        /*$E	n+$	1
        $AT	n+$	5
        $ABF	n+$	10
        $ABn	n+$	match
        $AB	+$	8
        $A	+$	2
        $AT+	+$	match
        $AT	$	error*/
        while (analyticalStack.size() > 1 || inputStack.size() > 1) {
            if (analyticalStack.top() == inputStack.top()) {
                // 规约
                cout << getStackContent(analyticalStack, true) << "\t"
                     << getStackContent(inputStack, false)
                     << "\tmatch"
                     << endl;
                analyticalStack.pop();
                inputStack.pop();
            } else {
                // 移进
                PredictIndex predictIndex = getPrediction(analyticalStack.top());
                Expression expression = predictIndex.predictor.find(inputStack.top())->second;
                if (expression.id != -1) {
                    cout << getStackContent(analyticalStack, true) << "\t"
                         << getStackContent(inputStack, false)
                         << "\t"
                         << expression.id
                         << endl;
                    analyticalStack.pop();
                    if(expression.end[0] != "ε") {
                        for_each(expression.end.rbegin(), expression.end.rend(),
                                 [&analyticalStack](const string &item) {
                                     analyticalStack.emplace(item);
                                 });
                    }

                } else {
                    // 预测表中没有对应的预测推导,报错
                    cout << getStackContent(analyticalStack, true) << "\t"
                         << getStackContent(inputStack, false)
                         << "\terror"
                         << endl;
                    break;
                }
            }
        }
        if (analyticalStack.top() == "$" && inputStack.top() == "$") {
            cout << getStackContent(analyticalStack, true) << "\t"
                 << getStackContent(inputStack, false)
                 << "\taccept"
                 << endl;
        }
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
    LR1Parser lr1Parser(grammar);
    string str;
    cin >> str;
    lr1Parser.parse(str);
    getchar();
    getchar();
    return 0;
}
