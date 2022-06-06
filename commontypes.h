#ifndef __EUCLIDIER_COMMON__
#define __EUCLIDIER_COMMON__
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>

using namespace std;
struct lanePatch
{
    bool enabled;
    unsigned char steps;
    unsigned char pulses;
    unsigned char shift;
    unsigned char loop;
    unsigned char gate;
    unsigned char div;
    unsigned char note;
    unsigned char BV;
    unsigned char VA;
    unsigned char type;
    unsigned char ch = 0;
};
struct NODE
{
    bool skip = false;
    unsigned char ROOT = 13;
    int octave = 5;
    int weight = 100;
    int velocity = 96;
};
vector<NODE> getDropSeq(NODE *matrix, unsigned char rotate, unsigned char dropMax, unsigned char dropMin, unsigned char octmin, unsigned char octmax);
unsigned char getOctave(unsigned char min, unsigned char max);
std::string FW(std::string label, int value, int max_digits);
std::string FW(std::string label, std::string value, int max_digits);
#endif