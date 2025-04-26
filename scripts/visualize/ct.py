import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
# Assuming df and dataset_map are defined elsewhere in your code
dataset_map = {
    "SDRBENCH-CESM-ATM-26x1800x3600": "CESM-ATM",
    "SDRBENCH-exaalt-copper": "EXAALT",
    "HurricaneISABEL/100x500x500": "Hurricane",
    "HACC/280953867": "HACC",
}
df = pd.read_csv('./datap/pw_rel.csv')
error_bounds = [0.01, 0.001, 0.0001]
colors = ['#4C78A8', '#F58518', '#E45756']  # Blue, Orange, Red
hatches = ['/', '\\', '|']  # Different hatch styles
desired_order = ['orisz', 'adt_fse', 'mac', 'lpaq',  'zfp']
display_labels = [m.replace('mac', 'mac-sz').replace('lpaq', 'lpaq-sz').replace('adt_fse', 'adt-fse-sz') for m in desired_order]
plt.rcParams.update({
    'font.size': 24,
    'font.weight': 'bold',
    'axes.labelweight': 'bold',
})

# Create a horizontal subplot, 1 row, 4 columns
fig, axes = plt.subplots(1, 4, figsize=(18, 5), sharey=True)

# Bar width and positions
bar_width = 0.25  # Width of each bar
index = np.arange(len(desired_order))  # Base positions for the methods

# Traverse all datasets and plot on corresponding subplots
for ax, dataset in zip(axes, list(dataset_map.values())[:4]):  # Limit to 4 datasets
    for i, (error_bound, color, hatch) in enumerate(zip(error_bounds, colors, hatches)):
        dataset_df = df[(df["Precision"] == error_bound) & (df["Dataset"] == dataset)]
        avg_compression_time = dataset_df.groupby("Method")["Compressiontime"].mean()        # Plot bars with offset positions
        avg_compression_time = avg_compression_time.reindex(desired_order)
        ax.bar(index + i * bar_width, 
               avg_compression_time, 
               bar_width, 
               color=color, 
               edgecolor='black', 
               alpha=0.8, 
               label=f'Error Bound {error_bound:.0e}', 
               hatch=hatch)
    
    # Set subplot labels and title
    print(dataset, error_bound, avg_compression_time)
    ax.set_title(dataset, fontsize=24, fontweight='bold')
    ax.set_xticks(index + bar_width * (len(error_bounds) - 1) / 2)  # Center the ticks
    ax.set_yticks([250, 500, 750])
    ax.set_xticklabels(display_labels, rotation=30, fontsize=24)
    ax.tick_params(axis='y', labelsize=24, width=5)
    ax.grid(True)
    # ax.grid(axis='y', linestyle='--', alpha=0.7)

# Set shared y-axis label
fig.text(0.01, 0.5, "Time", va='center', rotation='vertical', fontsize=24)

# Add legend (only on the last subplot to avoid repetition)
fig.legend(
    labels=[f'Error Bound {eb:.0e}' for eb in error_bounds],
    loc='upper center',
    bbox_to_anchor=(0.5, 1.05),
    ncol=len(error_bounds),
    fontsize=24
)

plt.tight_layout()

plt.subplots_adjust(left=0.08, top=0.82)  # Adjust layout for y-axis label
# Save and display
plt.savefig("./pics/ctime.pdf", bbox_inches='tight')
plt.show()
    
