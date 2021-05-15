#include "PC_FileIO.c"
const int MAX_SIZE = 500;
float dot[MAX_SIZE][2];

int readFile(TFileHandle & fin)
{
    int counter = 0;
    while(readFloatPC(fin, dot[counter][0]))
    {
        readFloatPC(fin, dot[counter][1]);
        counter++;
    }
    return counter;
}

void displayDone(int size, int plotted)
{
    float drawn = plotted;
    float done = (drawn/size)*100;
    displayTextLine(2, "%i points plotted of %i", plotted+1, size);
    displayBigTextLine(6, "%f %%", done);
}

int encoder(float distance)
{
    const float CM_PER_REV = 3.5; //one motor revolution covers 3.5 cm
    float rotations = distance/CM_PER_REV;
    float counts = 360*rotations;
    return counts;
}

void goOrigin(float & currentx, float & currenty)
{
    bool xdone = false;
    bool ydone = false;
    while (!ydone || !xdone)
    {
        if (!ydone)
            motor[motorC] = -100;
        if (SensorValue[S2] == true)
        {
            motor[motorC] = 0;
            ydone = true;
        }
        if (!xdone)
            motor[motorA] = 100;
        if (SensorValue[S1] == true)
        {
            motor[motorA] = 0;
            xdone = true;
        }
    }
    motor[motorC] = 100;
    wait1Msec(200);
    motor[motorC] = 0;
    ydone = false;
    while (!ydone)
    {
        if (!ydone)
            motor[motorC] = -20;
        if (SensorValue[S2] == true)
        {
            motor[motorC] = 0;
            ydone = true;
        }
    }
    motor[motorA] = -100;
    wait1Msec(200);
    motor[motorA] = 0;
    xdone = false;
    while (!xdone)
    {
        if (!xdone)
            motor[motorA] = 20;
        if (SensorValue[S1] == true)
        {
            motor[motorA] = 0;
            xdone = true;
        }
    }
    currentx = 0;
    currenty = 0;
    nMotorEncoder[motorA] = 0;
    nMotorEncoder[motorC] = 0;
}

void checkPause()
{
    if (getButtonPress(buttonEnter))
    {
        while (getButtonPress(buttonEnter))
        {}
        motor[motorA] = motor[motorB] = motor[motorC] = 0;
        displayString(4, "Paused");
        while (!getButtonPress(buttonEnter))
        {}
        while(getButtonPress(buttonEnter))
        {}
        eraseDisplay();
    }
}

void changePenHeight(int updown)
{
    const int ENCODER = 1000;
    nMotorEncoder[motorB] = 0;
    while (abs(nMotorEncoder[motorB]) < ENCODER)
    {
        motor[motorB] = updown*100;
        checkPause();
    }
    motor[motorB] = 0;
}

void plotPoint(float x, float y, float & currentx, float & currenty)
{
    int xdirection = -1;
    int ydirection = -1;
    bool xdone = false;
    bool ydone = false;
    if (-x > currentx)
        xdirection = 1;
    if (y > currenty)
        ydirection = 1;
    while (!xdone || !ydone)
    {
        if ((nMotorEncoder[motorA] < encoder(-x) && xdirection == 1) || (nMotorEncoder[motorA] > encoder(-x) && xdirection == -1))
            motor[motorA] = xdirection*100;
        else
        {
            xdone = true;
            motor[motorA] = 0;
        }
        if ((nMotorEncoder[motorC] < encoder(y) && ydirection == 1) || (nMotorEncoder[motorC] > encoder(y) && ydirection == -1))
            motor[motorC] = ydirection*100;
        else
        {
            ydone = true;
            motor[motorC] = 0;
        }
        checkPause();
    }
    motor[motorA] = motor[motorC] = 0;
    currentx = -x;
    currenty = y;
}

task main()
{
    SensorType[S1] = sensorEV3_Touch;
    SensorType[S2] = sensorEV3_Touch;
    int file = 0;
    int size  = 0;
    int counter = 0;
    float currentx = 0;
    float currenty = 0;
    bool fileOkay[5];
    TFileHandle fin[5];
    string fileName[5];
    bool fileExists = true;

    fileOkay[0] = openReadPC(fin[0], "file0.txt");
    fileOkay[1] = openReadPC(fin[1], "file1.txt");
    fileOkay[2] = openReadPC(fin[2], "file2.txt");
    fileOkay[3] = openReadPC(fin[3], "file3.txt");
    fileOkay[4] = openReadPC(fin[4], "file4.txt");
    for (int counter = 0; counter < 5; counter++)
    {
        if (fileOkay[counter])
            readTextPC(fin[counter], fileName[counter]);
    }

    while (!getButtonPress(buttonEnter) || !fileExists)
    {
        if (getButtonPress(buttonRight))
        {
            file++;
            while (getButtonPress(buttonRight))
            {}
        eraseDisplay();
        }
        if (getButtonPress(buttonLeft))
        {
            file--;
            while (getButtonPress(buttonLeft))
            {}
            eraseDisplay();
        }
        if (file == -1)
            file = 4;
        if (file == 5)
            file = 0;
        if (fileName[file] == "")
        {
            fileExists = false;
            displayBigTextLine(2, "File not found");
        }
        else
        {
            fileExists = true;
            displayBigTextLine(2, fileName[file]);
        }
    }
    eraseDisplay();
    displayTextLine(2, "file selected is %s.", fileName[file]);
    displayTextLine(4, "Press any button to continue.");
    wait1Msec(1000);
    while (!getButtonPress(buttonAny))
    {}
    while (getButtonPress(buttonAny))
    {}
    eraseDisplay();
    size = readFile(fin[file]);
    goOrigin(currentx, currenty);
    while (dot[counter][0] != -1)
    {
        plotPoint(dot[counter][0], dot[counter][1], currentx, currenty);
        changePenHeight(1);
        changePenHeight(-1);
        counter++;
        eraseDisplay();
        displayDone(size, counter);
        if (getButtonPress(buttonDown))
            goOrigin(currentx, currenty);
    }
    goOrigin(currentx, currenty);
    eraseDisplay();
    displayString(2, "Drawing complete.");
    displayString(4, "Press any button to close.");
    while (!getButtonPress(buttonAny))
    {}
    while (getButtonPress(buttonAny))
    {}
}