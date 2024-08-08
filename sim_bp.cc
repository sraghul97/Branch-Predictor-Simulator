#include "sim_bp.h"

int main(int ArgumentCount, char *ArgumentValue[])
{
    FILE *TraceFilePointer;
    char *TraceFileName;
    BrachPredictorModule BranchPredictor;
    string TraceOutcome;
    uint32_t CurrentTraceAddress;

    if (!(ArgumentCount == 4 || ArgumentCount == 5 || ArgumentCount == 7))
    {
        cout << "Error: Wrong number of inputs:" << ArgumentCount - 1 << '\n';
        exit(EXIT_FAILURE);
    }

    BranchPredictor.BpMode = ArgumentValue[1];

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if (strcmp(BranchPredictor.BpMode, Bimodal) == 0)
    {
        if (ArgumentCount != 4)
        {
            cout << "Error: " << BranchPredictor.BpMode << " wrong number of inputs:" << (ArgumentCount - 1);
            exit(EXIT_FAILURE);
        }
        BranchPredictor.M2 = strtoul(ArgumentValue[2], NULL, 10);
        TraceFileName = ArgumentValue[3];
    }
    else if (strcmp(BranchPredictor.BpMode, Gshare) == 0)
    {
        if (ArgumentCount != 5)
        {
            cout << "Error: " << BranchPredictor.BpMode << " wrong number of inputs:" << (ArgumentCount - 1);
            exit(EXIT_FAILURE);
        }
        BranchPredictor.M1 = strtoul(ArgumentValue[2], NULL, 10);
        BranchPredictor.N = strtoul(ArgumentValue[3], NULL, 10);
        TraceFileName = ArgumentValue[4];
    }
    else if (strcmp(BranchPredictor.BpMode, Hybrid) == 0)
    {
        if (ArgumentCount != 7)
        {
            cout << "Error: " << BranchPredictor.BpMode << " wrong number of inputs:" << (ArgumentCount - 1);
            exit(EXIT_FAILURE);
        }
        BranchPredictor.K = strtoul(ArgumentValue[2], NULL, 10);
        BranchPredictor.M1 = strtoul(ArgumentValue[3], NULL, 10);
        BranchPredictor.N = strtoul(ArgumentValue[4], NULL, 10);
        BranchPredictor.M2 = strtoul(ArgumentValue[5], NULL, 10);
        TraceFileName = ArgumentValue[6];
    }
    else
    {
        cout << "Error: Wrong branch predictor name:" << BranchPredictor.BpMode << '\n';
        exit(EXIT_FAILURE);
    }

    BranchPredictor.BranchPredictorInitialize();

    // Open TraceFileName in read mode
    TraceFilePointer = fopen(TraceFileName, "r");
    if (TraceFilePointer == NULL)
    {
        cout << "Error: Unable to open file " << TraceFileName << '\n';
        exit(EXIT_FAILURE);
    }

    char str[2];
    while (fscanf(TraceFilePointer, "%x %s", &CurrentTraceAddress, str) != EOF)
    {
        TraceOutcome = str[0];

        /*      if (TraceOutcome == "t")
                  cout << hex<<CurrentTraceAddress << " t" << '\n';
              else if (TraceOutcome == "n")
                  cout <<hex<< CurrentTraceAddress << " n" << '\n';
        */

        if (strcmp(BranchPredictor.BpMode, Bimodal) == 0)
            BranchPredictor.BranchPredictionBimodal(CurrentTraceAddress, TraceOutcome);
        else if (strcmp(BranchPredictor.BpMode, Gshare) == 0)
            BranchPredictor.BranchPredictionGshare(CurrentTraceAddress, TraceOutcome);
        else if (strcmp(BranchPredictor.BpMode, Hybrid) == 0)
            BranchPredictor.BranchPredictionHybrid(CurrentTraceAddress, TraceOutcome);
        }

    BranchPredictor.PrintOutput(TraceFileName);
    return 0;
}
