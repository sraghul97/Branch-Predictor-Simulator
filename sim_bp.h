#ifndef SIM_BP_H
#define SIM_BP_H

#define Bimodal "bimodal"
#define Gshare "gshare"
#define Hybrid "hybrid"

#include <bitset>
#include <vector>
#include <iostream>
#include <iomanip>
#include <array>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
using namespace std;

typedef struct BrachPredictorModule
{
    uint32_t K = 0;
    uint32_t M1 = 0;
    uint32_t M2 = 0;
    uint32_t N = 0;
    char *BpMode;

    uint32_t TotalPredictions = 0;
    uint32_t TotalMissPredictions = 0;

    vector<uint32_t> PredictionTableBimodal;
    vector<uint32_t> PredictionTableGshare;
    vector<uint32_t> ChooserTableHybrid;
    string BranchHistoryRegister = "";

    void BranchPredictorInitialize()
    {
        BranchHistoryRegister = "";
        PredictionTableBimodal.resize(pow(2, M2));
        PredictionTableGshare.resize(pow(2, M1));
        ChooserTableHybrid.resize(pow(2, K));
        for (uint32_t IndexCounter = 0; IndexCounter < pow(2, M1); IndexCounter++)
            PredictionTableGshare[IndexCounter] = 0x2;
        for (uint32_t IndexCounter = 0; IndexCounter < pow(2, M2); IndexCounter++)
            PredictionTableBimodal[IndexCounter] = 0x2;
        for (uint32_t IndexCounter = 0; IndexCounter < N; IndexCounter++)
            BranchHistoryRegister += "0";
        for (uint32_t IndexCounter = 0; IndexCounter < pow(2, K); IndexCounter++)
            ChooserTableHybrid[IndexCounter] = 0x1;
    }

    string BranchPredictionBimodal(uint32_t TraceAddress, string TraceOutcome, bool HybridGetPredictionFlag = false)
    {

        string TraceAddressBinary = bitset<32>(TraceAddress).to_string();
        uint32_t BpTableIndex = 0;
        if (M2 != 0)
            BpTableIndex = stoull(TraceAddressBinary.substr(32 - 2 - M2, M2), 0, 2);
        string TableOutcome = "n";
        if (PredictionTableBimodal[BpTableIndex] >= 2)
            TableOutcome = "t";

        if (HybridGetPredictionFlag)
            return TableOutcome;

        TotalPredictions += 1;
        if (TraceOutcome != TableOutcome)
            TotalMissPredictions += 1;

        if (TraceOutcome == "t")
        {
            if (PredictionTableBimodal[BpTableIndex] < 3)
                PredictionTableBimodal[BpTableIndex] += 1;
        }
        else
        {
            if (PredictionTableBimodal[BpTableIndex] > 0)
                PredictionTableBimodal[BpTableIndex] -= 1;
        }
        return TableOutcome;
    }

    string BranchPredictionGshare(uint32_t TraceAddress, string TraceOutcome, bool HybridGetPredictionFlag = false)
    {
        string TraceAddressBinary = bitset<32>(TraceAddress).to_string();
        string BranchHistoryRegisterMBits = BranchHistoryRegister;
        string NextBHRDataBit = "";

        for (uint32_t IndexCounter = N; IndexCounter < M1; IndexCounter++)
            BranchHistoryRegisterMBits += "0";

        uint32_t BpTableIndex = 0;
        if (M1 != 0)
            BpTableIndex = (stoull(TraceAddressBinary.substr(32 - 2 - M1, M1), 0, 2)) ^ (stoull(BranchHistoryRegisterMBits, 0, 2));
        //else
        //    BpTableIndex = stoull(BranchHistoryRegisterMBits, 0, 2);
        string TableOutcome = "n";
        if (PredictionTableGshare[BpTableIndex] >= 2)
            TableOutcome = "t";

        if (HybridGetPredictionFlag)
            return TableOutcome;

        TotalPredictions += 1;
        if (TraceOutcome != TableOutcome)
            TotalMissPredictions += 1;

        if (TraceOutcome == "t")
        {
            if (PredictionTableGshare[BpTableIndex] < 3)
                PredictionTableGshare[BpTableIndex] += 1;
            NextBHRDataBit = "1";
        }
        else
        {
            if (PredictionTableGshare[BpTableIndex] > 0)
                PredictionTableGshare[BpTableIndex] -= 1;
            NextBHRDataBit = "0";
        }
        BranchHistoryRegister = (NextBHRDataBit + BranchHistoryRegister);
        BranchHistoryRegister = BranchHistoryRegister.substr(0, (BranchHistoryRegister.length() - 1));
        return TableOutcome;
    }

    void BranchPredictionHybrid(uint32_t TraceAddress, string TraceOutcome)
    {
        string TraceAddressBinary = bitset<32>(TraceAddress).to_string();
        uint32_t chooserTableIndex = 0;
        if (K != 0)
            chooserTableIndex = stoull(TraceAddressBinary.substr(32 - 2 - K, K), 0, 2);
        string BimodalTableOutcome = "";
        string GshareTableOutcome = "";
        string NextBHRDataBit = "";

        if (ChooserTableHybrid[chooserTableIndex] >= 2)
        {
            GshareTableOutcome = BranchPredictionGshare(TraceAddress, TraceOutcome);
            BimodalTableOutcome = BranchPredictionBimodal(TraceAddress, TraceOutcome, true);
        }
        else
        {
            BimodalTableOutcome = BranchPredictionBimodal(TraceAddress, TraceOutcome);
            GshareTableOutcome = BranchPredictionGshare(TraceAddress, TraceOutcome, true);

            if (TraceOutcome == "t")
                NextBHRDataBit = "1";
            else
                NextBHRDataBit = "0";
            BranchHistoryRegister = (NextBHRDataBit + BranchHistoryRegister);
            BranchHistoryRegister = BranchHistoryRegister.substr(0, (BranchHistoryRegister.length() - 1));
        }

        if (!(GshareTableOutcome == BimodalTableOutcome))
        {
            if (BimodalTableOutcome == TraceOutcome)
            {
                if (ChooserTableHybrid[chooserTableIndex] > 0)
                    ChooserTableHybrid[chooserTableIndex] -= 1;
            }
            else if (GshareTableOutcome == TraceOutcome)
            {
                if (ChooserTableHybrid[chooserTableIndex] < 3)
                    ChooserTableHybrid[chooserTableIndex] += 1;
            }
        }
    }

    void PrintOutput(string TraceFileName)
    {
        cout << "COMMAND" << '\n';
        cout << " ./sim ";
        if (strcmp(BpMode, Bimodal) == 0)
            cout << BpMode << " " << M2 << " " << TraceFileName << '\n';
        else if (strcmp(BpMode, Gshare) == 0)
            cout << BpMode << " " << M1 << " " << N << " " << TraceFileName << '\n';
        else if (strcmp(BpMode, Hybrid) == 0)
            cout << BpMode << " " << K << " " << M1 << " " << N << " " << M2 << " " << TraceFileName << '\n';

        cout << "OUTPUT" << '\n';
        cout << " number of predictions:    " << TotalPredictions << '\n';
        cout << " number of mispredictions: " << TotalMissPredictions << '\n';
        cout << " misprediction rate:       " << fixed << setprecision(2) << (((double(TotalMissPredictions)) / (double(TotalPredictions))) * 100) << "%" << '\n';

        if (strcmp(BpMode, Hybrid) == 0)
        {
            cout << "FINAL CHOOSER CONTENTS" << '\n';
            for (uint32_t IndexCounter = 0; IndexCounter < pow(2, K); IndexCounter++)
                cout << IndexCounter << "	" << ChooserTableHybrid[IndexCounter] << '\n';
        }

        if ((strcmp(BpMode, Gshare) == 0) || (strcmp(BpMode, Hybrid) == 0))
        {
            cout << "FINAL GSHARE CONTENTS" << '\n';
            for (uint32_t IndexCounter = 0; IndexCounter < pow(2, M1); IndexCounter++)
                cout << IndexCounter << "	" << PredictionTableGshare[IndexCounter] << '\n';
        }

        if ((strcmp(BpMode, Bimodal) == 0) || (strcmp(BpMode, Hybrid) == 0))
        {
            cout << "FINAL BIMODAL CONTENTS" << '\n';
            for (uint32_t IndexCounter = 0; IndexCounter < pow(2, M2); IndexCounter++)
                cout << IndexCounter << "	" << PredictionTableBimodal[IndexCounter] << '\n';
        }
    }
} BrachPredictorModule;

#endif
