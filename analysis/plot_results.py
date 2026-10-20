#!/usr/bin/env python3
"""
Generador de gráficas vectoriales (SVG de calidad de publicación) a partir de los
archivos CSV exportados por G4AnalysisManager en Geant4-DNA:
  1. analysis/ros_kinetics_comparison.svg:
     Curvas G(t) de ·OH, ·OH_eff (post-scavenging por melanina/GXM), e-_aq y H2O2
     en escala logarítmica desde 1 ps hasta 1 us (1e6 ps).
  2. analysis/layer_dosimetry_comparison.svg:
     Comparación de energía depositada (keV) y dosis absorbida (Gy) por capa celular
     (Cápsula/Glucocáliz, Pared/Corteza, Citoplasma y Núcleo).
"""

import csv
import math
import pathlib
import sys
from collections import defaultdict


def read_g4_csv(filepath: pathlib.Path):
    rows = []
    if not filepath.exists():
        return rows
    with filepath.open("r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            for parsed in csv.reader([s]):
                rows.append(parsed)
    return rows


def load_kinetics(csv_path: pathlib.Path):
    rows = read_g4_csv(csv_path)
    by_t = defaultdict(lambda: [0.0, 0.0, 0.0, 0.0, 0])
    for r in rows:
        t_ps = float(r[2])
        acc = by_t[t_ps]
        acc[0] += float(r[7])   # G_OH_raw
        acc[1] += float(r[8])   # G_e_aq_raw
        acc[2] += float(r[9])   # G_H2O2_raw
        acc[3] += float(r[10])  # G_OH_eff
        acc[4] += 1
    times = sorted(by_t.keys())
    data = {"t": times, "oh": [], "eaq": [], "h2o2": [], "oh_eff": []}
    for t in times:
        a = by_t[t]
        n = max(a[4], 1)
        data["oh"].append(a[0] / n)
        data["eaq"].append(a[1] / n)
        data["h2o2"].append(a[2] / n)
        data["oh_eff"].append(a[3] / n)
    return data


def load_dosimetry(csv_path: pathlib.Path):
    rows = read_g4_csv(csv_path)
    edep_kev = [0.0, 0.0, 0.0, 0.0]
    dose_gy = [0.0, 0.0, 0.0, 0.0]
    for r in rows:
        for i in range(4):
            edep_kev[i] += float(r[2 + i]) / 1e3
            dose_gy[i] += float(r[6 + i])
    return edep_kev, dose_gy


def generate_ros_svg(crypto, human, out_path: pathlib.Path):
    w, h = 980, 560
    ml, mr, mt, mb = 85, 40, 65, 75
    pw = w - ml - mr
    ph = h - mt - mb

    def x_pos(t_ps):
        # log10(t_ps) from 0 (1 ps) to 6 (1e6 ps)
        return ml + (math.log10(max(t_ps, 1.0)) / 6.0) * pw

    y_max = 6.0

    def y_pos(g_val):
        return mt + ph - (min(max(g_val, 0.0), y_max) / y_max) * ph

    def poly(times, vals):
        return " ".join(f"{x_pos(t):.1f},{y_pos(v):.1f}" for t, v in zip(times, vals))

    svg = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}" style="font-family: -apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif; background: #ffffff;">',
        f'<text x="{w/2}" y="32" text-anchor="middle" font-size="17" font-weight="bold" fill="#1e293b">Cinética Temporal de Especies Reactivas de Oxígeno (ROS: 1 ps a 1 μs)</text>',
        f'<text x="{w/2}" y="52" text-anchor="middle" font-size="13" fill="#475569">Cryptococcus neoformans (Forma Estándar) vs. Célula Humana Epitelial — Geant4-DNA (e⁻ 30 keV)</text>',
    ]

    # Grid lines Y
    for g in range(0, 7):
        yp = y_pos(g)
        svg.append(f'<line x1="{ml}" y1="{yp:.1f}" x2="{w-mr}" y2="{yp:.1f}" stroke="#e2e8f0" stroke-width="1"/>')
        svg.append(f'<text x="{ml-12}" y="{yp+4:.1f}" text-anchor="end" font-size="12" fill="#334155">{g}.0</text>')

    # Grid lines X (10^0 to 10^6 ps)
    labels_x = [
        (1, "1 ps"), (10, "10 ps"), (100, "100 ps"),
        (1e3, "1 ns"), (1e4, "10 ns"), (1e5, "100 ns"), (1e6, "1 μs")
    ]
    for t_val, lbl in labels_x:
        xp = x_pos(t_val)
        svg.append(f'<line x1="{xp:.1f}" y1="{mt}" x2="{xp:.1f}" y2="{mt+ph}" stroke="#e2e8f0" stroke-width="1" stroke-dasharray="3,3"/>')
        svg.append(f'<text x="{xp:.1f}" y="{mt+ph+24}" text-anchor="middle" font-size="12" fill="#334155">{lbl}</text>')

    # Axes border
    svg.append(f'<rect x="{ml}" y="{mt}" width="{pw}" height="{ph}" fill="none" stroke="#64748b" stroke-width="1.5"/>')

    # Axis titles
    svg.append(f'<text x="{ml + pw/2}" y="{h-20}" text-anchor="middle" font-size="14" font-weight="600" fill="#1e293b">Tiempo Químico Post-Irradiación (escala logarítmica)</text>')
    svg.append(f'<text x="24" y="{mt + ph/2}" text-anchor="middle" font-size="14" font-weight="600" fill="#1e293b" transform="rotate(-90, 24, {mt + ph/2})">Valor G (moléculas / 100 eV)</text>')

    # Plot curves
    series = [
        (crypto["t"], crypto["oh"], "#dc2626", "2.5", "none", "C. neoformans ·OH (bruto)"),
        (human["t"], human["oh"], "#f87171", "2.0", "6,4", "Humana ·OH (bruto)"),
        (crypto["t"], crypto["oh_eff"], "#7c2d12", "3.0", "none", "C. neoformans ·OH_eff (scavenging Melanina/GXM)"),
        (human["t"], human["oh_eff"], "#ea580c", "2.5", "5,4", "Humana ·OH_eff (scavenging basal)"),
        (crypto["t"], crypto["eaq"], "#2563eb", "2.5", "none", "C. neoformans e⁻_aq"),
        (human["t"], human["eaq"], "#60a5fa", "2.0", "6,4", "Humana e⁻_aq"),
        (crypto["t"], crypto["h2o2"], "#059669", "2.5", "none", "C. neoformans H₂O₂"),
        (human["t"], human["h2o2"], "#34d399", "2.0", "6,4", "Humana H₂O₂"),
    ]

    for t_arr, v_arr, color, sw, dash, _ in series:
        dash_attr = f'stroke-dasharray="{dash}"' if dash != "none" else ""
        svg.append(f'<polyline fill="none" stroke="{color}" stroke-width="{sw}" {dash_attr} points="{poly(t_arr, v_arr)}"/>')
        for t_pt, v_pt in zip(t_arr, v_arr):
            svg.append(f'<circle cx="{x_pos(t_pt):.1f}" cy="{y_pos(v_pt):.1f}" r="3" fill="{color}"/>')

    # Legend box
    lx, ly = ml + pw - 350, mt + 14
    svg.append(f'<rect x="{lx}" y="{ly}" width="336" height="190" rx="6" fill="#f8fafc" stroke="#cbd5e1" stroke-width="1"/>')
    for idx, (_, _, color, sw, dash, label) in enumerate(series):
        row_y = ly + 20 + idx * 22
        dash_attr = f'stroke-dasharray="{dash}"' if dash != "none" else ""
        svg.append(f'<line x1="{lx+12}" y1="{row_y-4}" x2="{lx+42}" y2="{row_y-4}" stroke="{color}" stroke-width="{sw}" {dash_attr}/>')
        svg.append(f'<text x="{lx+50}" y="{row_y}" font-size="11.5" fill="#1e293b">{label}</text>')

    svg.append("</svg>")
    out_path.write_text("\n".join(svg), encoding="utf-8")


def generate_dosimetry_svg(crypto_dos, human_dos, out_path: pathlib.Path):
    c_edep, c_gy = crypto_dos
    h_edep, h_gy = human_dos

    w, h = 980, 480
    svg = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}" style="font-family: -apple-system, BlinkMacSystemFont, Segoe UI, Roboto, sans-serif; background: #ffffff;">',
        f'<text x="{w/2}" y="32" text-anchor="middle" font-size="17" font-weight="bold" fill="#1e293b">Microdosimetría Comparativa por Compartimento Celular (10 electrones de 30 keV)</text>',
        f'<text x="{w/2}" y="52" text-anchor="middle" font-size="13" fill="#475569">Izquierda: Energía Depositada E_dep (keV) | Derecha: Dosis Absorbida (Gy = J/kg)</text>',
    ]

    layers = ["Capa Externa\n(Cápsula/Glucoc.)", "Pared Melanina /\nMembrana-Corteza", "Citoplasma", "Núcleo\n(1C vs 2C)"]

    # Panel 1: Edep (keV)
    p1_x, p1_y, p1_w, p1_h = 75, 85, 390, 310
    max_edep = 160.0
    svg.append(f'<rect x="{p1_x}" y="{p1_y}" width="{p1_w}" height="{p1_h}" fill="none" stroke="#64748b" stroke-width="1.5"/>')
    svg.append(f'<text x="{p1_x + p1_w/2}" y="{p1_y - 10}" text-anchor="middle" font-size="14" font-weight="600" fill="#1e293b">Energía Depositada Acumulada (keV)</text>')

    for val in [0, 40, 80, 120, 160]:
        yp = p1_y + p1_h - (val / max_edep) * p1_h
        svg.append(f'<line x1="{p1_x}" y1="{yp:.1f}" x2="{p1_x+p1_w}" y2="{yp:.1f}" stroke="#e2e8f0" stroke-width="1"/>')
        svg.append(f'<text x="{p1_x-8}" y="{yp+4:.1f}" text-anchor="end" font-size="11.5" fill="#334155">{val}</text>')

    for i in range(4):
        gx = p1_x + 28 + i * 92
        bh_c = (c_edep[i] / max_edep) * p1_h
        bh_h = (h_edep[i] / max_edep) * p1_h
        svg.append(f'<rect x="{gx}" y="{p1_y + p1_h - bh_c:.1f}" width="28" height="{bh_c:.1f}" fill="#0284c7" rx="2"/>')
        svg.append(f'<text x="{gx+14}" y="{p1_y + p1_h - bh_c - 5:.1f}" text-anchor="middle" font-size="10.5" font-weight="600" fill="#0369a1">{c_edep[i]:.1f}</text>')
        svg.append(f'<rect x="{gx+32}" y="{p1_y + p1_h - bh_h:.1f}" width="28" height="{bh_h:.1f}" fill="#e11d48" rx="2"/>')
        svg.append(f'<text x="{gx+46}" y="{p1_y + p1_h - bh_h - 5:.1f}" text-anchor="middle" font-size="10.5" font-weight="600" fill="#be123c">{h_edep[i]:.1f}</text>')
        lbl_parts = layers[i].split("\n")
        svg.append(f'<text x="{gx+30}" y="{p1_y + p1_h + 20}" text-anchor="middle" font-size="11" fill="#1e293b">{lbl_parts[0]}</text>')
        if len(lbl_parts) > 1:
            svg.append(f'<text x="{gx+30}" y="{p1_y + p1_h + 34}" text-anchor="middle" font-size="10.5" fill="#475569">{lbl_parts[1]}</text>')

    # Panel 2: Dose (Gy)
    p2_x, p2_y, p2_w, p2_h = 545, 85, 390, 310
    max_gy = 0.10
    svg.append(f'<rect x="{p2_x}" y="{p2_y}" width="{p2_w}" height="{p2_h}" fill="none" stroke="#64748b" stroke-width="1.5"/>')
    svg.append(f'<text x="{p2_x + p2_w/2}" y="{p2_y - 10}" text-anchor="middle" font-size="14" font-weight="600" fill="#1e293b">Dosis Absorbida por Capa (Gy)</text>')

    for val in [0.00, 0.02, 0.04, 0.06, 0.08, 0.10]:
        yp = p2_y + p2_h - (val / max_gy) * p2_h
        svg.append(f'<line x1="{p2_x}" y1="{yp:.1f}" x2="{p2_x+p2_w}" y2="{yp:.1f}" stroke="#e2e8f0" stroke-width="1"/>')
        svg.append(f'<text x="{p2_x-8}" y="{yp+4:.1f}" text-anchor="end" font-size="11.5" fill="#334155">{val:.2f}</text>')

    for i in range(4):
        gx = p2_x + 28 + i * 92
        bh_c = (c_gy[i] / max_gy) * p2_h
        bh_h = (h_gy[i] / max_gy) * p2_h
        svg.append(f'<rect x="{gx}" y="{p2_y + p2_h - bh_c:.1f}" width="28" height="{bh_c:.1f}" fill="#0284c7" rx="2"/>')
        svg.append(f'<text x="{gx+14}" y="{p2_y + p2_h - bh_c - 5:.1f}" text-anchor="middle" font-size="10.5" font-weight="600" fill="#0369a1">{c_gy[i]:.3f}</text>')
        svg.append(f'<rect x="{gx+32}" y="{p2_y + p2_h - bh_h:.1f}" width="28" height="{bh_h:.1f}" fill="#e11d48" rx="2"/>')
        svg.append(f'<text x="{gx+46}" y="{p2_y + p2_h - bh_h - 5:.1f}" text-anchor="middle" font-size="10.5" font-weight="600" fill="#be123c">{h_gy[i]:.3f}</text>')
        lbl_parts = layers[i].split("\n")
        svg.append(f'<text x="{gx+30}" y="{p2_y + p2_h + 20}" text-anchor="middle" font-size="11" fill="#1e293b">{lbl_parts[0]}</text>')
        if len(lbl_parts) > 1:
            svg.append(f'<text x="{gx+30}" y="{p2_y + p2_h + 34}" text-anchor="middle" font-size="10.5" fill="#475569">{lbl_parts[1]}</text>')

    # Legend at bottom
    svg.append(f'<rect x="310" y="442" width="16" height="16" fill="#0284c7" rx="2"/>')
    svg.append(f'<text x="332" y="454" font-size="12.5" font-weight="600" fill="#1e293b">Cryptococcus neoformans (Forma Estándar)</text>')
    svg.append(f'<rect x="605" y="442" width="16" height="16" fill="#e11d48" rx="2"/>')
    svg.append(f'<text x="627" y="454" font-size="12.5" font-weight="600" fill="#1e293b">Célula Humana Epitelial de Referencia</text>')

    svg.append("</svg>")
    out_path.write_text("\n".join(svg), encoding="utf-8")


def main():
    base_dir = pathlib.Path("/Users/gaelgarciachavez/Tesis")
    out_dir = base_dir / "analysis"
    out_dir.mkdir(parents=True, exist_ok=True)

    crypto_kin = load_kinetics(base_dir / "CryptoStandard_Output_nt_ROS_Kinetics_GValues.csv")
    human_kin = load_kinetics(base_dir / "HumanEpithelial_Output_nt_ROS_Kinetics_GValues.csv")
    generate_ros_svg(crypto_kin, human_kin, out_dir / "ros_kinetics_comparison.svg")

    crypto_dos = load_dosimetry(base_dir / "CryptoStandard_Output_nt_LayerMicrodosimetry.csv")
    human_dos = load_dosimetry(base_dir / "HumanEpithelial_Output_nt_LayerMicrodosimetry.csv")
    generate_dosimetry_svg(crypto_dos, human_dos, out_dir / "layer_dosimetry_comparison.svg")

    print("Gráficas generadas exitosamente en:")
    print(f" - {out_dir / 'ros_kinetics_comparison.svg'}")
    print(f" - {out_dir / 'layer_dosimetry_comparison.svg'}")


if __name__ == "__main__":
    main()

