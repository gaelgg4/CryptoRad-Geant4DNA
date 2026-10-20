#!/usr/bin/env python3
"""
Script de post-procesamiento para analizar y comparar los archivos CSV generados por
G4AnalysisManager en la simulación Geant4-DNA:
  1. Dosis depositada (Gy) y energía (eV) por capa celular:
     - C. neoformans (Forma Estándar): Cápsula (GXM), Pared (Quitosano/Melanina), Citoplasma, Núcleo (1C)
     - Célula Humana Epitelial: Glucocáliz, Complejo Membrana-Corteza, Citoplasma, Núcleo (2C)
  2. Evolución temporal (1 ps a 1 us) de los rendimientos químicos (G-values: moléculas / 100 eV)
     de ·OH, e-aq y H2O2, incluyendo el efecto de scavenging por melanina y GXM.
"""

import csv
import math
import pathlib
import sys
from collections import defaultdict


def read_g4_csv(filepath: pathlib.Path):
    """Lee un archivo CSV de G4AnalysisManager ignorando las cabeceras '#class'/'#column'."""
    columns = []
    rows = []
    if not filepath.exists():
        print(f"[Aviso] No se encontró el archivo: {filepath}")
        return columns, rows

    with filepath.open("r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()
            if not stripped:
                continue
            if stripped.startswith("#column"):
                parts = stripped.split()
                if len(parts) >= 3:
                    columns.append(parts[2])
            elif stripped.startswith("#"):
                continue
            else:
                reader = csv.reader([stripped])
                for parsed in reader:
                    rows.append(parsed)
    return columns, rows


def summarize_dosimetry(csv_path: pathlib.Path, label: str):
    _, rows = read_g4_csv(csv_path)
    if not rows:
        return

    n_events = len(rows)
    sums_gy = [0.0, 0.0, 0.0, 0.0]  # Outer, Wall/Cortex, Cyto, Nucleus
    sums_ev = [0.0, 0.0, 0.0, 0.0]

    for r in rows:
        for i in range(4):
            sums_ev[i] += float(r[2 + i])
            sums_gy[i] += float(r[6 + i])

    print(f"\n=== DOSIMETRÍA POR CAPA: {label} ({n_events} eventos) ===")
    layer_names = [
        "1. Capa Externa (Cápsula GXM / Glucocáliz)",
        "2. Pared Melaninizada / Membrana-Corteza  ",
        "3. Citoplasma                             ",
        "4. Núcleo (1C Haploide vs 2C Diploide)    ",
    ]
    for idx, name in enumerate(layer_names):
        print(
            f"  {name}: Edep total = {sums_ev[idx]/1e3:8.3f} keV | "
            f"Dosis total = {sums_gy[idx]:8.4f} Gy | "
            f"Dosis media/evento = {sums_gy[idx]/n_events:8.5f} Gy"
        )


def summarize_ros_kinetics(csv_path: pathlib.Path, label: str):
    _, rows = read_g4_csv(csv_path)
    if not rows:
        return

    by_time = defaultdict(lambda: [0.0, 0.0, 0.0, 0.0, 0])
    for r in rows:
        t_ps = float(r[2])
        g_oh = float(r[7])
        g_eaq = float(r[8])
        g_h2o2 = float(r[9])
        g_oh_eff = float(r[10])
        acc = by_time[t_ps]
        acc[0] += g_oh
        acc[1] += g_eaq
        acc[2] += g_h2o2
        acc[3] += g_oh_eff
        acc[4] += 1

    print(f"\n=== CINÉTICA QUÍMICA DE ROS (1 ps a 1 us): {label} ===")
    print(f"{'Tiempo (ps)':>12} | {'G(·OH)':>10} | {'G(·OH)_eff':>11} | {'G(e-_aq)':>10} | {'G(H2O2)':>10}")
    print("-" * 65)
    for t_ps in sorted(by_time.keys()):
        acc = by_time[t_ps]
        n = max(acc[4], 1)
        print(
            f"{t_ps:12.1f} | {acc[0]/n:10.4f} | {acc[3]/n:11.4f} | "
            f"{acc[1]/n:10.4f} | {acc[2]/n:10.4f}"
        )


def main():
    base_dir = pathlib.Path(".")
    if len(sys.argv) > 1:
        base_dir = pathlib.Path(sys.argv[1])

    summarize_dosimetry(
        base_dir / "CryptoStandard_Output_nt_LayerMicrodosimetry.csv",
        "Cryptococcus neoformans (Forma Estándar)",
    )
    summarize_ros_kinetics(
        base_dir / "CryptoStandard_Output_nt_ROS_Kinetics_GValues.csv",
        "Cryptococcus neoformans (Forma Estándar)",
    )

    summarize_dosimetry(
        base_dir / "HumanEpithelial_Output_nt_LayerMicrodosimetry.csv",
        "Célula Humana Epitelial de Referencia",
    )
    summarize_ros_kinetics(
        base_dir / "HumanEpithelial_Output_nt_ROS_Kinetics_GValues.csv",
        "Célula Humana Epitelial de Referencia",
    )


if __name__ == "__main__":
    main()

