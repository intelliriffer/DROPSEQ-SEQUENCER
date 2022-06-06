# DropSEQ - You May Never Know What You Gonna Get.. v 0.1.2
**A unique Weight & Drop Based  Dynamic Midi Note Sequencer** for Akai Force, Akai MPC, Raspberry Pi and Max Osx (intel)

## Download Latest Binaries 0.1.1
1. [Akai Force, Akai MPC & Raspberry Pi](https://mega.nz/file/4t5mCSgT#s8Y0G6zhUyMv4BVUNFYsuuiowGR4VdVMKIeQgRZCGlw)


## Features
1. 4 Independent Tracks (Channels)
2. Different Time Divisions / Channel
3. Different Note duration (Gate) / Channel…
4. Different Sequences / Channel
5. Auto Sequence Regeneration  / Channel
6. Internal + External Clock Support.
7. Full Midi Control **See Below for Midi Mapping.


## Concept
This Sequencer is Based on a **Subtractive Sequencer** concept by a Fellow Akai Force User: **Steven Law**

1. It consists of 12 Notes + Rest and you assign a weight to each note (value 0, note is not used, values 9-100, Note is used).
2. At Each Pulse Step (Clock Division or Manual Trig),The Weights are evaluated and the Note with Maximum Weight is Chosen, ( . Silence for rest).
3. If 2 or more notes share same weight, the First Note takes Priority.
4. The User Specified **DROP MAX** Value is then Subtracted from this Note.
5. This Process Repeats until Any of the Note Weights reaches value of 8.
6. All the Chosen Notes Thus Form a Sequence that will be Played.
7. Through Various Other Parameter Ranges, The Algorithm will use a Random Value between Min and Max Value provided.

**The Three Ranged parameters are :**
   1. **DropMax and DropMin**:  When Drop Min = 0, its Not Used. and only Drop Max is Used. Otherwise At Each Pulse, a Random Drop Value is Computed between Drop Max and Drop Min and Applied to that Particular note. This Can change Note order and Sequence dynamically.
   2. **OctaveMax & Octave Min**: values (0-8). The Note at the Step Will be assigned a Random Octave between Octave Max and Octave Min, Use OctaveMax=OctaveMin to generate Sequence Notes from a Fixed Octave.
   3. **VelocityMax and VelocityMin** : Each Seq Note will be assigned Random Velocity from within the Range. Keep both same for Fixed Velocity.

## Functions Explained

1. ### Note Advance (Global)
   The Sequencer Runs in 2 Modes:
   1. Clocked (0):
      1.  The Step in the Generated Sequence is Triggered (Note or Rest) for each in coming Clock Pulse (1/6, or 1/8 etc). 
      2.  Based on Incoming Note a Transpose 1-11 is Applied to The Sequence, Transpose Octave is Calculated base on Middle C (60) , values Below 60 will also transpose -1 octave, Values 72+ Transposes + octave. Giving you range of 3 octaves for transposition of the sequence.
      
   2. Note Advance (1):
      1.  The Step in the Generated Sequence is Triggered by each Incoming Note On (Gate/Trig) Message, and is Advanced to Next Step. With this you can program your own Rhythms in a Midi Clip.
      2. No Transpose is Applied in Note Advance Mode.

2. ### Auto Regen Cycles
    With Value above 0, Each Sequence will Regenerate after the Provided Number of Cycles. (1-127).
    Regeneration will apply:    
    1. Random Drop (if Applicable)
    2. Random Octave (If Applicable)
    3. Random Velocity (If Applicable)


## Installation: 
1. If Using Mockba Mod and my NodeJs App Server, Copy All the Files into your 662522/AddOns/nodeServer/modules Folder and use web to  start/stop  it.

2. If not using my NodeApp server, you can copy the dropseq file anywhere on your device and run using ssh shell (perhaps someone can better explain the process).
or if using Mockba/Kick Gen Mod, you can put in a launch script in their respective autolaunch directors to launch this in background on Startup.


## Setup:
1. Load the Provided Midi Track in Force
2. Set its midi output to Mockba DropSeq, out ch to 16 (all channels work right now),
3. Set Input to none for Now.
4. Create your instrument track(s) set input to ch: 1 for example: and input port to Mockba DropSeq.
5. Start play on Force, it should start playing Channel 1 sequence.
6. Repeat for other Channels. (go to DropSeq Control Track Midi Control to edit Settings : shift +  clip)
7. First 3 Pages on Midi Control are Setup, Pages 4-7 Are for Setting up Note Weights for each Channel


## Midi Mapping :
1. **Node Advance** (0-1)
   1. When 0: Midi Clock Pulses Play and advance the Sequence.
   2. When 1: Incoming NoteOn Messages play and advance the Sequence Step.
2. **Note Weights (0,9-100)**
   1.  CC: 1-13  : Set Note Weight For Notes C <-> . (Rest) for CH1
   2.  CC: 21-13 : Set Note Weight For Notes C <-> . (Rest) for CH2
   3.  CC: 41-53 : Set Note Weight For Notes C <-> . (Rest) for CH3
   4.  CC: 61-73 : Set Note Weight For Notes C <-> . (Rest) for CH4
3. **Channel On/Off (value >63 = On)**
    1.  CC: 14,34,54,74 : Ch:1-4 Respectively
4. **DropMax (1--50), DropMin (0-50)**
     1.  CC: 15,35,55,75 : Drop Max for Ch 1-4 Resp.
     2.  CC: 91,92,93,94 : Drop min for Ch 1-4 Resp.
5. **OctaveMax, OctaveMin (0-8)**
   1. OctaveMax: CC: 16,36,56,76 : Ch:1-4
   2. OctaveMin: CC: 17,37,57,77 : Ch:1-4
6. **DIV** (1-10) CC: 18,38,58,78: Set Time Divisions for Ch 1-4
       1.  1/16
       2.  1/8
       3.  1/4
       4.  1/2
       5.  1
       6.  1/32
       7.  1/64
       8.  .1/16
       9.  .1/8
       10. .1/4 
7. **Rotate**
   1. CC: 59,60,89,90 : Set Rotation for Ch:1-4
8. **Velocities**
   1. VelocityMax: CC 101-104 for Ch 1-4
   2. VelocityMin: CC 105-108 for Ch 1-4
9.  **Restart** Jumps to Sequence Start Point (any odd value)
   3. CC : 95-98
10. **Gate** (Note Duration % of Step Length based on Time Div ) (10-95%)
   4.  CC : 81-84 for Ch 1-4
11. **Midi Channel (1-15)**
    1.  CC: 85-88 for Ch 1-4
12. **Auto Regen Cycles** (0-127)
    1.  CC: 115-118 for Ch 1-4
13. **Regen** (0-1) Recomputes and Restarts the Sequence. (any odd value)
    1.  CC: 115-118 for Ch 1-4
14. **Instant Update**
    1. CC 109: (0-1) When On (1), Changes Are Applied instantly and Sequence is Regenerated. When Off(0) Changes Applied at the end of Current Sequence.
15. **Clock Control**
    1.  Use External Clock: CC: 100 (0-1) 
        1.  When 0 Internal Clock will be used, and also sent out. Clock Must be started using Start / Stop CC: 80 (0-1)
    2. Stop / Start Internal Clock : CC:80 Values (0-1)
    3. BPM1: CC:39 Values (30-127)
    4. BPM2: CC:40 Values (0-127)
    5. Internal Clock BPM is Set to Values of BPM1 + BPM2 
       1. So if BPM1 = 100 and BPM 2 = 40, The Actual BPM becomes: 140



