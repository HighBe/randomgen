#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <string>
#include <cstring>
//#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include "frame.h"
#include <shlwapi.h>

using namespace std;

class sfunction
{
public:
    string funcname;//函数名
    int visbility;//函数可见性 0 public 1 private 2 external 3 internal 
    //string funcstate;//函数状态
    string returnPara;//函数返回值都有哪些,暂定最多一个返回值

    sfunction(const string& funcname, int visbility, const string& returnPara)
        : funcname(funcname), visbility(visbility), returnPara(returnPara)
    {
    }

    sfunction() = default;

    bool operator==(const sfunction& other) const
    {
        return funcname == other.funcname && visbility == other.visbility && returnPara == other.returnPara;
    }
};

class Variable
{
public:
    Variable() = default;
    string Typename;//存储类型名
    string Variablename;//变量名
    int num;//存储变量对应的值
    int a, b, c;//存放数组的三维，如果全为零，说明不是数组
    Variable(string tname, string vname, int t) {
        Typename = tname;
        Variablename = vname;
        num = t;
        a = b = c = 0;
    }
};



int inLoop = 0;//用来记录在第几循环中，最多三重
int blockcount = 0;//终止调用block
string infunc;//记录当前在生成哪个函数中的内容，避免调用当前函数

vector<Variable> VariableList;//存放状态变量
vector<Variable> LocalVarList;//存放局部变量
vector<Variable> GlobalVarList = { };//存放全局变量
vector<Variable> AssemblyVarList; // 存放内联函数中的变量

// 全局变量只有在运行时才能获得具体的值，无法判断是否溢出，暂移除
// Variable("uint256","block.basefee",0),
// Variable("uint256","block.chainid",0),
// Variable("uint256","block.gaslimit",0),Variable("uint256","block.number",0),
// Variable("uint256","block.timestamp",0),
// Variable("uint256","tx.gasprice",0)
//Variable("address","msg.sender",3),Variable("address","block.coinbase",3),  类型与现在要求不符
//, Variable("address", "tx.origin", 3)
//Variable("uint256","msg.value",0),只能用在 payable public 的函数中
//,Variable("uint256","block.prevrandao",0)

vector<sfunction> FuncList;//存放函数
vector<sfunction> FuncReturnList;//存放函数名和返回值类型



string assign_make_random();
string block_make_random();
string getFrame();
string genfunc(string funcname);
string ifelse_make_random();
string loop_make_random();
string state_make_random();
string genVariable(vector<Variable>& VarList, int i, bool isLocal);
string genArray(vector<Variable>& VarList, int i);
string getName(int i);
string getInt256();
string getAddress();
string generateScript(int i);
string inlineassembly();
string assemblyfor(int i,int tmp);
string assemblyif(int i,int tmp);

string testtestevent;   //只记录状态变量
string testtestemit;  

int main() {
    srand(time(0));
    ofstream fout;

    vector<long long > a;
    for (int i = 0; i < 10; i++) {
        a.emplace_back(i);
    }
    cout << a.size() << endl;

    for (int i = 0; i < 50; i++) {
        string res = getFrame();
        //存放生成的测试用例   在linux下生成存放的目录
        //string path = "/home/liu/CODE/ConsoleApplication1/testsuite/test" + to_string(i);
        //int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        //fout.open("/home/liu/CODE/ConsoleApplication1/testsuite/test" + to_string(i) + "/ContractName.sol");
        
        string contractpath = "E:\\CODE\\ConsoleApplication1\\ConsoleApplication1\\testsuite\\test" + to_string(i);
        CreateDirectory(contractpath.c_str(), NULL);
        if (!fout.is_open()) {
            cout << "could not open" << endl;
        }
        fout.open("E:\\CODE\\ConsoleApplication1\\ConsoleApplication1\\testsuite\\test" + to_string(i) + "/ContractName.sol");
        fout << res;
        fout.close();
        //生成测试脚本
        for (int j = 0; j < 3; j++) {
            res = generateScript(j);
            //fout.open("/home/liu/CODE/ConsoleApplication1/testsuite/test" + to_string(i) + "/func_" + getName(j) + ".js");
            fout.open("E:\\CODE\\ConsoleApplication1\\ConsoleApplication1\\testsuite\\test" + to_string(i) + "\\func_" + getName(j) + ".js");
            if (fout.is_open()) cout << "could not open" << endl;
            fout << res;
            fout.close();
        }
    }


    return 0;
}
//大数运算，如果越界就重新生成当前语句，直到不再越界
//如果结果没有越界返回true，否则返回false
//n1和n2都是处理后的的数据，不带0x的十六进制数，
// bool bignum(string n1,string n2,char opt){
//     string res;//保留运算结果，不带0x的十六进制数
//     string num1 = n1,num2 = n2;
//     bool flag;
//     int f = 0;//记录进位
//     //大数加法
//     if(opt == '+'){
//         for(int i = sizeof(num1)-1;i >= 0;i --){
//             int temp1,temp2,temp3;
//             if(num1[i] >= 'a') temp1 = 10 + num1[i] - 'a';
//             else temp1 = num1[i] - '0';
//             if(num2[i] >= 'a') temp2 = 10 + num2[i] - 'a';
//             else temp2 = num2[i] - '0';
//             temp3 = temp1 + temp2 + flag;
//             if(temp3 >= 16){
//                 temp3 -= 16;
//                 f = 1;
//             }
//             if(temp3 >= 10) res += 'a' + (temp3 - 10);
//             else res += temp3 + '0';
//         }
//     }else if(opt == '-'){
//         //先找出较大的数，保证不会出现不够减的问题
//         for(int i = 0;i < sizeof(n1);++ i){
//             if(n1[i] == 0 && n2[i] == 0) continue;
//             else if(n1[i] == 0 && n2[i] != 0){
//                 num1 = n2;
//                 num2 = n1;
//             }else if(n1[i] != 0 && n2[i] == 0){
//                 num1 = n1;
//                 num2 = n2;
//             }else{
//                 if(n1[i] > n2[i]){
//                     num1 = n1;
//                     num2 = n2;
//                 }else if(n1[i] < n2[i]){
//                     num1 = n2;
//                     num2 = n1;
//                 }else continue;
//             }
//         }
//         for(int i = sizeof(num1)-1;i >= 0;i --){
//             int temp1,temp2,temp3;
//             if(num1[i] >= 'a') temp1 = 10 + num1[i] - 'a';
//             else temp1 = num1[i] - '0';
//             if(num2[i] >= 'a') temp2 = 10 + num2[i] - 'a';
//             else temp2 = num2[i] - '0';
//             //够减，直接出结果
//             if(temp1 >= temp2 + f) {
//                 temp3 = temp1 - temp2 - f;
//                 f = 0;//如果之前f有借位，重置
//             }else {
//                 //不够减，往前借位
//                 temp3 = temp1 + 16 - temp2 - f;
//                 f = 1;
//             }
//             if(temp3 >= 10) res += 'a' + (temp3 - 10);
//             else res += temp3 + '0';
//         }
//         //如果两个数的位置倒换了，要取负号
//         if(n1 == num2) res = '-' + res;
//     }else{
//     }
//     return flag;
// }

// bool dealintopt(string num1,string num2,char opt){
//     int f1,f2;
//     f1 = f2 = 0;
//     if(num1[0] == '-') f1 = 1;
//     if(num2[0] == '-') f2 = 1;
//     if(f1 == 0 && f2 == 0){
//         string n1 = num1.substr(2);
//         string n2 = num2.substr(2);
//         return bignum(n1,n2,opt);
//     }else if(f1 == 1 && f2 == 1){
//         string n1 = num1.substr(3);
//         string n2 = num2.substr(3);
//         return bignum(n1,n2,opt);
//     }else if(f1 == 0 && f2 == 1){
//         string n1 = num1.substr(2);
//         string n2 = num2.substr(3);
//         if(opt == '-') return bignum(n1,n2,'+');
//         if(opt == '+') return bignum(n1,n2,'-'); 
//     }else if(f1 == 1 && f2 == 0){
//         string n1 = num1.substr(3);
//         string n2 = num2.substr(2);
//         if(opt == '-') return bignum(n1,n2,'+');
//         if(opt == '+') return bignum(n2,n1,'-');
//     }
// }


string assign_make_random() {
    string res;
    long long numres = 0;//得到等号右边表达时的值，判断是否越界
    long long numtmp = 0;//记录运算的中间结果
    int a = 0;//记录运算符
    res += "\n";
    int ls = VariableList.size() + LocalVarList.size();//等号左边可以是状态变量和局部变量
    int rs = ls; // 全局变量和函数返回值会出问题
    // int rs = ls + GlobalVarList.size() + FuncReturnList.size();//右边可以是状态变量、局部变量、函数返回值、全局变量

    //确定左边的变量。
    int tmp = rand() % ls;
    Variable tmpV;
    if (tmp < VariableList.size()) {
        tmpV = VariableList[tmp];
        res += VariableList[tmp].Variablename;
        //testtest += "emit testtest(";
        //if (tmpV.Typename == "uint256") testtest += "int256(" + tmpV.Variablename + "));\n";
        //else testtest += tmpV.Variablename + ");\n";
    }
    else {
        tmpV = LocalVarList[tmp - VariableList.size()];
        res += LocalVarList[tmp - VariableList.size()].Variablename;
        //testtest += "emit testtest(";
        //if (tmpV.Typename == "uint256") testtest += "int256(" + tmpV.Variablename + "));\n";
        //else testtest += tmpV.Variablename + ");\n";
    }

    //如果左边变量是数组元素


    //确定等于号
    tmp = rand() % 1;// *= 容易越界  暂时只用a=b+c;形式
    switch (tmp)
    {
    case 0:
        res += " = ";
        break;
    case 1:
        res += " += ";
        break;
    case 2:
        res += " -= ";
        break;
    case 3:
        res += " /= ";
        break;
    case 4:
        res += " *= ";
        break;
    }

    //确定等号右边。确定有几个变量，分别是什么,运算符是什么，右值可以是字面量    需要判断是否越界
    //右边如果是多个变量的运算，在经过多次赋值后，有概率导致overflow，故只简单的将一个变量的值赋给另一个变量
    int count = rand() % 2;//最多2个最少1个
    bool flag = tmpV.Typename == "uint256";
    string tmpstring;//记录等号右边的表达式，如果最后右边表达式的结果越界，清空该字符串，重新生成表达式，直到成功，将字符串连接到res后
    Variable tmpR;

    tmp = rand() % rs;
    //避免调用当前函数,如果随机到的变量是当前函数，则继续随机
    while (tmp >= ls + GlobalVarList.size() &&
        FuncReturnList[tmp - ls - GlobalVarList.size()].funcname == infunc) {
        tmp = rand() % rs;
    }
    string varname;
    if (flag) tmpstring += "uint256(";//如果等号左边是uint256类型，则在每个变量前加一个显式类型转换
    else tmpstring += "int256(";      //如果等号右边是int256类型，则在每个变量前加一个显示类型转换
    if (tmp < VariableList.size()) { //状态变量
        tmpR = VariableList[tmp];
        tmpstring += tmpR.Variablename;
        numtmp = tmpR.num;
    }
    else if (tmp < ls) {
        tmpR = LocalVarList[tmp - VariableList.size()];//局部变量
        tmpstring += tmpR.Variablename;
        numtmp = tmpR.num;
    }
    else if (tmp < ls + GlobalVarList.size()) { //全局变量
        tmpR = GlobalVarList[tmp - ls];
        tmpstring += tmpR.Variablename;
    }
    else {//有返回值的函数
        if (FuncReturnList[tmp - ls - GlobalVarList.size()].visbility == 2) {//函数可见性为external,需要加this.才能在当前合约中使用
            tmpstring += "this.";
        }
        tmpstring += FuncReturnList[tmp - ls - GlobalVarList.size()].funcname + "()";
    }
    tmpstring += ")";
    tmpstring += ";\n";

    // for (int i = 0; i <= count; i++) {
    //     tmp = rand() % rs;
    //     //避免调用当前函数
    //     if(tmp >= ls + GlobalVarList.size() && 
    //     FuncReturnList[tmp - ls - GlobalVarList.size()].funcname == infunc){
    //         i --;
    //         continue;
    //     }
    //     string varname;
    //     if (flag) tmpstring += "uint256(";//如果等号左边是uint256类型，则在每个变量前加一个显式类型转换
    //     else tmpstring += "int256(";      //如果等号右边是int256类型，则在每个变量前加一个显示类型转换
    //     if (tmp < VariableList.size()) { //状态变量
    //         tmpR = VariableList[tmp];
    //         tmpstring += tmpR.Variablename;
    //         if(i == 0) numres = tmpR.num;
    //         numtmp = tmpR.num;
    //     }
    //     else if (tmp < ls) {
    //         tmpR = LocalVarList[tmp - VariableList.size()];//局部变量
    //         tmpstring  += tmpR.Variablename;
    //         if(i == 0) numres = tmpR.num;
    //         numtmp = tmpR.num;
    //     }
    //     else if (tmp < ls + GlobalVarList.size()) { //全局变量
    //         tmpR = GlobalVarList[tmp - ls];
    //         tmpstring += tmpR.Variablename;
    //     }
    //     else {//有返回值的函数
    //         if (FuncReturnList[tmp - ls - GlobalVarList.size()].visbility == 2) {//函数可见性为external,需要加this.才能在当前合约中使用
    //             tmpstring += "this.";
    //         }
    //         tmpstring += FuncReturnList[tmp - ls - GlobalVarList.size()].funcname + "()";
    //     }
    //     tmpstring += ")";

    //     //插入运算符
    //     if (i != count) {
    //         tmp = rand() % 3;// 乘法容易越界
    //         if(flag) tmp = 0;
    //         switch (tmp)
    //         {
    //         case 0:
    //             tmpstring += " + ";
    //             a = 0;
    //             break;
    //         case 1:
    //             tmpstring += " - ";
    //             a = 1;
    //             break;
    //         case 2:
    //             tmpstring += " / ";
    //             a = 2;
    //             break;
    //         case 3:
    //             tmpstring += " * ";
    //             a = 3;
    //             break;

    //         }
    //     }
    //     if(i != 0){
    //         switch(a){
    //         case 0:
    //             numres += numtmp;
    //             break;
    //         case 1:
    //             numres -= numtmp;
    //             break;
    //         case 2:
    //             //numres /= numtmp;
    //             break;
    //         case 3:
    //             numres *= numtmp;
    //             break;
    //         }
    //     }
    //     if (i == count) {
    //         //uint类型的右值小于0，加一个常数，使右值大于0
    //         // if(flag && (numres < 0)){
    //         //     numres = 0 - numres + rand() % 20;
    //         //     tmpstring += " + " + to_string(numres);
    //         // }
    //         tmpstring += ";\n";
    //     }
    // }
    res += tmpstring;
    return res;
}

string block_make_random() {
    //创建新块，深度加一
    blockcount++;
    string res;
    //超过设置的块的数量上限,终止这个块的创建
    if (blockcount >= 5) {
        int l = rand() % 10;
        for (int i = 0; i < l; i++)
            res += assign_make_random();
        return res;
    }
    int tmp = rand() % 5;
    for (int i = 0; i <= tmp; i++) {
        int t = rand() % 3;
        switch (t) {
        case 0://assign
            res += assign_make_random();
            break;
        case 1://for
            if (inLoop <= 3) {
                res += loop_make_random();
            }
            break;
        case 2://if
            res += ifelse_make_random();
            break;
        }
    }
    blockcount--;
    //出块，深度减一
    return res;
}

string getFrame() {

    //每个合约开通固定的部分
    string res;
    res += "//SPDX-License-Identifier: GPL-3.0\n"
        "pragma solidity ^0.8.0;\ncontract ContractName{\n\t";
        //"constructor(){}\n\t";
    //插入evnet用来比较数据，判断compiler是否有bug
    //res += "\tevent testtest(int256 a);\n";

    //生成随机数量个状态变量
    
    int count = rand() % 10 + 10;

    for (int i = 0; i <= count; i++) {
        res += genVariable(VariableList, i, false);
    }

    testtestevent.clear();
    testtestemit.clear();
    testtestevent += "event testtest(";
    testtestemit += "emit testtest(";
    //for (int i = 0; i < 10; i++) {
    //    testtestevent += VariableList[i].Typename + " " + VariableList[i].Variablename;
    //    testtestemit += VariableList[i].Variablename;
    //    if (i != 9) {
    //        testtestevent += ",";
    //        testtestemit += ",";
    //    }
    //}

    for (vector<Variable>::iterator iter = VariableList.begin(); iter != VariableList.end(); iter++) {
        testtestevent += iter->Typename + " " + iter->Variablename;
        testtestemit += iter->Variablename;
        if (iter->Variablename != (VariableList.end() - 1)->Variablename) {
            testtestevent += ",";
            testtestemit += ",";
        }
    }

    testtestevent += ");\n";
    testtestemit += ");\n";
    res += testtestevent;
    //res += to_string(VariableList.size()) + "\n";
    testtestevent.clear();
    //生成3个函数
    count = 3;  //rand() % 10 + 1;
    for (int i = 0; i < count; i++) {
        res += genfunc("func_" + getName(i));
    }

    res += "}";
    VariableList.clear();
    return res;
}

string genfunc(string funcName) {
    string res;
    string sfuncname = funcName;
    infunc = funcName;
    int svisibility;
    string sreturnPara;
    //修改状态：1修改状态变量2触发事件3创建其他合约4出发selfdestruct
    //5通过调用发送以太币6调用非view或pure函数7触发低级调用8使用包含某些操作码的内联汇编

    //读取状态：1从状态变量中读取2访问address(this).balance和address.balance
    //3访问block tx msg的任何成员(除了msg.sig msg.data)
    //4调用非pure函数5使用包含某些操作码的内联汇编
    //enum funcstate
    //{
    //    eView,//不修改状态 
    //    ePure//不读取和修改状态
    //};
    enum visibility
    {
        ePublic,
        ePrivate,
        eExternal,//只能从合约外部调用（如果要从智能合约内部调用它，必须使用this）
        eInternal
    };
    res = "function " + funcName + "() ";
    int tmp = 0;//rand() % 4;
    switch (tmp)
    {
    case 0:
        res += "public ";
        svisibility = 0;
        break;
    case 1:
        res += "private ";
        svisibility = 1;
        break;
    case 2:
        res += "external ";
        svisibility = 2;
        break;
    case 3:
        res += "internal ";
        svisibility = 3;
        break;


    }
    //添加函数状态关键词  暂时有点难实现，暂不添加
    // tmp = rand() % 3;
    // switch (tmp)
    // {
    // case 0:
    //     f.funcstate = 0;
    //     break;
    // case 1:
    //     res += "view ";
    //     f.funcstate = 1;
    //     break;
    // case 2:
    //     res += "pure ";
    //     f.funcstate = 2;
    //     break;
    // }

    //添加modifiy，最多3个，
    /*tmp = rand() % 3;
    for (int i = 0; i <= tmp; i++) {
        res += "mod" + to_string(i);
    }*/

    //确定returns的数量及类型,最多3个
    //暂时将returns中的个数改为最多一个，并且直接确定name，加入局部变量的list中，命名为b_a,之后如果改为多个，则都用b_作为前缀
    // tmp = rand() % 3;
    // for(int i = 0;i <= tmp;i ++){
    //     if(i == 0) res += "returns(";

    //     int t = rand() % 5;
    //     switch(t){
    //         case 0:
    //             res += "uint256";
    //             f.returnPara.emplace_back("uint256");
    //             break;
    //         case 1:
    //             res += "int256";
    //             f.returnPara.emplace_back("int256");
    //             break;
    //         case 2:
    //             res += "bool";
    //             f.returnPara.emplace_back("bool");
    //             break;
    //         case 3:
    //             res += "address";
    //             f.returnPara.emplace_back("address");
    //             break;
    //         case 4:
    //             res += "bytes32";
    //             f.returnPara.emplace_back("bytes32");
    //             break;
    //     }

    //     if(i != tmp) res +=",";
    //     if(i == tmp) res += ")";
    // }

    //返回的变量放到局部变量list中，同时将函数名和返回变量的变量类型组合成一个新的变量放到状态变量list中
    //returns(uint256 b_a)  其中b_a可以在函数中不参与任何过程。
    int hasreturn = rand() % 2;
    if (hasreturn == 1) {//有返回值
        res += "returns(";
        int t = rand() % 2;//将变量的类型暂时限定在uint和int两种
        switch (t) {
        case 0:
            res += "uint256";
            sreturnPara = "uint256";
            LocalVarList.emplace_back(Variable("uint256", "b_a", 0));
            FuncReturnList.emplace_back(sfunction(funcName, svisibility, "b_a"));
            break;
        case 1:
            res += "int256";
            sreturnPara = "int256";
            LocalVarList.emplace_back(Variable("int256", "b_a", 1));
            FuncReturnList.emplace_back(sfunction(funcName, svisibility, "b_a"));
            break;
        case 2:
            res += "bool";
            sreturnPara = "bool";
            LocalVarList.emplace_back(Variable("bool", "b_a", 2));
            FuncReturnList.emplace_back(sfunction(funcName, svisibility, "b_a"));
            break;
        case 3:
            res += "address";
            sreturnPara = "address";
            LocalVarList.emplace_back(Variable("address", "b_a", 3));
            FuncReturnList.emplace_back(sfunction(funcName, svisibility, "b_a"));
            break;
        case 4:
            res += "bytes32";
            sreturnPara = "bytes32";
            LocalVarList.emplace_back(Variable("bytes32", "b_a", 4));
            FuncReturnList.emplace_back(sfunction(funcName, svisibility, "b_a"));
            break;
        }
        res += " b_a)";

    }

    res += "\n{";

    //生成局部变量 上限4个
    tmp = rand() % 3;
    //先清空一下上个函数生成的局部变量
    LocalVarList.clear();
    for (int i = 0; i <= tmp; i++) {
        res += genVariable(LocalVarList, i, true);
    }

    //确定在函数体中语句的数量
    //用state来生成函数体
    res += state_make_random();
    res += testtestemit;
    if (hasreturn == 1) res += "\nreturn b_a;";
    
    res += "\n}\n";

    blockcount = 0;

    //加入函数列表
    FuncList.emplace_back(sfunction(sfuncname, svisibility, sreturnPara));
    return res;
}

string returnInt(string int1, string int2) {
    //输入两个int类型的值，得到int类型的值
    //可进行的运算有：  +  -  *  /  |  &  ^
    string  res;
    res += "(" + int1;
    switch (rand() % 7) {
    case 0:
        res += "+";
        break;
    case 1:
        res += "-";
        break;
    case 2:
        res += "*";
        break;
    case 3:
        res += "/";
        break;
    case 4:
        res += "|";
        break;
    case 5:
        res += "&";
        break;
    case 6:
        res += "^";
        break;
    }
    res += int2 + ")";
    return res;
}
string returnBool(string int1, string int2) {
    //输入两个字符串表示两个Int类型的数
    //两个int类型数要得到bool类型结果，可以由几种方法
    //  <  >  ==  !=  
    string  res;
    res += "(" + int1;
    switch (rand() % 4) {
    case 0:
        res += "<";
        break;
    case 1:
        res += ">";
        break;
    case 2:
        res += "==";
        break;
    case 3:
        res += "!=";
        break;
    }
    res += int2 + ")";
    return res;
}
string boolreturnBool(string bool1, string bool2) {
    //输入两个字符串表示bool类型的变量
    //将两个bool类型操作 || &&  == != 
    string  res;
    res += "(" + bool1;
    switch (rand() % 4) {
    case 0:
        res += "||";
        break;
    case 1:
        res += "&&";
        break;
    case 2:
        res += "==";
        break;
    case 3:
        res += "!=";
        break;
    }
    res += bool2 + ")";
    return res;
}


//将每两个整型变量进行得到bool类型的操作，再将得到的bool类型进行操作
string ifelse_make_random() {
    string res;
    res += "\nif(";

    //确定有几个变量或常量,保底一个
    int vl = VariableList.size() + LocalVarList.size(); +FuncReturnList.size() + GlobalVarList.size();
    // int vl = VariableList.size() + LocalVarList.size() + FuncReturnList.size() + GlobalVarList.size();

    int tmp = rand() % 1 + 1;
    // string val1,val2;

    tmp = rand() % 3 + 1;
    //得到几组bool类型变量
    string val1[4], val2[4], boolval[4];
    for (int i = 0; i < tmp; i++) {
        int t = rand() % vl;
        val1[i] = "int256(";
        if (t < VariableList.size()) { //状态变量
            val1[i] += VariableList[t].Variablename;
        }
        else if (t < VariableList.size() + LocalVarList.size()) { // 局部变量 
            val1[i] += LocalVarList[t - VariableList.size()].Variablename;
        }
        else if (t < VariableList.size() + LocalVarList.size() + FuncReturnList.size()) {  // 由返回值的函数
            if (FuncReturnList[t - VariableList.size() - LocalVarList.size()].visbility == 2) val1[i] += "this.";
            val1[i] += FuncReturnList[t - VariableList.size() - LocalVarList.size()].funcname + "()";
        }
        else {  //  全局变量
            val1[i] += GlobalVarList[t - VariableList.size() - LocalVarList.size() - FuncReturnList.size()].Variablename;
        }
        val1[i] += ")";

        t = rand() % vl;
        val2[i] = "int256(";
        if (t < VariableList.size()) { //状态变量
            val2[i] += VariableList[t].Variablename;
        }
        else if (t < VariableList.size() + LocalVarList.size()) { // 局部变量 
            val2[i] += LocalVarList[t - VariableList.size()].Variablename;
        }
        else if (t < VariableList.size() + LocalVarList.size() + FuncReturnList.size()) {  // 由返回值的函数
            if (FuncReturnList[t - VariableList.size() - LocalVarList.size()].visbility == 2) val2[i] += "this.";
            val2[i] += FuncReturnList[t - VariableList.size() - LocalVarList.size()].funcname + "()";
        }
        else {  //  全局变量
            val2[i] += GlobalVarList[t - VariableList.size() - LocalVarList.size() - FuncReturnList.size()].Variablename;
        }
        val2[i] += ")";
        boolval[i] = returnBool(val1[i], val2[i]);
    }
    for (int i = 1; i < tmp; ++i) {
        boolval[0] = boolreturnBool(boolval[0], boolval[i]);
    }
    res += boolval[0] + ")\n{";

    // for (int i = 0; i <= tmp; i++) {
    //     int t = rand() % vl;
    //     //避免调用当前函数，导致无限递归
    //     if(t >= VariableList.size() + LocalVarList.size() && 
    //         t < VariableList.size() + LocalVarList.size() + FuncReturnList.size() && 
    //     FuncReturnList[t - VariableList.size() - LocalVarList.size()].funcname == infunc){
    //         i --;
    //         continue;
    //     }
    //     if (i == 0) {
    //         val1 = "int256(";
    //         if (t < VariableList.size()) { //状态变量
    //             val1 += VariableList[t].Variablename;
    //         }
    //         else if (t < VariableList.size() + LocalVarList.size()) { // 局部变量 
    //             val1 += LocalVarList[t - VariableList.size()].Variablename;
    //         }
    //         else if (t < VariableList.size() + LocalVarList.size() + FuncReturnList.size()) {  // 由返回值的函数
    //             if (FuncReturnList[t - VariableList.size() - LocalVarList.size()].visbility == 2) val1 += "this.";
    //             val1 += FuncReturnList[t - VariableList.size() - LocalVarList.size()].funcname + "()";
    //         }
    //         else {  //  全局变量
    //             val1 += GlobalVarList[t - VariableList.size() - LocalVarList.size() - FuncReturnList.size()].Variablename;
    //         }
    //         val1 += ")";
    //     }
    //     else {
    //         val2 = "int256(";
    //         if (t < VariableList.size()) { //状态变量
    //             val2 += VariableList[t].Variablename;
    //         }
    //         else if (t < VariableList.size() + LocalVarList.size()) { // 局部变量 
    //             val2 += LocalVarList[t - VariableList.size()].Variablename;
    //         }
    //         else if (t < VariableList.size() + LocalVarList.size() + FuncReturnList.size()) {  // 由返回值的函数
    //             if (FuncReturnList[t - VariableList.size() - LocalVarList.size()].visbility == 2) val2 += "this.";
    //             val2 += FuncReturnList[t - VariableList.size() - LocalVarList.size()].funcname + "()";
    //         }
    //         else {  //  全局变量
    //             val2 += GlobalVarList[t - VariableList.size() - LocalVarList.size() - FuncReturnList.size()].Variablename;
    //         }
    //         val2 += ")";
    //         if (i != tmp) {
    //             val1 = returnInt(val1, val2);
    //         }
    //         else {
    //             val1 = returnBool(val1, val2);
    //         }
    //     }
    // }
    // res += val1 + "){";
    //for (int i = 0; i <= tmp; i++) {
    //    res += "uint256(";
    //    if (t < VariableList.size()) { //状态变量
    //        res += VariableList[t].Variablename;
    //    }
    //    else if (t < VariableList.size() + LocalVarList.size()) { // 局部变量 
    //        res += LocalVarList[t - VariableList.size()].Variablename;
    //    }
    //    else if (t < VariableList.size() + LocalVarList.size() + FuncReturnList.size()) {  // 由返回值的函数
    //        if (FuncReturnList[t - VariableList.size() - LocalVarList.size()].visbility == 2) res += "this.";
    //        res += FuncReturnList[t - VariableList.size() - LocalVarList.size()].funcname + "()";
    //    }
    //    else {  //  全局变量
    //        res += GlobalVarList[t - VariableList.size() - LocalVarList.size() - FuncReturnList.size()].Variablename;
    //    }
    //    res += ")";
    //    enum Operation {
    //        eAnd,
    //        eOr,
    //        eLess,
    //        eGreater,
    //        eEqual
    //    };
    //    if (i != tmp - 1) {
    //        t = rand() % 5;
    //        switch (t) {
    //        case eAnd:
    //            res += " & ";
    //            break;
    //        case eOr:
    //            res += " | ";
    //            break;
    //        case eLess:
    //            res += " < ";
    //            break;
    //        case eGreater:
    //            res += " > ";
    //            break;
    //        case eEqual:
    //            res += " == ";
    //            break;
    //        }
    //    }
    //    else {
    //        res += "){\n";
    //    }
    //}

    //插入其他语句
    res += block_make_random();

    if (inLoop != 0) {
        tmp = rand() % 3;
        switch (tmp) {
        case 0://break;
            res += "\nbreak;\n";
            break;
        case 1://continue;
            res += "\ncontinue;\n";
            break;
        case 2://
            break;
        }
    }

    res += "}\n";
    return res;
}



string loop_make_random() {
    string res;
    string varname;
    if (inLoop >= 3) return res;
    switch (inLoop)
    {
    case 0:
        varname = "for_i";
        break;
    case 1:
        varname = "for_j";
        break;
    case 2:
        varname = "for_k";
        break;
    }
    inLoop++;
    res += "for(uint256 " + varname + "=0;" + varname + "<" +
        to_string(rand() % 10 + 1) + ";" + varname + "++)\n{\n";

    //中间调用block生成一系列块中的内容
    res += block_make_random();


    res += " \n }\n";
    inLoop--;

    return res;
}

//一个函数中生成多少个块
string state_make_random() {
    string res;

    //生成随机数量的块
    int tmp = rand() % 3;
    for (int i = 0; i <= tmp; i++) {
        int t = rand() % 3;
        //在生成每个块前都将深度清空，保证每个块都可以最深为5
        blockcount = 0;
        res += block_make_random();
        /*switch (t)
        {
        case 0:
            res += assign_make_random();
            break;
        case 1:
            res += loop_make_random();
            break;
        case 2:
            res += ifelse_make_random();
            break;
        }*/
    }
    //在每一个函数的最后插入emit语句，用来比较数据
    //res += testtest;
    //将两个字符串清空，用于下一个函数
    //testtest.clear();
    return res;
}

//确定生成的变量的name
string getName(int n) {
    string res;
    if (n / 26 == 1) {
        res += "a";
    }
    res += ('a' + n % 26);
    return res;
}
//因为solidity的uint256,int256是2的255次方，c++中无法用数字表示，只能直接生成字符串。生成的数都是正数，负数额外添加负号。
//2的255次方可以用64位16进制数来表示。

string getUInt256() {

    // string res = "0x";
    //随机生成的数字
    // for(int i = 0;i < 5;i ++){
    //     res += '0';
    // }
    // for (int i = 5; i < 64; i++) {
    //     res += '0' + rand() % 10;
    // }
    string res = to_string(rand() % 255);
    return res;
}
string getInt256() {

    // string res = "0x";
    // //随机生成的数字
    // for(int i = 0;i < 5;i ++){
    //     res += '0';
    // }
    // for (int i = 5; i < 64; i++) {
    //     res += '0' + rand() % 10;
    // }
    string res = to_string(rand() % 255);
    return res;
}

string getAdress() {

    string res = "0x";
    //随机生成的数字，后n位是有效的，前64-n全是0。
    int n = rand() % 40;
    for (int i = 0; i < 40 - n; i++) {
        res += '0';
    }
    for (int i = n; i < 40; i++) {
        res += '0' + rand() % 10;
    }
    return res;
}

string genVariable(vector<Variable>& VarList, int i, bool isLocal) {
    string res;//记录生成的代码。
    string value;
    int typec = rand() % 2;
    string tmpname = getName(i);
    //如果是局部变量在名称前加一个l_的前缀
    if (isLocal) {
        tmpname = "l_" + tmpname;
    }
    switch (typec)
    {
    case 0:
        value = getUInt256();
        VarList.emplace_back(Variable("uint256", tmpname, stoi(value)));
        //生成文本
        res += "\n\tuint256 ";
        if (!isLocal) res += " public ";
        res += tmpname + " = " + value;

        break;
    case 1:
        value = getInt256();
        //如果是0，生成负数，否则为正数
        res += "\n\tint256 ";
        if (!isLocal) res += " public ";
        res += tmpname + " = ";
        if (rand() % 2 == 0) value = "-" + value;
        res += getInt256();
        VarList.emplace_back(Variable("int256", tmpname, stoi(value)));
        break;
    case 2:
        VarList.emplace_back(Variable("bool", tmpname, 2));
        if (rand() % 2 == 0) res += "\n\tbool " + tmpname + " = " + "false";
        else res += "\n\tbool " + tmpname + " = " + "true";
        break;
    case 3://address 使用40个16进制数表示
        VarList.emplace_back(Variable("address", tmpname, 3));
        res += "\n\taddress " + tmpname + " = " + getAdress();
        break;
    case 4:
        VarList.emplace_back(Variable("bytes32", getName(i), 4));
        res += "\n\tbytes32 " + tmpname + " = " + getInt256();
        break;
    }
    res += ";\n";
    return res;
}

string genArray(vector<Variable>& VarList, int i) {
    string res;
    Variable tmpV;

    //确定数组大小,最小1 最大3
    int tmp = rand() % 3 + 1;
    //生成每一维对应的大小
    int* a = new int[tmp];
    for (int i = 0; i < tmp; i++) {
        a[i] = rand() % 5 + 5;
    }
    string tmps;
    string arrayName = "array_" + getName(i);
    tmpV.a = a[0];
    tmps += "[" + to_string(a[0]) + "]";
    if (tmp > 1) {
        tmpV.b = a[1];
        tmps = "[" + to_string(a[1]) + "]" + tmps;
    }
    if (tmp > 2) {
        tmpV.c = a[2];
        tmps = "[" + to_string(a[2]) + "]" + tmps;
    }
    //确定数组类型
    if (rand() % 2 == 0) {//uint
        res = "uint256" + tmps + arrayName + " = [";
        for (int i = 0; i < tmpV.a; i++) {
            res += "[";
            for (int j = 0; j < tmpV.b; j++) {
                res += "[";
                for (int k = 0; k < tmpV.c; k++) {//最里一层只生成数字
                    res += to_string(rand() % 50);
                    if (k != tmpV.c - 1) res += ",";
                }
                res += "]";
                if (i != tmpV.b - 1) res += ",";
            }
            res += "]";
            if (i != tmpV.a - 1) res += ",";
        }
        res += "];";

    }
    else {
        res = "int256" + tmps + arrayName + " = ";
        for (int i = 0; i < tmpV.a; i++) {
            res += "[";
            for (int j = 0; j < tmpV.b; j++) {
                res += "[";
                for (int k = 0; k < tmpV.c; k++) {//最里一层只生成数字
                    if (rand() % 2 == 0) res += "-";//int256生成负数
                    res += to_string(rand() % 50);
                    if (k != tmpV.c - 1) res += ",";
                }
                res += "]";
                if (i != tmpV.b - 1) res += ",";
            }
            res += "]";
            if (i != tmpV.a - 1) res += ",";
        }
        res += "];";
    }
    VarList.emplace_back(tmpV);
    return res;
}

string inlineassembly(){

    /*
    let x := y
    let res := add(x,div(x,y))      
    
    */
    string res = "assembly{\n";
    //随机定义数个变量
    int tmp = rand() % 5;
    for(int i = 0;i < tmp;++ i){
        string varname = "aa" + i + 'a';
        int varval = rand() % 10;
        res += "let varname := " + to_string(varval) + "\n";
        AssemblyVarList.emplace_back(Variable("assemblyvar",varname,varval));
    }
    switch(rand() % 2){
    case 0:
        assemblyfor(0,tmp);
        break;
    case 1:
        assemblyif(0,tmp);
        break;
    }
    res += "}";
    return res;
}
string assemblyfor(int i,int tmp){ // i控制深度,tmp表示有多少个变量
    if(i >3)return "";
    string var;
    var.push_back('i'+i);
    string res = "for {let "+var+" := 0} lt("+var+"," + to_string(rand() % 10) + ") {"+var+" := add(i,1)}\n{\n";
    // 添加其他代码


    switch(rand() % 2){
    case 0:
        res += assemblyfor(i+1,tmp);
        break;
    case 1:
        res += assemblyif(i+1,tmp);
        break;
    }
    res += "}";
    return res;
}
string assemblyif(int i,int tmp){
    string res;
    if(i > 3) return res;
    res += "if eq(" + AssemblyVarList[rand()%tmp].Variablename + "," + AssemblyVarList[rand() % tmp].Variablename + ")\n{\n";

    // 添加其他代码



    switch(rand()%2){
    case 0:
        res += assemblyif(i+1,tmp);
        break;
    case 1:
        res += assemblyfor(i+1,tmp);
        break;
    }
    res += "}";
    return res;
}
//一共三个函数每个函数都生成一个测试脚本
string generateScript(int i) {
    string res;
    res = "const {expect} = require(\"""chai\""");\n" \
        "const {loadFixture} = require(\"""@nomicfoundation/hardhat-network-helpers\""");\n" \
        "const web3 = require(\"""web3\""");\n\n" \
        "describe(\"ContractName\",function(){\n";
    res += "\tasync function deployOneYearLockFixture(){\n" \
        "\t\tconst _Contract = await ethers.getContractFactory(\"ContractName\");\n" \
        "\t\tconst [account0,account1,account2] = await ethers.getSigners();\n" \
        "\t\tconst _contract = await _Contract.deploy();\n" \
        "\t\treturn {_contract,account0,account1,account2};" \
        "\n\t}\n\n";
    res += "\tdescribe(\"func_" + getName(i) + "\",function(){\n" \
        "\t\tit(\"""testing func_" + getName(i) + "\",async function(){\n" \
        "\t\t\tconst {_contract,account0,account1,account2} = await loadFixture(deployOneYearLockFixture);\n" \
        "\t\t\tawait _contract.connect(account0).func_" + getName(i) + "();\n" \
        "\t\t});\n" \
        "\t});\n";
    //res += "\tconst filter = { \n\t\t address:_contract.address, \n\t\t topic:[ethers.utils.id('')]\n\t};\n" \
    //    "\tconst events = await _contract.provider.getLogs(filter);\n" \
    //    "\tconst parseEvents = events.map((event)=>_contract.interface.parseLog(event));\n" \
    //    "\tfor(var i = 0;i < parseEvents.length;i++){\n" \
    //    "\t\tif(parseEvents[i].name == \"testtestglobal\"){\n" \
    //    "\t\t\tfor(var j = 0;j < parseEvents[i].args.length;j ++){\n" \
    //    "\t\t\t\tconsole.log('',parseEvents[i].args[j]);\n" \
    //    "\t\t\t}\n\t\t}\n" \
    //    "\t\tif(parseEvents[i].name == \"testtestlocal\"){\n" \
    //    "\t\t\tfor(var j = 0;j < parseEvents[i].args.length;j ++){\n" \
    //    "\t\t\t\tconsole.log('',parseEvents[i].args[j]);\n" \
    //    "\t\t\t}\n\t\t}\n\t}\n";
    res += "});";
    return res;
}