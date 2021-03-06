/*******************************************************************
 Drop Sequencer Main Program
 ***************************************************************** */
#include "DROPSEQ.h"
#include <unistd.h>
#include <cmath>
#include <sys/stat.h>
#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>
#include <sstream>
#include "RtMidi.h"
#include <math.h>
#include <algorithm>
#include "commontypes.h"
using namespace std;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
void onMIDI(double deltatime, std::vector<unsigned char> *message, void * /*userData*/);
void sendNote(unsigned char type, unsigned char ch, unsigned char note, unsigned char vel);
void sendClock(unsigned char msg);
void listInports();
uint getinPort(std::string str);
uint getOutPort(std::string str);
void doTempo();
void handleClockMessage(unsigned char message);
void pulse();
void updateBPM(float bpm);
void clockStart();
long long getUS();
void sendTicks();
void clear();
bool velSense = true;
bool bgprocess = false;
bool receiveNotes = true;
bool extClock = true;
int bpm1 = 120;
int bpm2 = 0;
long long lastPulse = 0;
float BPM = 120.00;
int intBPM = 120;
float syncDiv = 3;
bool doSync = true;

void clockStop();
string typeLabel(int trk);
unsigned long long now();
unsigned long long ms();
unsigned long long BOOT_TIME;
unsigned long long sstep;                // sequence step
const bool CONNECT_AKAI_NETWORK = false; // automatically connevt to akai network remote port

const unsigned VEL_SENSE_MIN = 22;
const unsigned VEL_SENSE_MAX = 127;
unsigned char currSlot = 0;
long long tick = 0;
int SLEEP_UNIT = 5000; // 5ms

vector<double> pulses;
int limit(int v, int min, int max);
bool paused = false;
bool started = false;
void printAll(bool _clear);
void printLane(int trk);
void resync(bool force, bool print);
void createBANK(string filename);
bool isClocked = true;
void setSleep();
string basePath = "";

// std::string FW(std::string label, int value, int max_digits);
float getDiv(unsigned char d);

RtMidiIn *midiIn = 0;
RtMidiIn *HWIN = 0;
RtMidiOut *midiOut = 0;

const unsigned char SEQS = 4;

DROPSEQ *SQ = new DROPSEQ[SEQS]; // creante the 8 track sequencer in an array
int main(int argc, char *argv[])
{
    bgprocess = argc > 1 && string(argv[1]) == "-v";

    srand(time(NULL));
    clear();
    char c[260];
    BOOT_TIME = now();
    midiIn = new RtMidiIn();
    midiIn->setCallback(&onMIDI);
    midiIn->ignoreTypes(false, false, false); // dont ignore clocK
    midiOut = new RtMidiOut();
    if (CONNECT_AKAI_NETWORK)
    {
        HWIN = new RtMidiIn();
        HWIN->setCallback(&onMIDI);
        HWIN->ignoreTypes(false, false, false); // dont ignore clocK

        int op = getOutPort("Akai Network - MIDI");
        if (op != 99)
        {

            midiOut->openPort(op);
            cout << "Opened Network output" << endl;
        }
        else
        {
            midiOut->openVirtualPort("DropSeq");
        }
        int dp = getinPort("Akai Network - MIDI");
        if (dp != 99)
        {
            delete midiIn;
            HWIN->openPort(dp);
            cout << "Opened Network input" << endl;
        }
    }
    else
    { // normal midi
        midiIn->openVirtualPort("DropSeq");
        midiOut->openVirtualPort("DropSeq");
    }
    if (!bgprocess)
        cout << "Ports opened - Waiting for Midi Clock Input to Start " << endl;

    for (int i = 0; i < SEQS; i++) // initialize sequencer parameters
    {
        SQ[i].setPORT(midiOut);
        // SQ[i].setRefreshHandler(&printAll);
        //__hook(&DROPSEQ::refresh, SQ[i], &printAll);
    }
    SQ[0].ENABLE(true);
    SQ[0].setOctave(6);
    SQ[0].setOctRange(4);
    printAll(false);

    while (true) // the main loop
    {
        long long us = getUS();

        if (started)
        {

            if (!extClock) // generate pulse
            {
                float pls = (60000 * 1000) / (BPM * 24);
                long long pulseUS = round(pls);
                long long diff = us - lastPulse;
                if ((diff >= pulseUS) || lastPulse == 0)
                {
                    long correction = 0;
                    if (lastPulse > 0)
                    {
                        correction = pls - diff;
                        // float _bpm = (24 * diff)
                    }
                    BPM = intBPM;
                    pulse();
                    lastPulse = us + correction;
                }
            }

            bool needsRefresh = false;

            for (char i = 0; i < SEQS; i++)
            {
                SQ[i].clock(us);
                if (SQ[i].needsRefresh)
                {
                    needsRefresh = true;
                    SQ[i].needsRefresh = false;
                }
            }
            if (needsRefresh)
            {
                printAll(true);
                needsRefresh = false;
            }
        }

        usleep(SLEEP_UNIT);
    }

    return 0;
}
void printAll(bool _clear = true) // prints the sequence to console.
{
    if (bgprocess)
        return;
    if (_clear)
        clear();

    cout << string(80, '*') << endl;

    cout << "        <<<  SEQUENCES  >>>  " << endl;
    cout << string(80, '*') << endl;

    for (int i = 0; i < SEQS; i++)
    {
        cout << i + 1 << ". " << (SQ[i].enabled ? "* " : "  ")
             << FW("1/", to_string(SQ[i].getDiv()), 2)
             << " | ";

        SQ[i].print();
    }

    /*   cout << endl
            << "  *********************************" << endl
            << endl;*/
    cout << string(80, '*') << endl;
}
void clear()
{
    if (bgprocess)
        return;
    // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
    cout
        << "\x1B[2J\x1B[H";
    cout << "  ************************************" << endl;
    cout << "  DropSeq  (Press Ctrl + C to Quit)" << endl;
    cout << "  ************************************" << endl;
}

void onMIDI(double deltatime, std::vector<unsigned char> *message, void * /*userData*/) // handles incomind midi
{

    unsigned char byte0 = (int)message->at(0);
    unsigned char typ = byte0 & 0xF0;
    uint size = message->size();

    if (size == 1) // system realtime message
    {

        handleClockMessage(message->at(0));
        return;
    }
    if (typ == 0x90) // note on
    {
        if (!isClocked && started)
        {
            for (unsigned char i = 0; i < SEQS; i++)
            {
                long long us = getUS();
                SQ[i].tick(tick, us);
            }
        }
        else if (isClocked)
        { // transpose
            unsigned char note = message->at(1);

            int oct = note < 60 ? -1 : 0;
            oct = note >= 72 ? 1 : oct;
            int xpose = (note % 12) + oct;
            for (unsigned char i = 0; i < SEQS; i++)
            {
                SQ[i].xpose = xpose;
            }
        }

        return;
    }

    if (typ == 0xB0) // cc message
    {

        /*

    1-13 weight
    Drop Min 14
    Drop Range 15
    Enable 16
    Ch: 17
    RandOct:18


    80+
    4 Gates,
    4 Divs


        */

        unsigned char CC = (int)message->at(1);
        unsigned char VAL = (int)message->at(2);
        unsigned char ch = byte0 & 0x0F;
        unsigned char trk = CC / 20;
        unsigned char cmd = CC % 20;
        if (CC == 39)
        { // BPM1
            bpm1 = limit(VAL, 30, 127);
            intBPM = bpm1 + bpm2;
        }
        if (CC == 40)
        { // BPM1
            bpm2 = limit(VAL, 0, 127);
            intBPM = bpm1 + bpm2;
            return;
        }

        if (CC == 100)
        {
            extClock = limit(VAL, 0, 1);

            return;
        }
        if (CC == 109)
        {
            for (int i = 0; i < SEQS; i++)
            {
                SQ[i].instant_update = limit(VAL, 0, 1);
            }
            return;
        }
        if (CC == 110) // note advance mode
        {
            isClocked = !limit(VAL, 0, 1);
            for (unsigned char i = 0; i < SEQS; i++)
            {
                SQ[i].clocked = isClocked;
                SQ[i].reset(true);
            }
            return;
        }

        if (CC == 119)
        {
            bool wasActive = velSense;
            velSense = limit(VAL, 0, 1);
            if (!velSense && wasActive) // reset the octaves
            {
                for (unsigned char i = 0; i < SEQS; i++)
                {

                    SQ[i].octave = 0;
                }
            }

            return;
        }

        if (CC == 99) // receive notes
        {

            receiveNotes = limit(VAL, 0, 1);

            return;
        }
        if (CC == 80) // start stop
        {
            if (extClock)
                return;
            if (VAL == 0 && started)
            { // steop

                clockStop();
            }
            if (VAL == 1 & !started)
            {
                clockStart();
            }

            return;
        }
        if (CC == 70) // resync
        {
            resync(true, true);
            return;
        }
        if (CC >= 81 && CC <= 84)
        {
            SQ[CC - 81].setGATE(limit(VAL, 10, 95));
            return;
        }
        if (CC >= 85 && CC <= 88)
        {
            SQ[CC - 85].updateCH(limit(VAL, 0, 15));
            return;
        }
        if (CC >= 91 && CC <= 94)
        {
            SQ[CC - 91].setDropRange(limit(VAL, 0, 50));
            printAll(true);
            return;
        }
        if (CC >= 95 && CC <= 98)
        {
            if (VAL % 2)
            {
                SQ[CC - 95].reset(true);
                printAll(true);
            }
            return;
        }
        if (CC >= 101 && CC <= 104)
        {
            SQ[CC - 101].setVel(VAL);
            printAll(true);
            return;
        }
        if (CC >= 105 && CC <= 108)
        {
            SQ[CC - 105].setVelh(VAL);
            printAll(true);
            return;
        }
        if (CC >= 111 && CC <= 114)
        {
            SQ[CC - 111].autoregen = limit(VAL, 0, 127);
            return;
        }
        if (CC >= 115 && CC <= 118)
        {
            if (VAL % 2 != 0)
            {
                SQ[CC - 115].regen();
                printAll(true);
            }
            return;
        }
        if (CC >= 59 && CC <= 60)
        {
            SQ[CC - 59].doRotate(VAL);
            printAll(true);
            return;
        }
        if (CC >= 89 && CC <= 90)
        {
            SQ[CC - 89].doRotate(VAL);
            printAll(true);
            return;
        }

        if (trk < SEQS) // if sequncer track messages tracks < 80
        {
            if (CC == 20)
            {
                SQ[0].setWeight(9, VAL);
                printAll(true);
                return;
            }

            switch (cmd)
            {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
                if ((trk == 0 && (cmd == 7 || cmd == 10)) || (trk == 3 && cmd == 4)) // filter reserved on force
                {
                    return;
                }
                SQ[trk].setWeight(cmd - 1, VAL);
                printAll(true);
                break;

            case 14: // enabled
                SQ[trk].ENABLE(VAL > 63);
                printAll(true);
                return;
                break;

            case 15: // Drop
                     //  SQ[trk].updateDiv(limit(VAL, 1, 10));

                SQ[trk].setDrop(VAL);
                //
                // SQ[trk].updateSeq();
                printAll(true);
                return;

                break;

            case 16:
                SQ[trk].setOctave(limit(VAL, 0, 8));
                //  SQ[trk].updateCH(limit(VAL, 1, 15));
                printAll(true);
                break;
            case 17:
                SQ[trk].setOctRange(VAL);

                printAll(true);
                return;
                if (trk == 0) // ignore cc 64
                    break;

                SQ[trk].setGATE(limit(VAL, 10, 95));
                break;
            case 18:
                SQ[trk].updateDiv(limit(VAL, 1, 10));

                //  SQ[trk].updateCH(limit(VAL, 1, 15));
                printAll(true);
                break;

            case 19: // edge cases for force
                if (trk == 0)
                {
                    SQ[trk].setWeight(6, VAL);
                    printAll(true);
                    break;
                }
                if (trk == 6) // track 6 cc64
                {
                    SQ[trk].setWeight(3, VAL);
                    printAll(true);
                    break;
                }
                break;
            }
        }

        else // trk > 7
        {
            /*  if (trk == 8 && cmd > 0 && cmd <= 8) // velocity messages
              {
                  SQ[cmd - 1].vel = VAL;
              }
              if (trk == 9 && cmd > 0 && cmd <= 8) // velocity humanization messages
              {
                  SQ[cmd - 1].velh = VAL;
              }
              if (trk == 10 && cmd > 0 && cmd <= 8) // loop steps 1-64
              {
                  //  cout << "loop" << endl;
                  SQ[cmd - 1].loop = limit(VAL, 0, 64); // 0=off
                  printAll();
              }
              if (trk == 11 && cmd > 0 && cmd <= 8) // set Modes 1-3
              {
                  SQ[cmd - 1].setMode(VAL); // 0=off
                  printAll();
              }*/
        }
    }
}

void sendNote(unsigned char type, unsigned char ch, unsigned char note, unsigned char vel)
{
    std::vector<unsigned char> messageOut;
    messageOut.push_back(ch + type);
    messageOut.push_back(note);
    messageOut.push_back(vel);
    // cout << "Sending " << (int)note << " with value " << (int)vel << endl;
    midiOut->sendMessage(&messageOut);
}
void sendClock(unsigned char msg)
{
    std::vector<unsigned char> messageOut;
    messageOut.push_back(msg);
    midiOut->sendMessage(&messageOut);
}

int limit(int v, int min, int max)
{
    if (v < min)
        v = min;
    if (v > max)
        v = max;
    return v;
}
void resync(bool force, bool print) // after parameter update , set all sequences to step 0 so they are in sync
{
    for (char i = 0; i < SEQS; i++)
    {

        SQ[i].reset(force);
    }
    if (print)
        printAll();
    paused = true;
}

void listInports()
{
    uint nPorts = HWIN->getPortCount();
    for (uint i = 0; i < nPorts; i++)
    {
        std::string portName = HWIN->getPortName(i);

        std::cout << "Port: " << i << " = " << portName << "\n";
    }
}
uint getinPort(std::string str)
{
    uint nPorts = HWIN->getPortCount();
    for (uint i = 0; i < nPorts; i++)
    {
        std::string portName = HWIN->getPortName(i);
        size_t found = portName.find(str);
        if (found != string::npos)
        {
            return i;
        }
    }
    return 99;
}
uint getOutPort(std::string str)
{
    uint nPorts = midiOut->getPortCount();
    for (uint i = 0; i < nPorts; i++)
    {
        std::string portName = midiOut->getPortName(i);
        size_t found = portName.find(str);
        if (found != string::npos)
        {
            return i;
        }
    }
    return 99;
}

void handleClockMessage(unsigned char message)
{

    // cout << "message: " << (int)message << endl;
    switch (message)
    {

    case 248:
        if (extClock)
            pulse();
        return;
    case 250:
        if (extClock)
            clockStart();
        return;
    case 252:
        clockStop();
        return;
    }
}

void pulse() // used to compute bpm and send clock message to sequencer for sync
{
    if (!extClock)
    {
        sendClock(248);
    }
    if (isClocked)
        sendTicks();
    tick += 1;
    if (extClock)
    {

        const uint mSize = 12; // fill up averaging buffer before computing final bpm;
        pulses.push_back(ms());
        /* if (pulses.size() < mSize)
             return;*/
        if (pulses.size() > mSize)
        {
            pulses.erase(pulses.begin());
        }

        double avg = 0;
        unsigned cnt = 0;
        for (uint i = 1; i < pulses.size(); i++)
        {
            double msd = pulses.at(i) - pulses.at(i - 1);
            double localavg = (60 / ((msd * 24) / 1000));
            avg += localavg;
            cnt++;
        }
        float bpm = (avg / cnt);
        BPM = bpm;
    }
    updateBPM(BPM);
}
void clockStart()
{
    if (!bgprocess)
        std::cout << "Clock Started"
                  << "\n"
                  << std::flush;
    if (!extClock)
        sendClock(250);
    tick = 0;
    sstep = 0;
    lastPulse = 0;
    resync(true, true);
    started = true;
    setSleep();
}
void clockStop()
{
    if (!extClock)
        sendClock(252);
    if (!bgprocess)
        std::cout << "Clock Stopped"
                  << "\n"
                  << std::flush;
    started = false;
    setSleep();
    resync(true, false);
}

unsigned long long now() // current time since epoch in ms
{
    return (unsigned long long)duration_cast<milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

unsigned long long ms() // ms elapsed since app started.
{
    return now() - BOOT_TIME;
}
void updateBPM(float bpm) // sends the computed bpm to all sequencers
{
    if (isinf(bpm))
        return;

    for (char i = 0; i < SEQS; i++)
    {
        SQ[i].setBPM(bpm);
    }
}
void sendTicks() // send midi pulse to all sequencers
{
    long long us = getUS();

    for (int i = 0; i < SEQS; i++)
    {

        SQ[i].tick(tick, us);
    }
}

long long getUS() // gets time since epch in microseconds
{
    auto t1 = std::chrono::system_clock::now();
    long long us = duration_cast<microseconds>(t1.time_since_epoch()).count();
    return us;
}

float getDiv(unsigned char d) // return midi time division 1/16,1/4 etc based on midi received valiees 1-6
{
    float div = 16.0;
    switch (d)
    {
    case 0:
    case 1:
        div = 16.0;
        break;
    case 2:
        div = 8.0;
        break;

    case 3:
        div = 4.0;
        break;

    case 4:
        div = 2.0;
        break;

    case 5:
        div = 1.0;
        break;

    case 6:
        div = 1.0 / 2;
        break;
    case 7:
        div = 1.0 / 4;
        break;
    case 8:
        div = 1.0 / 8;
        break;
    }
    return div;
}

void setSleep()
{
    if (started)
    {
        SLEEP_UNIT = 100;
        return;
    }
    SLEEP_UNIT = 5000;
}