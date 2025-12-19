import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

# Filtro base para visualização clara
base = df[
    (df["N"] == 1000000) &
    (df["K"] == 20) &
    (df["B"] == 256)
]

plt.figure()
for sched in base["SCHEDULE"].unique():
    sub = base[base["SCHEDULE"] == sched]
    plt.errorbar(
        sub["THREADS"],
        sub["T_NAIVE_MEAN"],
        yerr=sub["T_NAIVE_STD"],
        label=f"Naive - {sched}",
        marker="o"
    )
    plt.errorbar(
        sub["THREADS"],
        sub["T_ORG_MEAN"],
        yerr=sub["T_ORG_STD"],
        label=f"Organized - {sched}",
        marker="s"
    )

plt.xlabel("Threads")
plt.ylabel("Tempo (s)")
plt.title("Organização da Região Paralela")
plt.legend()
plt.grid(True)
plt.savefig("tempo_vs_threads.png", dpi=150)

# Speedup
plt.figure()
for sched in base["SCHEDULE"].unique():
    sub = base[base["SCHEDULE"] == sched]
    plt.plot(
        sub["THREADS"],
        sub["SPEEDUP_MEAN"],
        marker="o",
        label=sched
    )

plt.xlabel("Threads")
plt.ylabel("Speedup (Organizada)")
plt.title("Speedup vs Threads")
plt.legend()
plt.grid(True)
plt.savefig("speedup_vs_threads.png", dpi=150)

plt.show()
