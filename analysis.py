
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os
import sys

def read_data(folder_path):
    # Construct file paths and read CSV files into Pandas DataFrames
    files = [f for f in os.listdir(folder_path) if f.endswith('.csv')]
    dfs = {}
    for file in files:
        path = os.path.join(folder_path, file)
        dfs[file.split('.')[0]] = pd.read_csv(path)
    return dfs

def combine_dfs_with_t(dfs):
    # Combine dataframes with an additional column for T (number of threads)
    combined_df = pd.DataFrame()
    for key, df in dfs.items():
        df['T'] = int(key.split('_')[-1][1:])
        combined_df = pd.concat([combined_df, df])
    return combined_df

def find_best_t(df):
    # Find the best T (number of threads) based on the average time
    avg_times = df.groupby('T')['Time'].mean()
    return avg_times.idxmin(), avg_times.min()

def plot_boxplots(data_frames, output_folder):
    # Define the color palette
    cmap = 'hls'

    # REF Boxplot
    ref_df = data_frames['results_REF']
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=ref_df, palette=cmap)
    plt.title('Boxplot for REF Method')
    plt.ylabel('Execution Time (secs)')
    plt.savefig(os.path.join(output_folder, 'boxplot_ref.png'))

    # Individual Boxplots for Each Method
    methods = ['COARSE', 'FINE', 'FINEHUNGRY', 'PYTHON']
    for method in methods:
        dfs = {key: df for key, df in data_frames.items() if method in key}
        combined = combine_dfs_with_t(dfs)
        plt.figure(figsize=(10, 6))
        sns.boxplot(x="T", y="Time", data=combined, hue="T", palette=cmap, legend=False)
        plt.title(f'{method} Method Boxplot by Number of Threads')
        plt.xlabel('Number of Threads (T)')
        plt.ylabel('Execution Time (secs)')
        plt.savefig(os.path.join(output_folder, f'boxplot_{method}.png'))

    # Best T Comparison Boxplot
    combined_best_df = pd.DataFrame()
    for method in methods:
        dfs = {key: df for key, df in data_frames.items() if method in key}
        combined = combine_dfs_with_t(dfs)
        best_t, _ = find_best_t(combined)
        best_df = combined[combined['T'] == best_t]
        combined_best_df = pd.concat([combined_best_df, best_df.assign(Method=method)])
    combined_best_df = pd.concat([combined_best_df, ref_df.assign(Method="REF")])

    plt.figure(figsize=(12, 8))
    sns.boxplot(x="Method", y="Time", data=combined_best_df, palette=cmap)
    plt.title('Comparison of Best T for Each Method Including REF')
    plt.xlabel('Method')
    plt.ylabel('Execution Time (secs)')
    plt.savefig(os.path.join(output_folder, 'best_t_comparison.png'))

# Rest of your script remains the same
def plot_mean_time_vs_t(data_frames, output_folder):
    plt.figure(figsize=(12, 8))
    methods = ['COARSE', 'FINE', 'FINEHUNGRY', 'PYTHON']

    # Plotting mean times for each method
    for method in methods:
        dfs = {key: df for key, df in data_frames.items() if method in key}
        combined = combine_dfs_with_t(dfs)
        means = combined.groupby('T')['Time'].mean()
        sns.lineplot(x=means.index, y=means.values, label=method)

    # Plotting REF as a constant line
    ref_mean = data_frames['results_REF']['Time'].mean()
    plt.axhline(y=ref_mean, color='gray', linestyle='--', label='REF')

    plt.title('Mean Time vs. Number of Threads (T) for Each Method')
    plt.xlabel('Number of Threads (T)')
    plt.ylabel('Mean Execution Time (secs)')
    plt.legend()
    plt.savefig(os.path.join(output_folder, 'mean_time_vs_t.png'),dpi=300)

def main(folder_path):
    data_frames = read_data(folder_path)
    output_folder = os.path.join(folder_path, 'plots')
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    plot_boxplots(data_frames, output_folder)
    plot_mean_time_vs_t(data_frames, output_folder)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <folder_path>")
    else:
        main(sys.argv[1])

