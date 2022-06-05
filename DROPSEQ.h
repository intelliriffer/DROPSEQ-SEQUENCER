#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <sys/time.h>
#include <ctime>
#include "RtMidi.h"
#include "commontypes.h"

using namespace std;
class DROPSEQ
{
    const int OFF_US = 10;
    const int WEIGHT_MIN = 8;

private:
    bool dirty = true;
    vector<NODE> SEQ;
    vector<NODE> RSEQ;
    float _bpm = 120.00;
    bool _ready = false;
    int _gate = 50;
    long long interval();
    int _step = 0;
    long long __lastpulse = 0;
    long long __OFF = 0;
    int lastNote = 36;
    RtMidiOut *midi = 0;
    void sendNote(unsigned char type, unsigned char ch, unsigned char note, unsigned char vel);
    void killHanging();

    int getVel();
    int limit(int v, int min, int max);
    long long sstep = 0;
    int lastRandVal = 0;

public:
    int ch = 1;
    int div = 1;
    int gate = 65;
    int drop = 10;
    int drop_range = 0;
    bool instant_update = false;
    bool clocked = true;

    int note = 60;
    int xpose = 0;
    int octave = 5;
    unsigned char octmin = 4;
    unsigned char octmax = 6;
    int octrand = 0;

    int vel = 96;
    int velh = 0;
    int master = 3;

    bool clockSync = true;
    bool enabled = false;
    void update();

    void updateSeq();
    void print();
    void reset(bool force);
    void sync();
    NODE matrix[13];
    void init();
    DROPSEQ();
    void setBPM(float newbpm);
    void setMode(unsigned char _mode);
    void setGATE(int value);
    void clock(long long ts);
    void tick(long long _tick, long long ts);
    void setPORT(RtMidiOut *port);
    void ENABLE(bool e);
    void updateCH(int ch);
    void updateDiv(int div);
    void setWeight(int N, int wt);
    void setDrop(int drop);
    void setDropRange(int range);
    void setOctRange(int range);
    void setOctave(int o);

    int getDiv();

    const char *notenames[13] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B", "."};
};
