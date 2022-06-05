#include "commontypes.h"
#include <iostream>
#include <sstream>
using namespace std;
void init();

NODE matrix[13];
const char *notenames[13] = {"C  ", "C# ", "D ", "Eb ", "E  ", "F  ", "F# ", "G ", "Ab ", "A ", "Bb ", "B  ", ".  "};
/*int main()
{
    srand(time(NULL));
    init();
    vector<NODE> SEQ = getDropSeq(matrix, 5, 15);
    cout << SEQ.size() << "  ";

    for (int i = 0; i != SEQ.size(); i++)
    {
        cout << notenames[SEQ.at(i).ROOT];
    }
    cout << endl;
    cout
        << "ready" << endl;
}
*/
void init()
{
    for (int i = 0; i != 13; i++)
    {
        matrix[i].ROOT = i;
        matrix[i].skip = (i != 0);
        matrix[i].weight = i == 0 ? 64 : 0;
    }
    matrix[1].skip = false;
    matrix[1].weight = 56;
}