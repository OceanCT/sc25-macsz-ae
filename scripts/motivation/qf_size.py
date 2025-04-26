import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv("../visualize/datap/pw_rel.csv")
df = df[df["Method"] == "orisz"]
df[]:
print(df)
dataset_map = {
    "SDRBENCH-CESM-ATM-26x1800x3600": "CESM-ATM",
    "SDRBENCH-exaalt-copper": "EXAALT",
    "HurricaneISABEL/100x500x500": "Hurricane",
    "HACC/280953867": "HACC",
}
datasets = df["Dataset"].unique()
for dataset in datasets:
    df1 = df[df["Dataset"] == dataset]
    for precision in [1e-2, 1e-3, 1e-4]:
        df2 = df1[df1["Precision"] == precision]
        # print(df2)
        cqfs = df2.groupby("Method")["CompressedQfsize"].mean()
        cfs = df2.groupby("Method")["CompressedSize"].mean()
        print(dataset, precision ,cqfs["orisz"], cfs["orisz"])
    

