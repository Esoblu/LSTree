#ifndef GRAMMAR_H
#define GRAMMAR_H
#include<iostream>
#include<unordered_map>
#include<vector>
using std::string;
using std::unordered_map;
using std::vector;

class Grammar {
public:
    Grammar() : level(1) {}
    Grammar(int iter, string start) : level(iter), start(start) {}
    string get_start() const { return start; }
    string get_result() const { return result; }
    auto get_rules() const { return rules; }
    int get_level() const { return level; }
    void clear() {
        level = 1;
        start = "";
        result = "";
        rules.clear();
    }
    void set_level(int i) { level = i; }
    void set_start(string s) { start = s; }
    void set_result(string s) { result = s; }
    void add_rule(char c, string s) { rules[c].push_back(s); }

    void generate(){
        result = start;
        for(int i = 0; i < level; i++) {
            string temp = "";
            for(auto c : result){
                if(rules.find(c) != rules.end()){
                    int index = rand() % rules[c].size();
                    temp += rules[c][index];
                }
                else {
                    temp += c;
                }
            }
            // printf("Iteration %d: %s\n", i+1, temp.c_str());
            result = temp;
        }
    }

public:
    int level;
    string start;
    string result;
    unordered_map<char, vector<string>> rules;
};

#endif