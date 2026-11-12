//  isoprocess.cpp (IsoProcess)
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

#include "isoprocess.hpp"

IsoProcess::IsoProcess()
    : pairIsoProcess{}, m_gasType{GasType::e_air}, m_processType{IsoProcessType::e_isothermal},
      m_AmountSubstance{}, m_MassGas{}, m_KineticEnergyMedium{},
      m_volumeChange{}, m_pressureChange{}, m_temperChange{},
      m_volume{}, m_pressure{}, m_temper{},
      m_experimentList{}, m_meanInvariant{}, m_meanAbsError{}, m_totalRelErrorPercent{}
{
    ResetRandomState();
}

IsoProcess::IsoProcess(int volume, int pressure, int temper, GasType gasType)
    : pairIsoProcess{}, m_gasType{gasType}, m_processType{IsoProcessType::e_isothermal},
      m_AmountSubstance{}, m_MassGas{}, m_KineticEnergyMedium{},
      m_experimentList{}, m_meanInvariant{}, m_meanAbsError{}, m_totalRelErrorPercent{}
{
    SetInitialState(volume, pressure, temper);
}

IsoProcess::~IsoProcess() {}

void IsoProcess::ResetRandomState() {
    auto t_seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 t_gen(static_cast<unsigned int>(t_seed));
    std::uniform_int_distribution<int> t_volumeRand(cnst_volumeMin, cnst_volumeMax);
    std::uniform_int_distribution<int> t_pressureRand(cnst_pressureMin, cnst_pressureMax);
    std::uniform_int_distribution<int> t_temperRand(cnst_temperMin, cnst_temperMax);

    m_volume = t_volumeRand(t_gen);
    m_pressure = t_pressureRand(t_gen);
    m_temper = t_temperRand(t_gen);

    m_volumeChange = static_cast<double>(m_volume);
    m_pressureChange = static_cast<double>(m_pressure);
    m_temperChange = static_cast<double>(m_temper);

    RecalculateAllThermodynamics();
}

void IsoProcess::SetInitialState(int volume, int pressure, int temper) {
    m_volume = CheckValue(volume, ValueType::volume);
    m_pressure = CheckValue(pressure, ValueType::pressure);
    m_temper = CheckValue(temper, ValueType::temper);

    m_volumeChange = static_cast<double>(m_volume);
    m_pressureChange = static_cast<double>(m_pressure);
    m_temperChange = static_cast<double>(m_temper);

    RecalculateAllThermodynamics();
}

void IsoProcess::SetGasType(GasType gasType) {
    m_gasType = gasType;
    RecalculateAllThermodynamics();
}

void IsoProcess::SetIsoProcessType(IsoProcessType processType) {
    m_processType = processType;
    m_volumeChange = static_cast<double>(m_volume);
    m_pressureChange = static_cast<double>(m_pressure);
    m_temperChange = static_cast<double>(m_temper);
    ClearExperimentList();
}

void IsoProcess::SetAmountSubstance() {
    double t_temperK = static_cast<double>(m_temper) + cnst_K;
    m_AmountSubstance = (static_cast<double>(m_pressure) * static_cast<double>(m_volume)) / (cnst_R * t_temperK);
}

void IsoProcess::SetMassGas(GasType gasType) {
    m_gasType = gasType;
    double t_temperK = static_cast<double>(m_temper) + cnst_K;
    if (m_AmountSubstance > 0.0) {
        m_MassGas = m_AmountSubstance * GetMolarMass(m_gasType);
    }
    else {
        m_MassGas = (static_cast<double>(m_pressure) * static_cast<double>(m_volume) * GetMolarMass(m_gasType)) / (cnst_R * t_temperK);
    }
}

void IsoProcess::SetKineticEnergyMedium() {
    double t_temperK = m_temperChange + cnst_K;
    m_KineticEnergyMedium = (3.0 / 2.0) * cnst_k * t_temperK;
}

void IsoProcess::RecalculateAllThermodynamics() {
    SetAmountSubstance();
    SetMassGas(m_gasType);
    SetKineticEnergyMedium();
}

PairIsoProcess IsoProcess::ChangesState(double deltaValue, IsoProcessType isoProcessType) {
    double t_T0_K = static_cast<double>(m_temper) + cnst_K;
    double t_P0 = static_cast<double>(m_pressure);
    double t_V0 = static_cast<double>(m_volume);

    switch (isoProcessType)
    {
    case IsoProcessType::e_isothermal: {
        // T = T0 = const | P * V = P0 * V0
        m_volumeChange += deltaValue;
        if (m_volumeChange < 2.0) { m_volumeChange = 2.0; }
        if (m_volumeChange > 40.0) { m_volumeChange = 40.0; }

        m_pressureChange = (t_P0 * t_V0) / m_volumeChange;
        m_temperChange = static_cast<double>(m_temper);
        pairIsoProcess = { m_pressureChange, m_volumeChange };
        return pairIsoProcess;
    }
    case IsoProcessType::e_isobaric: {
        // P = P0 = const | V / T = V0 / T0
        m_temperChange += deltaValue;
        if (m_temperChange < -150.0) { m_temperChange = -150.0; }
        if (m_temperChange > 250.0) { m_temperChange = 250.0; }

        double t_T_new_K = m_temperChange + cnst_K;
        m_volumeChange = t_V0 * (t_T_new_K / t_T0_K);
        m_pressureChange = t_P0;
        SetKineticEnergyMedium();
        pairIsoProcess = { m_volumeChange, m_temperChange };
        return pairIsoProcess;
    }
    case IsoProcessType::e_isochoric: {
        // V = V0 = const | P / T = P0 / T0
        m_temperChange += deltaValue;
        if (m_temperChange < -150.0) { m_temperChange = -150.0; }
        if (m_temperChange > 250.0) { m_temperChange = 250.0; }

        double t_T_new_K = m_temperChange + cnst_K;
        m_pressureChange = t_P0 * (t_T_new_K / t_T0_K);
        m_volumeChange = t_V0;
        SetKineticEnergyMedium();
        pairIsoProcess = { m_pressureChange, m_temperChange };
        return pairIsoProcess;
    }
    default:
        return { 0.0, 0.0 };
    }
}

void IsoProcess::ApplySliderValue(double newPrimaryValue) {
    if (m_processType == IsoProcessType::e_isothermal) {
        double t_delta = newPrimaryValue - m_volumeChange;
        ChangesState(t_delta, IsoProcessType::e_isothermal);
    }
    else {
        double t_delta = newPrimaryValue - m_temperChange;
        ChangesState(t_delta, m_processType);
    }
}

void IsoProcess::RecordExperimentStep(double sensorNoisePercent) {
    auto t_seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 t_gen(static_cast<unsigned int>(t_seed));
    std::uniform_real_distribution<double> t_noiseDist(-sensorNoisePercent / 100.0, sensorNoisePercent / 100.0);

    double t_noiseFactor = 1.0 + t_noiseDist(t_gen);

    ExperimentStep t_step{};
    t_step.m_stepIndex = static_cast<unsigned int>(m_experimentList.size() + 1);
    t_step.m_volumeL = std::round(m_volumeChange * 100.0) / 100.0;
    t_step.m_pressureKPa = std::round((m_pressureChange * t_noiseFactor) * 10.0) / 10.0;
    t_step.m_temperC = std::round(m_temperChange * 10.0) / 10.0;
    t_step.m_temperK = t_step.m_temperC + cnst_K;

    if (m_processType == IsoProcessType::e_isothermal) {
        t_step.m_constInvariant = t_step.m_pressureKPa * t_step.m_volumeL;
    }
    else if (m_processType == IsoProcessType::e_isobaric) {
        t_step.m_constInvariant = t_step.m_volumeL / t_step.m_temperK;
    }
    else {
        t_step.m_constInvariant = t_step.m_pressureKPa / t_step.m_temperK;
    }

    m_experimentList.push_back(t_step);
    CalculateErrors();
}

void IsoProcess::AddManualExperimentStep(double volumeL, double pressureKPa, double temperC) {
    ExperimentStep t_step{};
    t_step.m_stepIndex = static_cast<unsigned int>(m_experimentList.size() + 1);
    t_step.m_volumeL = volumeL;
    t_step.m_pressureKPa = pressureKPa;
    t_step.m_temperC = temperC;
    t_step.m_temperK = temperC + cnst_K;

    if (m_processType == IsoProcessType::e_isothermal) {
        t_step.m_constInvariant = t_step.m_pressureKPa * t_step.m_volumeL;
    }
    else if (m_processType == IsoProcessType::e_isobaric) {
        t_step.m_constInvariant = t_step.m_volumeL / t_step.m_temperK;
    }
    else {
        t_step.m_constInvariant = t_step.m_pressureKPa / t_step.m_temperK;
    }

    m_experimentList.push_back(t_step);
    CalculateErrors();
}


void IsoProcess::CalculateErrors() {
    if (m_experimentList.empty()) {
        m_meanInvariant = 0.0;
        m_meanAbsError = 0.0;
        m_totalRelErrorPercent = 0.0;
        return;
    }

    double t_sumInvariant = 0.0;
    for (size_t i = 0; i < m_experimentList.size(); i++) {
        t_sumInvariant += m_experimentList[i].m_constInvariant;
    }
    m_meanInvariant = t_sumInvariant / static_cast<double>(m_experimentList.size());

    double t_sumAbsError = 0.0;
    for (size_t i = 0; i < m_experimentList.size(); i++) {
        m_experimentList[i].m_absError = std::abs(m_experimentList[i].m_constInvariant - m_meanInvariant);
        if (m_meanInvariant != 0.0) {
            m_experimentList[i].m_relErrorPercent = (m_experimentList[i].m_absError / m_meanInvariant) * 100.0;
        }
        t_sumAbsError += m_experimentList[i].m_absError;
    }

    m_meanAbsError = t_sumAbsError / static_cast<double>(m_experimentList.size());
    if (m_meanInvariant != 0.0) {
        m_totalRelErrorPercent = (m_meanAbsError / m_meanInvariant) * 100.0;
    }
}

void IsoProcess::ClearExperimentList() {
    m_experimentList.clear();
    m_meanInvariant = 0.0;
    m_meanAbsError = 0.0;
    m_totalRelErrorPercent = 0.0;
}

void IsoProcess::PrintConsoleReport() {
    std::cout << "========================================" << std::endl;
    std::cout << "Gas: " << GetGasName() << " | Process: " << GetProcessName() << std::endl;
    std::cout << "Mass m: " << GetMassGasGrams() << " g | Nu: " << m_AmountSubstance << " mol" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (size_t i = 0; i < m_experimentList.size(); i++) {
        const auto& t_row = m_experimentList[i];
        std::cout << "#" << t_row.m_stepIndex
                  << " | V=" << t_row.m_volumeL << " L"
                  << " | P=" << t_row.m_pressureKPa << " kPa"
                  << " | T=" << t_row.m_temperK << " K"
                  << " | Const=" << t_row.m_constInvariant
                  << " | dC=" << t_row.m_absError
                  << " | eps=" << t_row.m_relErrorPercent << "%" << std::endl;
    }
    std::cout << "Mean Const: " << m_meanInvariant
              << " | Mean Abs Error: " << m_meanAbsError
              << " | Rel Error: " << m_totalRelErrorPercent << "%" << std::endl;
}

double IsoProcess::GetMolarMass(GasType gasType) const {
    switch (gasType)
    {
    case GasType::e_air: return 0.029;
    case GasType::e_acetylene: return 0.026;
    case GasType::e_methane: return 0.016;
    case GasType::e_argon: return 0.040;
    case GasType::e_carbonDioxide: return 0.044;
    default: return 0.029;
    }
}

int IsoProcess::CheckValue(int value, ValueType type) {
    int tmp_iter = 0;
    switch (type)
    {
    case ValueType::volume: tmp_iter = 0; break;
    case ValueType::pressure: tmp_iter = 1; break;
    case ValueType::temper: tmp_iter = 2; break;
    default: tmp_iter = 0; break;
    }

    bool valueDiap = (value >= tmp_cnstListValue[tmp_iter][0] && value <= tmp_cnstListValue[tmp_iter][1]);
    if (valueDiap) { return value; }
    else {
        return ((value < tmp_cnstListValue[tmp_iter][0]) ? (tmp_cnstListValue[tmp_iter][0]) : (tmp_cnstListValue[tmp_iter][1]));
    }
}

GasType IsoProcess::GetGasType() const { return m_gasType; }
IsoProcessType IsoProcess::GetIsoProcessType() const { return m_processType; }

std::string IsoProcess::GetGasName() const {
    switch (m_gasType) {
    case GasType::e_air: return "Air (M=29 g/mol)";
    case GasType::e_acetylene: return "Acetylene C2H2 (M=26 g/mol)";
    case GasType::e_methane: return "Methane CH4 (M=16 g/mol)";
    case GasType::e_argon: return "Argon Ar (M=40 g/mol)";
    case GasType::e_carbonDioxide: return "CO2 (M=44 g/mol)";
    default: return "Air";
    }
}

std::string IsoProcess::GetProcessName() const {
    switch (m_processType) {
    case IsoProcessType::e_isothermal: return "Isothermal (T = const, Boyle-Mariotte)";
    case IsoProcessType::e_isobaric: return "Isobaric (P = const, Gay-Lussac)";
    case IsoProcessType::e_isochoric: return "Isochoric (V = const, Charles)";
    default: return "Isothermal";
    }
}

double IsoProcess::GetVolumeCurrent() const { return m_volumeChange; }
double IsoProcess::GetPressureCurrent() const { return m_pressureChange; }
double IsoProcess::GetTemperCurrentC() const { return m_temperChange; }
double IsoProcess::GetTemperCurrentK() const { return m_temperChange + cnst_K; }

int IsoProcess::GetVolumeInit() const { return m_volume; }
int IsoProcess::GetPressureInit() const { return m_pressure; }
int IsoProcess::GetTemperInitC() const { return m_temper; }

double IsoProcess::GetAmountSubstance() const { return m_AmountSubstance; }
double IsoProcess::GetMassGasKg() const { return m_MassGas; }
double IsoProcess::GetMassGasGrams() const { return m_MassGas * 1000.0; }
double IsoProcess::GetKineticEnergyMedium() const { return m_KineticEnergyMedium; }
double IsoProcess::GetMolarMassCurrent() const { return GetMolarMass(m_gasType); }

const std::vector<ExperimentStep>& IsoProcess::GetExperimentList() const { return m_experimentList; }
double IsoProcess::GetMeanInvariant() const { return m_meanInvariant; }
double IsoProcess::GetMeanAbsError() const { return m_meanAbsError; }
double IsoProcess::GetTotalRelErrorPercent() const { return m_totalRelErrorPercent; }
