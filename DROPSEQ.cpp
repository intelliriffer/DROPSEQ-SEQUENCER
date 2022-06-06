/*******************************************************************
 The Euclidean Sequencer Class for single channel Sequencer.
 Author: Amit Talwar <www.amitszone.com // intelliriffer@gmail.com>
 ***************************************************************** */
#include "DROPSEQ.h"
#include <algorithm>

DROPSEQ::DROPSEQ()
{
    // constructoir
    this->init();
    DROPSEQ::updateSeq();
    this->_ready = true;
}

void DROPSEQ::updateSeq()
{
    try
    {
        this->SEQ = getDropSeq(matrix, this->rotate, this->drop, this->drop_range, this->octmin, this->octmax);
        for (int i = 0; i != this->SEQ.size(); i++)
        {
            this->SEQ.at(i).velocity = getVel();
        }
    }
    catch (...)
    {
        cout << "exception at update" << endl;
    }
    if (this->instant_update)
    {

        this->loopCount = 0;
        this->RSEQ = this->SEQ;
    }

    this->dirty = true;
    this->_step = 0;
}

void DROPSEQ::setBPM(float newbpm)
{
    this->_bpm = newbpm;
}

void DROPSEQ::tick(long long _tick, long long ts) // tick is a midi clock pulse (24ppq)
{

    int div = this->getDiv();
    int step = _tick == 0 ? 0 : (_tick + 1) % ((24 * 4 / div));

    if ((clocked && step == 0) || !clocked) // valid timed step
    {
        if (_step == 0 && this->dirty)
        {
            // cout << "dirty" << endl;
            this->updateSeq();

            this->RSEQ = this->SEQ;
            this->loopCount = 0;
            if (this->autoregen)
            {
                this->needsRefresh = true;
            }

            if (!this->SEQ.size())
                return;

            this->dirty = false;
        }

        if (this->RSEQ.at(_step).ROOT < 12) // trig note (active)
        {

            long long interval = this->interval();

            if (this->enabled)
            {

                int note = this->RSEQ.at(_step).ROOT + (12 * this->RSEQ.at(_step).octave) + this->xpose;
                ;
                //        +this->xpose;

                int mul = note > 127 ? -1 : 1;
                while (note < 0 || note > 127)
                {
                    note = note + (12 * mul);
                }
                //    cout << note << " " << this->ch << endl;

                this->sendNote(0x90, this->ch - 1, note, this->RSEQ.at(_step).velocity); // send note on
                this->lastNote = note;

                try
                { // compute the time at which the note off will be sent,
                    float _gate = (float)this->gate / 100;
                    this->__OFF = ts + (long long)(interval * _gate) - OFF_US; // remove OFF_US microseconds (master pulse)
                }
                catch (std::exception &e)
                {
                    cout << " *** Caught Note OFF Setting Exception";
                    this->__OFF = 0;
                    this->sendNote(0x80, this->ch - 1, this->note, 0); // kill the note
                }
            }
        }

        __lastpulse = ts;

        _step++;

        if (_step == this->RSEQ.size())
        {
            this->loopCount++;
            if (this->autoregen != 0 && ((this->loopCount % this->autoregen) == 0))
            {
                this->dirty = true;
            }
            _step = 0;
        }
    }
}
void DROPSEQ::clock(long long ts) // triggered every 100 microseconds (1/10 ms)
{
    if (!this->_ready)
        return;
    if (ts - this->__OFF > 0 && this->__OFF > 0) // send note off or cc reset
    {

        this->sendNote(0x80, this->ch - 1, this->lastNote, 0);

        this->__OFF = 0; // reset note off time
    }
}

long long DROPSEQ::interval() // compute the single step duration based on bpm and time division for track
{
    double N = (60000 * 1000 / this->_bpm) * 4;

    int div = this->getDiv();
    return (long long)N / div;
}

int DROPSEQ::getDiv() // return midi time division 1/16,1/4 etc based on midi received valiees 1-6
{
    int div = 16;
    switch (this->div)
    {
    case 0:
    case 1:
        div = 16;
        break;
    case 2:
        div = 8;
        break;

    case 3:
        div = 4;
        break;

    case 4:
        div = 2;
        break;

    case 5:
        div = 1;
        break;

    case 6:
        div = 32;
        break;
    case 7:
        div = 24; // dotted 32
        break;
    case 8: // dotted 16
        div = 12;
        break;
    case 9: // dotted 8
        div = 6;
        break;
    case 10: // // dotted 4
        div = 3;
        break;
    }
    return div;
}

void DROPSEQ::setPORT(RtMidiOut *port) // assign selected midi output port
{
    this->midi = port;
}

void DROPSEQ::sendNote(unsigned char type, unsigned char ch, unsigned char note, unsigned char vel) // send midi note message to assigned output port
{
    std::vector<unsigned char> messageOut;
    messageOut.push_back(ch + type);
    messageOut.push_back(note);
    messageOut.push_back(vel);
    this->midi->sendMessage(&messageOut);
}

void DROPSEQ::ENABLE(bool e)
{
    if (!this->enabled == e)
    {
        this->enabled = e;

        killHanging();
    }
}
void DROPSEQ::updateCH(int ch)
{
    killHanging();
    this->ch = ch;
}
void DROPSEQ::updateDiv(int div)
{
    this->killHanging();
    this->div = div;
}

void DROPSEQ::reset(bool force)
{
    if (force)
    {
        this->_step = 0;
        this->killHanging();
        return;
    }

    // this->sync();
}

void DROPSEQ::killHanging() // stops any playing note used before any operation that could cause stuck notes..
{
    if (this->__OFF > 0)
    {

        this->sendNote(0x80, this->ch - 1, this->lastNote, 0); // kill hanging notes

        this->__OFF = 0;
    }
}
int DROPSEQ::getVel() // computes step velocity based on base velocity and humanization factor.
{
    // note mode
    if (this->velh == 0)
        return this->vel;
    int min = this->velh;
    int max = this->vel;

    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }
    if (max == min)
        return max;
    if (max - min == 1)
    {
        return limit(min + ((rand() % 2) == 0 ? 0 : 1), 0, 127);
    }
    return min + (rand() % (max + 1 - min));

    //        int mul = ((rand() % 10)) % 2 == 0 ? 1 : -1;
}

int DROPSEQ::limit(int v, int min, int max) // Restricts an interger value between limits
{
    if (v < min)
        v = min;
    if (v > max)
        v = max;
    return v;
}
void DROPSEQ::setGATE(int value) // set note duration (10-95%)
{
    bool wasEnabled = this->enabled;
    if (wasEnabled)
        this->enabled = false;
    this->gate = value;
    if (wasEnabled)
        this->enabled = true;
    this->killHanging();
}

void DROPSEQ::init()
{
    for (int i = 0; i != 13; i++)
    {
        matrix[i].ROOT = i;
        matrix[i].skip = (i != 0);
        matrix[i].weight = i == 0 ? 100 : 0;
    }
}
void DROPSEQ::setWeight(int N, int weight)
{
    N = this->limit(N, 0, 12);
    weight = limit(weight, 0, 100);
    weight = weight <= WEIGHT_MIN ? 0 : weight;
    this->matrix[N].weight = weight;
    this->matrix[N].skip = weight <= WEIGHT_MIN;
    this->updateSeq();
    this->dirty = true;
    // cout << "weight " << weight << " was set for nt" << N << endl;
}
void DROPSEQ::doRotate(int r)
{
    if (this->rotate == r)
        return;
    this->rotate = this->limit(r, 0, 127);
    this->updateSeq();
}

void DROPSEQ::setDrop(int drop)
{
    this->drop = this->limit(drop, 1, 50);
    this->updateSeq();
    // this->dirty = true;
}
void DROPSEQ::setDropRange(int range)
{
    this->drop_range = this->limit(range, 0, 50);
    this->updateSeq();
    // this->dirty = true;
}
void DROPSEQ::setOctRange(int range)
{
    this->octmin = this->limit(range, 0, 8);
    this->updateSeq();
    // this->dirty = true;
}
void DROPSEQ::setOctave(int o)
{
    this->octmax = this->limit(o, 0, 8);
    this->updateSeq();
    // this->dirty = true;
}

void DROPSEQ::print()
{
    try
    {
        cout << FW("", to_string(this->SEQ.size()), 4);
        for (int i = 0; i != this->SEQ.size(); i++)
        {
            string v = this->SEQ.at(i).ROOT < 12 ? std::to_string(this->SEQ.at(i).octave) : "";

            cout << FW(notenames[this->SEQ[i].ROOT], v, 2);
        }
    }
    catch (...)
    {
        cout << "exception at print" << endl;
    }
    cout << endl;
}
void DROPSEQ::regen()
{
    this->updateSeq();
    this->RSEQ = this->SEQ;
}

void DROPSEQ::setVel(int vel)
{
    this->vel = vel;
    this->updateSeq();
    // this->dirty = true;
}
void DROPSEQ::setVelh(int vel)
{
    this->velh = vel;
    this->updateSeq();
    // this->dirty = true;
}