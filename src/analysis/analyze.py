import argparse
import pandas as pd
import numpy as np

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--run", default="data/runs/run_1.csv")
    ap.add_argument("--summary", default="data/summary.csv")
    ap.add_argument("--out1", default="report/figures/graph1.png")
    ap.add_argument("--out2", default="report/figures/graph2.png")
    args = ap.parse_args()

    import matplotlib.pyplot as plt
    from pathlib import Path

    Path("report/figures").mkdir(parents=True, exist_ok=True)

    run = pd.read_csv(args.run)

    plt.figure()
    plt.plot(run["processed_fraction"], run["true_F0"], label="F0 (true)")
    plt.plot(run["processed_fraction"], run["estimate_Nt"], label="Nt (HLL)")
    plt.xlabel("processed_fraction")
    plt.ylabel("unique_count")
    plt.legend()
    plt.tight_layout()
    plt.savefig(args.out1, dpi=200)
    plt.close()

    summ = pd.read_csv(args.summary)
    x = summ["processed_fraction"].to_numpy()
    m = summ["mean_estimate_Nt"].to_numpy()
    s = summ["sigma_estimate_Nt"].to_numpy()

    plt.figure()
    plt.plot(x, m, label="E(Nt)")
    plt.fill_between(x, m - s, m + s, alpha=0.25, label="± sigma")
    plt.xlabel("processed_fraction")
    plt.ylabel("estimate_stats")
    plt.legend()
    plt.tight_layout()
    plt.savefig(args.out2, dpi=200)
    plt.close()

if __name__ == "__main__":
    main()
