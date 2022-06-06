#include "commontypes.h";
#include <iostream>
#include <sstream>
#include <algorithm>
int getDrop(unsigned char dropMax, unsigned char dropMin);

vector<NODE> getDropSeq(NODE *matrix, unsigned char rotate, unsigned char dropMax, unsigned char dropMin, unsigned char octmin, unsigned char octmax)
{
    vector<NODE> S;

    vector<NODE> VALID;
    vector<NODE> TEMP;

    for (int i = 0; i != 13; i++)
    {
        if (!matrix[i].skip)
        {
            VALID.push_back(matrix[i]);
            TEMP.push_back(matrix[i]);
        }
    }
    int index = 0;
    int maxindex = index;
    unsigned char maxValue = 0;
    // std::cout << "Valid are: " << TEMP.size() << std::endl;
    if (!VALID.size())
        return S;
    int iIndex = 0; // iteration index;
    while (true)
    {

        NODE N = TEMP.at(index);
        if (N.weight <= 8)
        {

            break;
        }

        if (N.weight > maxValue)
        {
            maxValue = N.weight;
            maxindex = index;
        }

        index++;
        iIndex++;
        if (index == TEMP.size())
        { // restart
            S.push_back(VALID.at(maxindex));
            //    cout << S.back().octave << endl;
            S.back().octave = getOctave(octmin, octmax);
            TEMP.at(maxindex).weight -= getDrop(dropMax, dropMin);
            //    cout << "Restarting" << endl;
            maxValue = 0;
            index = 0;
        }
    }
    rotate = rotate > S.size() ? rotate % S.size() : rotate;
    std::rotate(S.begin(), S.begin() + rotate, S.end());

    return S;
}
int getDrop(unsigned char dropMax, unsigned char dropMin)
{

    if (dropMin > dropMax)
    {
        int tmp = dropMax;
        dropMax = dropMin;
        dropMin = tmp;
    }
    if (dropMin == 0 || dropMin == dropMax)
        return dropMax;
    if (dropMax - dropMin == 1)
    {
        return dropMin + ((rand() % 2) == 0 ? 0 : 1);
    }
    return dropMin + (rand() % (dropMax + 1 - dropMin));
}
unsigned char getOctave(unsigned char min, unsigned char max)
{

    if (min > max)
    {
        unsigned char tmp = max;
        max = min;
        min = tmp;
    }
    if (max == min)
        return max;
    if (max - min == 1)
    {
        return min + ((rand() % 2) == 0 ? 0 : 1);
    }
    return min + (rand() % (max + 1 - min));
}

std::string FW(std::string label, std::string value, int max_digits)
{

    string s = label.append(value);

    int spaces = max_digits - value.length();
    if (spaces > 0)
    {
        string ss = string(spaces, ' ');

        s.append(ss);
    }
    return s;
}