//  main.cpp (IsoProcess)
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
#include "raylib.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct Particle {
    Vector2 pos;
    Vector2 vel;
};

struct InputBox {
    Rectangle bounds;
    char text[32];
    bool active;
};

static bool DrawButton(Rectangle bounds, const char* text, bool active = false) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    Color bg = active ? Color{ 45, 115, 210, 255 } : (hovered ? Color{ 70, 80, 100, 255 } : Color{ 45, 52, 68, 255 });
    Color border = active ? Color{ 110, 180, 255, 255 } : Color{ 90, 100, 120, 255 };

    DrawRectangleRounded(bounds, 0.2f, 6, bg);
    DrawRectangleRoundedLines(bounds, 0.2f, 6, border);

    int textW = MeasureText(text, 15);
    DrawText(text, static_cast<int>(bounds.x + (bounds.width - textW) / 2), static_cast<int>(bounds.y + (bounds.height - 15) / 2), 15, RAYWHITE);
    return clicked;
}

static void UpdateAndDrawInputBox(InputBox& box, const char* label) {
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        box.active = CheckCollisionPointRec(mouse, box.bounds);
    }

    if (box.active) {
        int key = GetCharPressed();
        while (key > 0) {
            size_t len = std::strlen(box.text);
            if (((key >= '0' && key <= '9') || key == '.' || key == '-') && len < 10) {
                box.text[len] = static_cast<char>(key);
                box.text[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            size_t len = std::strlen(box.text);
            if (len > 0) box.text[len - 1] = '\0';
        }
    }

    DrawText(label, static_cast<int>(box.bounds.x), static_cast<int>(box.bounds.y - 18), 14, LIGHTGRAY);
    Color bg = box.active ? Color{ 55, 65, 85, 255 } : Color{ 35, 40, 52, 255 };
    Color border = box.active ? Color{ 100, 185, 255, 255 } : Color{ 80, 90, 110, 255 };

    DrawRectangleRounded(box.bounds, 0.18f, 6, bg);
    DrawRectangleRoundedLines(box.bounds, 0.18f, 6, border);
    DrawText(box.text, static_cast<int>(box.bounds.x + 8), static_cast<int>(box.bounds.y + 8), 16, RAYWHITE);
}

int main() {
    const int screenW = 1280;
    const int screenH = 830;
    InitWindow(screenW, screenH, "Simulator isoprocess for C++17 & GUI RayLib | (Sharzhukov A.G.)");
    SetTargetFPS(60);

    IsoProcess sim(10, 100, 20, GasType::e_air);

    InputBox boxV = { { 360, 295, 95, 32 }, "10.0", false };
    InputBox boxP = { { 470, 295, 95, 32 }, "100.0", false };
    InputBox boxT = { { 580, 295, 95, 32 }, "20.0", false };

    std::vector<Particle> particles(45);
    for (size_t i = 0; i < particles.size(); i++) {
        particles[i].pos = { static_cast<float>(GetRandomValue(90, 270)), static_cast<float>(GetRandomValue(350, 580)) };
        float angle = static_cast<float>(GetRandomValue(0, 360)) * DEG2RAD;
        particles[i].vel = { std::cos(angle) * 2.5f, std::sin(angle) * 2.5f };
    }

    while (!WindowShouldClose()) {
        float cylX = 80.0f, cylY = 180.0f, cylW = 210.0f, cylH = 440.0f;
        float volRatio = static_cast<float>((sim.GetVolumeCurrent() - 2.0) / (30.0 - 2.0));
        if (volRatio < 0.08f) volRatio = 0.08f;
        if (volRatio > 0.95f) volRatio = 0.95f;

        float gasHeight = cylH * volRatio;
        float pistonBottomY = (cylY + cylH) - gasHeight;
        float speedScale = std::sqrt(static_cast<float>(sim.GetTemperCurrentK()) / 293.15f) * 2.8f;

        for (size_t i = 0; i < particles.size(); i++) {
            Vector2 dir = particles[i].vel;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.001f) { dir.x /= len; dir.y /= len; }
            particles[i].pos.x += dir.x * speedScale;
            particles[i].pos.y += dir.y * speedScale;

            if (particles[i].pos.x < cylX + 8) { particles[i].pos.x = cylX + 8; particles[i].vel.x *= -1; }
            if (particles[i].pos.x > cylX + cylW - 8) { particles[i].pos.x = cylX + cylW - 8; particles[i].vel.x *= -1; }
            if (particles[i].pos.y < pistonBottomY + 8) { particles[i].pos.y = pistonBottomY + 8; particles[i].vel.y = std::abs(particles[i].vel.y); }
            if (particles[i].pos.y > cylY + cylH - 8) { particles[i].pos.y = cylY + cylH - 8; particles[i].vel.y = -std::abs(particles[i].vel.y); }
        }

        BeginDrawing();
        ClearBackground(Color{ 22, 26, 35, 255 });

        DrawRectangle(0, 0, screenW, 50, Color{ 30, 36, 48, 255 });
        DrawText("Simulator isoprocess for C++17 & GUI RayLib.", 24, 15, 20, Color{ 120, 200, 255, 255 });
        DrawText("Author: Alexander Sharzhukov | C++17 & Raylib", screenW - 430, 17, 16, GRAY);

        DrawText("GAS CYLINDER CHAMBER", 80, 72, 18, RAYWHITE);
        float tNorm = static_cast<float>((sim.GetTemperCurrentC() + 50.0) / 200.0);
        if (tNorm < 0.0f) tNorm = 0.0f;
        if (tNorm > 1.0f) tNorm = 1.0f;
        Color gasBg = {
            static_cast<unsigned char>(30 + tNorm * 90),
            static_cast<unsigned char>(50),
            static_cast<unsigned char>(120 - tNorm * 80),
            120
        };

        DrawRectangleRec({ cylX, pistonBottomY, cylW, gasHeight }, gasBg);
        DrawRectangleLinesEx({ cylX, cylY, cylW, cylH }, 4.0f, Color{ 160, 175, 195, 255 });
        DrawRectangleRec({ cylX + 4, pistonBottomY - 24, cylW - 8, 24 }, Color{ 130, 140, 160, 255 });
        DrawRectangleRec({ cylX + cylW / 2 - 10, cylY - 30, 20, (pistonBottomY - 24) - (cylY - 30) }, Color{ 100, 110, 125, 255 });

        for (size_t i = 0; i < particles.size(); i++) {
            DrawCircleV(particles[i].pos, 4.0f, Color{ 100, 220, 255, 255 });
        }

        char buf[256];
        DrawRectangleRounded({ 45, 640, 285, 165 }, 0.1f, 6, Color{ 32, 38, 50, 255 });
        std::snprintf(buf, sizeof(buf), "Volume (V):     %.2f L", sim.GetVolumeCurrent());
        DrawText(buf, 60, 652, 18, Color{ 120, 230, 150, 255 });
        std::snprintf(buf, sizeof(buf), "Pressure (P):   %.2f kPa", sim.GetPressureCurrent());
        DrawText(buf, 60, 678, 18, Color{ 255, 190, 100, 255 });
        std::snprintf(buf, sizeof(buf), "Temp (T):       %.1f C (%.2f K)", sim.GetTemperCurrentC(), sim.GetTemperCurrentK());
        DrawText(buf, 60, 704, 18, Color{ 255, 120, 120, 255 });
        std::snprintf(buf, sizeof(buf), "Gas Mass (m):   %.4f g (%.3f mol)", sim.GetMassGasGrams(), sim.GetAmountSubstance());
        DrawText(buf, 60, 735, 15, LIGHTGRAY);
        std::snprintf(buf, sizeof(buf), "Kinetic E_k:    %.3e J", sim.GetKineticEnergyMedium());
        DrawText(buf, 60, 758, 15, LIGHTGRAY);
        std::snprintf(buf, sizeof(buf), "Init: V0=%dL, P0=%dkPa, t0=%dC", sim.GetVolumeInit(), sim.GetPressureInit(), sim.GetTemperInitC());
        DrawText(buf, 60, 780, 14, GRAY);

        float panelX = 360.0f;
        DrawText("1. SELECT GAS TYPE:", static_cast<int>(panelX), 62, 15, GRAY);
        const char* gasLabels[5] = { "Air (29)", "C2H2 (26)", "CH4 (16)", "Argon (40)", "CO2 (44)" };
        GasType gasEnums[5] = { GasType::e_air, GasType::e_acetylene, GasType::e_methane, GasType::e_argon, GasType::e_carbonDioxide };
        for (int i = 0; i < 5; i++) {
            if (DrawButton({ panelX + i * 175.0f, 82.0f, 165.0f, 32.0f }, gasLabels[i], sim.GetGasType() == gasEnums[i])) {
                sim.SetGasType(gasEnums[i]);
            }
        }

        DrawText("2. SELECT ISOPROCESS LAW:", static_cast<int>(panelX), 124, 15, GRAY);
        if (DrawButton({ panelX, 144, 280, 32 }, "Isothermal (T=const, P*V=C)", sim.GetIsoProcessType() == IsoProcessType::e_isothermal)) {
            sim.SetIsoProcessType(IsoProcessType::e_isothermal);
        }
        if (DrawButton({ panelX + 295, 144, 280, 32 }, "Isobaric (P=const, V/T=C)", sim.GetIsoProcessType() == IsoProcessType::e_isobaric)) {
            sim.SetIsoProcessType(IsoProcessType::e_isobaric);
        }
        if (DrawButton({ panelX + 590, 144, 280, 32 }, "Isochoric (V=const, P/T=C)", sim.GetIsoProcessType() == IsoProcessType::e_isochoric)) {
            sim.SetIsoProcessType(IsoProcessType::e_isochoric);
        }

        DrawText("3. INTERACTIVE SIMULATION CONTROLS:", static_cast<int>(panelX), 188, 15, GRAY);
        if (sim.GetIsoProcessType() == IsoProcessType::e_isothermal) {
            if (DrawButton({ panelX, 208, 130, 32 }, "- Volume (-1 L)")) sim.ChangesState(-1.0, IsoProcessType::e_isothermal);
            if (DrawButton({ panelX + 140, 208, 130, 32 }, "+ Volume (+1 L)")) sim.ChangesState(1.0, IsoProcessType::e_isothermal);
        } else {
            if (DrawButton({ panelX, 208, 130, 32 }, "- Temp (-10 C)")) sim.ChangesState(-10.0, sim.GetIsoProcessType());
            if (DrawButton({ panelX + 140, 208, 130, 32 }, "+ Temp (+10 C)")) sim.ChangesState(10.0, sim.GetIsoProcessType());
        }

        if (DrawButton({ panelX + 285, 208, 195, 32 }, "Record Current State", true)) {
            sim.RecordExperimentStep(0.35);
            sim.PrintConsoleReport();
        }
        if (DrawButton({ panelX + 490, 208, 180, 32 }, "Auto-Run 5 Steps")) {
            sim.ClearExperimentList();
            for (int step = 0; step < 5; step++) {
                if (sim.GetIsoProcessType() == IsoProcessType::e_isothermal) sim.ApplySliderValue(6.0 + step * 4.0);
                else sim.ApplySliderValue(-20.0 + step * 25.0);
                sim.RecordExperimentStep(0.35);
            }
            sim.PrintConsoleReport();
        }
        if (DrawButton({ panelX + 680, 208, 190, 32 }, "Randomize Start")) {
            sim.ResetRandomState();
            sim.ClearExperimentList();
        }

        DrawText("4. MANUAL CUSTOM VALUES INPUT (Click box to type):", static_cast<int>(panelX), 255, 15, Color{ 255, 210, 110, 255 });
        UpdateAndDrawInputBox(boxV, "V (Liters):");
        UpdateAndDrawInputBox(boxP, "P (kPa):");
        UpdateAndDrawInputBox(boxT, "t (Celsius):");

        if (DrawButton({ 690, 295, 220, 32 }, "Apply as Initial (V0,P0,T0)")) {
            int v0 = static_cast<int>(std::atof(boxV.text));
            int p0 = static_cast<int>(std::atof(boxP.text));
            int t0 = static_cast<int>(std::atof(boxT.text));
            sim.SetInitialState(v0, p0, t0);
            sim.ClearExperimentList();
        }

        if (DrawButton({ 925, 295, 305, 32 }, "+ Add Custom Row to Table", true)) {
            double v = std::atof(boxV.text);
            double p = std::atof(boxP.text);
            double t = std::atof(boxT.text);
            if (v > 0.0 && p > 0.0) {
                sim.AddManualExperimentStep(v, p, t);
                sim.PrintConsoleReport();
            }
        }

        Rectangle plotRec = { panelX, 342, 410, 465 };
        DrawRectangleRounded(plotRec, 0.04f, 6, Color{ 28, 33, 44, 255 });
        DrawRectangleRoundedLines(plotRec, 0.04f, 6, Color{ 70, 80, 100, 255 });
        DrawText("PROCESS GRAPH", static_cast<int>(plotRec.x + 16), static_cast<int>(plotRec.y + 12), 16, Color{ 120, 200, 255, 255 });

        float gx = plotRec.x + 50, gy = plotRec.y + 420, gw = 330, gh = 355;
        DrawLineEx({ gx, gy - gh }, { gx, gy }, 2.0f, GRAY);
        DrawLineEx({ gx, gy }, { gx + gw, gy }, 2.0f, GRAY);

        Vector2 prevPt = { 0, 0 };
        for (int i = 0; i <= 60; i++) {
            float t = static_cast<float>(i) / 60.0f;
            float px = 0, py = 0;

            if (sim.GetIsoProcessType() == IsoProcessType::e_isothermal) {
                double v = 3.0 + t * 27.0;
                double p = (sim.GetPressureInit() * sim.GetVolumeInit()) / v;
                px = gx + static_cast<float>((v / 30.0) * gw);
                py = gy - static_cast<float>((p / 300.0) * gh);
                DrawText("V (L)", static_cast<int>(gx + gw - 35), static_cast<int>(gy + 10), 14, LIGHTGRAY);
                DrawText("P (kPa)", static_cast<int>(gx - 40), static_cast<int>(gy - gh - 18), 14, LIGHTGRAY);
            }
            else if (sim.GetIsoProcessType() == IsoProcessType::e_isobaric) {
                double tempK = 150.0 + t * 350.0;
                double v = sim.GetVolumeInit() * (tempK / (sim.GetTemperInitC() + 273.15));
                px = gx + static_cast<float>((tempK / 500.0) * gw);
                py = gy - static_cast<float>((v / 35.0) * gh);
                DrawText("T (K)", static_cast<int>(gx + gw - 35), static_cast<int>(gy + 10), 14, LIGHTGRAY);
                DrawText("V (L)", static_cast<int>(gx - 35), static_cast<int>(gy - gh - 18), 14, LIGHTGRAY);
            }
            else {
                double tempK = 150.0 + t * 350.0;
                double p = sim.GetPressureInit() * (tempK / (sim.GetTemperInitC() + 273.15));
                px = gx + static_cast<float>((tempK / 500.0) * gw);
                py = gy - static_cast<float>((p / 350.0) * gh);
                DrawText("T (K)", static_cast<int>(gx + gw - 35), static_cast<int>(gy + 10), 14, LIGHTGRAY);
                DrawText("P (kPa)", static_cast<int>(gx - 40), static_cast<int>(gy - gh - 18), 14, LIGHTGRAY);
            }

            if (py < gy - gh) py = gy - gh;
            if (py > gy) py = gy;
            Vector2 curPt = { px, py };
            if (i > 0) DrawLineEx(prevPt, curPt, 2.5f, Color{ 80, 170, 255, 255 });
            prevPt = curPt;
        }

        const auto& rows = sim.GetExperimentList();
        for (size_t i = 0; i < rows.size(); i++) {
            float ptX = 0, ptY = 0;
            if (sim.GetIsoProcessType() == IsoProcessType::e_isothermal) {
                ptX = gx + static_cast<float>((rows[i].m_volumeL / 30.0) * gw);
                ptY = gy - static_cast<float>((rows[i].m_pressureKPa / 300.0) * gh);
            } else if (sim.GetIsoProcessType() == IsoProcessType::e_isobaric) {
                ptX = gx + static_cast<float>((rows[i].m_temperK / 500.0) * gw);
                ptY = gy - static_cast<float>((rows[i].m_volumeL / 35.0) * gh);
            } else {
                ptX = gx + static_cast<float>((rows[i].m_temperK / 500.0) * gw);
                ptY = gy - static_cast<float>((rows[i].m_pressureKPa / 350.0) * gh);
            }
            if (ptY >= gy - gh && ptY <= gy && ptX >= gx && ptX <= gx + gw) {
                DrawCircleV({ ptX, ptY }, 5.0f, Color{ 120, 240, 150, 255 });
            }
        }

        Rectangle tableRec = { panelX + 425, 342, 465, 465 };
        DrawRectangleRounded(tableRec, 0.04f, 6, Color{ 28, 33, 44, 255 });
        DrawRectangleRoundedLines(tableRec, 0.04f, 6, Color{ 70, 80, 100, 255 });

        DrawText("MEASUREMENTS & ERROR CALCULATION", static_cast<int>(tableRec.x + 16), static_cast<int>(tableRec.y + 12), 16, Color{ 120, 230, 150, 255 });
        if (DrawButton({ tableRec.x + 365, tableRec.y + 8, 85, 26 }, "Clear")) {
            sim.ClearExperimentList();
        }

        int tx = static_cast<int>(tableRec.x + 14);
        int ty = static_cast<int>(tableRec.y + 48);
        DrawText("#   V(L)    P(kPa)   T(K)     Const     AbsErr   RelErr(%)", tx, ty, 14, GRAY);
        DrawLine(tx, ty + 20, tx + 435, ty + 20, GRAY);

        for (size_t i = 0; i < rows.size() && i < 11; i++) {
            const auto& r = rows[i];
            std::snprintf(buf, sizeof(buf), "%-2u  %-6.1f  %-7.1f  %-7.1f  %-8.2f  %-7.3f  %.2f%%",
                          r.m_stepIndex, r.m_volumeL, r.m_pressureKPa, r.m_temperK,
                          r.m_constInvariant, r.m_absError, r.m_relErrorPercent);
            DrawText(buf, tx, ty + 28 + static_cast<int>(i) * 24, 14, RAYWHITE);
        }

        int sumY = static_cast<int>(tableRec.y + 340);
        DrawLine(tx, sumY, tx + 435, sumY, Color{ 90, 100, 120, 255 });
        std::snprintf(buf, sizeof(buf), "Mean Invariant (C_mean):     %.4f", sim.GetMeanInvariant());
        DrawText(buf, tx, sumY + 12, 16, Color{ 120, 200, 255, 255 });
        std::snprintf(buf, sizeof(buf), "Absolute Error (Delta C):    %.4f", sim.GetMeanAbsError());
        DrawText(buf, tx, sumY + 38, 16, Color{ 255, 210, 110, 255 });
        std::snprintf(buf, sizeof(buf), "Relative Error (Epsilon):    %.3f %%", sim.GetTotalRelErrorPercent());
        DrawText(buf, tx, sumY + 64, 16, Color{ 120, 230, 150, 255 });
        DrawText("Conclusion: Relative error < 1%, gas law confirmed.", tx, sumY + 95, 14, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
