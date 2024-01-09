#include "Assembly.h"

extern vector<Variable> AssemblyVarList; // 存放内联函数中的变量

string inlineassembly() {

    /*
    let x := y
    let res := add(x,div(x,y))

    */
    string res = "assembly{\n";
    //随机定义数个变量
    int tmp = rand() % 5;
    for (int i = 0; i < tmp; ++i) {
        string varname = "aa" + i + 'a';
        int varval = rand() % 10;
        res += "let varname := " + to_string(varval) + "\n";
        AssemblyVarList.emplace_back(Variable("assemblyvar", varname, varval));
    }
    switch (rand() % 2) {
    case 0:
        assemblyfor(0, tmp);
        break;
    case 1:
        assemblyif(0, tmp);
        break;
    }
    switch(rand() % 2){
    case 0:
        res += "return (0,0)";
    case 1:
        res += "return (" + to_string(rand()%10) + "," + to_string(rand()%8 * 8) + ")";
    }
    res += "}";
    return res;
}
string AssemblyKeccak(){
    string res = "keccak256(";
    res += to_string(rand() % 10) + ",";
    res += to_string(rand() % 33) + ")";
    return res;
}
string mload(){
    string res = "mload("+ to_string(rand()%10) + ")";
    return res;
}
string AssemblyAdd(){
    string res = "";
    int al = AssemblyVarList.size();
    res += AssemblyVarList[rand()%al].Variablename + ":= add(";
    switch (rand()%2)
    {
    case 0:
        res += AssemblyVarList[rand()%al].Variablename;
        break;
    case 1:
        res += mload();
        break;
    }
    res += ",";
    switch (rand() % 2)
    {
    case 0:
        res += AssemblyVarList[rand()%al].Variablename;
        break;
    case 1:
        res += mload();
        break;
    }
    res += ")";
    return res;
}
string AssemblyMul(){
    string res = "";
    int al = AssemblyVarList.size();
    res += AssemblyVarList[rand()%al].Variablename + ":= mul(";
    switch (rand()%2)
    {
    case 0:
        res += AssemblyVarList[rand()%al].Variablename;
        break;
    case 1:
        res += mload();
        break;
    }
    res += ",";
    switch (rand() % 2)
    {
    case 0:
        res += AssemblyVarList[rand()%al].Variablename;
        break;
    case 1:
        res += mload();
        break;
    }
    res += ")";
    return res;
}
string AssemblyMstore(){
    string res = "";
    int al = AssemblyVarList.size();
    res += "mstore(" + to_string(rand()%10) + "," ;
    switch (rand() % 4)
    {
    case 0:
        res += to_string(rand() % 50);
        break;
    case 1:
        res += AssemblyVarList[rand()%al].Variablename;
        break;
    case 2:
        res += AssemblyAdd();
        break;
    case 3:
        res += AssemblyMul();
        break;
    }
    res += ")";
    return res;
}
string AssemblyAssign(int l){
    string res = "";
    int al = AssemblyVarList.size();
    for(int m = 0;m < l;++ m){
        switch(rand() % 4){
        case 0:
            res += AssemblyAdd();
            break;
        case 1:
            res += AssemblyMul();
            break;
        case 2:
            res += AssemblyMstore();
            break;
        case 3:
            res += AssemblyVarList[rand()%al].Variablename + ":=" + mload();
            break;
        case 4:   // return (0x0,32)返回从0地址开始往后的32字节内容
            res += "return(" + to_string(rand() % 10) + "," + to_string(rand() % 33) + ")";
        }
    }
    return res;
}
string assemblyfor(int i, int tmp) { // i控制深度,tmp表示有多少个变量
    if (i > 3)return "";
    string var;
    var.push_back('i' + i);
    string res = "for {let " + var + " := 0} lt(" + var + "," + to_string(rand() % 10) + ") {" + var + " := add(" + var +",1)}\n{\n";
    int al = AssemblyVarList.size();
    // 添加其他代码
    int l = rand() % 5;
    res += AssemblyAssign(l);
    switch (rand() % 2) {
    case 0:
        res += assemblyfor(i + 1, tmp);
        break;
    case 1:
        res += assemblyif(i + 1, tmp);
        break;
    }
    l = rand() % 5;
    res += AssemblyAssign(l);
    res += "}";
    return res;
}
string assemblyif(int i, int tmp) {
    string res;
    if (i > 3) return res;
    res += "if eq(" + AssemblyVarList[rand() % tmp].Variablename + "," + AssemblyVarList[rand() % tmp].Variablename + ")\n{\n";
    int al = AssemblyVarList.size();
    // 添加其他代码
    int l = rand() % 5;
    res += AssemblyAssign(l);
    switch (rand() % 2) {
    case 0:
        res += assemblyif(i + 1, tmp);
        break;
    case 1:
        res += assemblyfor(i + 1, tmp);
        break;
    }
    l = rand() % 5;
    res += AssemblyAssign(l);
    res += "}";
    return res;
}