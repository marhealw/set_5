import argparse
import pandas as pd
import numpy as np
import math

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--summary", default="data/summary.csv")
    ap.add_argument("--B", type=int, required=True)
    ap.add_argument("--tail", type=int, default=5)
    args = ap.parse_args()

    df = pd.read_csv(args.summary)
    m = 2 ** args.B
    theo1 = 1.042 / math.sqrt(m)
    theo2 = 1.32 / math.sqrt(m)

    df = df.copy()
    df["rse_emp"] = df["sigma_estimate_Nt"] / df["mean_estimate_Nt"]
    tail = df.tail(args.tail)

    rse_mean = tail["rse_emp"].mean()
    rse_max = tail["rse_emp"].max()

    print(f"B={args.B} m={m}")
    print(f"theory_rse_1_042={theo1:.6f}")
    print(f"theory_rse_1_32={theo2:.6f}")
    print(f"empirical_rse_mean_last{args.tail}={rse_mean:.6f}")
    print(f"empirical_rse_max_last{args.tail}={rse_max:.6f}")

if __name__ == "__main__":
    main()
