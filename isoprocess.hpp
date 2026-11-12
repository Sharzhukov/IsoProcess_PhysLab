//  isoprocess.hpp (IsoProcess)
//  Simulator isoprocess for C++17 & GUI RayLib, for Physics lab.
//
//  Copyright (C) 2026 Alexander Sharzhukov
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>
#include <vector>
#include <string>
#include <iomanip>

enum class GasType { e_air, e_acetylene, e_methane, e_argon, e_carbonDioxide };
enum class IsoProcessType {
    e_isothermal, // T - const (Boyle-Mariotte)
    e_isobaric,   // P - const (Gay-Lussac)
    e_isochoric   // V - const (Charles)
};
enum class ValueType { volume, pressure, temper };

struct PairIsoProcess {
    double FindValue;
    double ChangesValue;
};

struct ExperimentStep {
    unsigned int m_stepIndex;
    double m_volumeL;
    double m_pressureKPa;
    double m_temperC;
    double m_temperK;
    double m_constInvariant;   // P*V, V/T or P/T
    double m_absError;         // |C_i - C_mean|
    double m_relErrorPercent;  // (absError / C_mean) * 100%
};

class IsoProcess
{
private:
    static constexpr int cnst_volumeMin = 5, cnst_volumeMax = 25;
    static constexpr int cnst_pressureMin = 40, cnst_pressureMax = 250;
    static constexpr int cnst_temperMin = -50, cnst_temperMax = 100;


    static constexpr double cnst_R = 8.314;
    static constexpr double cnst_k = 1.38e-23;
    static constexpr double cnst_N_a = 6.022e23;
    static constexpr double cnst_K = 273.15;

    const int tmp_cnstListValue[3][2] = {
        { cnst_volumeMin, cnst_volumeMax },
        { cnst_pressureMin, cnst_pressureMax },
        { cnst_temperMin, cnst_temperMax }
    };

    PairIsoProcess pairIsoProcess;
    GasType m_gasType;
    IsoProcessType m_processType;

    double m_AmountSubstance;
    double m_MassGas;
    double m_KineticEnergyMedium;

    double m_volumeChange;
    double m_pressureChange;
    double m_temperChange;

    int m_volume;
    int m_pressure;
    int m_temper;

    std::vector<ExperimentStep> m_experimentList;
    double m_meanInvariant;
    double m_meanAbsError;
    double m_totalRelErrorPercent;

    int CheckValue(int value, ValueType type);
    double GetMolarMass(GasType gasType) const;

public:
    IsoProcess();
    IsoProcess(int volume, int pressure, int temper, GasType gasType = GasType::e_air);
    ~IsoProcess();

    void ResetRandomState();
    void SetInitialState(int volume, int pressure, int temper);

    void SetGasType(GasType gasType);
    void SetIsoProcessType(IsoProcessType processType);

    void SetAmountSubstance();
    void SetMassGas(GasType gasType);
    void SetKineticEnergyMedium();
    void RecalculateAllThermodynamics();

    PairIsoProcess ChangesState(double deltaValue, IsoProcessType isoProcessType);
    void ApplySliderValue(double newPrimaryValue);

    void AddManualExperimentStep(double volumeL, double pressureKPa, double temperC);
    void RecordExperimentStep(double sensorNoisePercent = 0.4);
    void CalculateErrors();
    void ClearExperimentList();
    void PrintConsoleReport();

    GasType GetGasType() const;
    IsoProcessType GetIsoProcessType() const;
    std::string GetGasName() const;
    std::string GetProcessName() const;

    double GetVolumeCurrent() const;
    double GetPressureCurrent() const;
    double GetTemperCurrentC() const;
    double GetTemperCurrentK() const;

    int GetVolumeInit() const;
    int GetPressureInit() const;
    int GetTemperInitC() const;

    double GetAmountSubstance() const;
    double GetMassGasKg() const;
    double GetMassGasGrams() const;
    double GetKineticEnergyMedium() const;
    double GetMolarMassCurrent() const;

    const std::vector<ExperimentStep>& GetExperimentList() const;
    double GetMeanInvariant() const;
    double GetMeanAbsError() const;
    double GetTotalRelErrorPercent() const;
};
