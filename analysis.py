
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

def combine_dfs_with_t(dfs, time_column):
    # Combine dataframes with an additional column for T (number of threads)
    # and use the specified time column
    combined_df = pd.DataFrame()
    for key, df in dfs.items():
        df['T'] = int(key.split('_')[-1][1:])
        df['Time'] = df[time_column]  # Use the specified time column
        combined_df = pd.concat([combined_df, df])
    return combined_df

def find_best_t(df):
    # Find the best T (number of threads) based on the average time
    avg_times = df.groupby('T')['Time'].mean()
    return avg_times.idxmin(), avg_times.min()

def plot_boxplots(data_frames, output_folder,time_column,methods):
    # Define the color palette
    cmap = 'hls'

    # REF Boxplot
    ref_df = data_frames['results_REF']
    plt.figure(figsize=(10, 6))
    sns.boxplot(data=ref_df, palette=cmap)
    plt.title('Boxplot for REF Method')
    plt.ylabel(f'{time_column} (secs)')
    plt.savefig(os.path.join(output_folder, f'boxplot_ref__{time_column}_{len(methods)}.pdf'))

    for method in methods:
        dfs = {key: df for key, df in data_frames.items() if method in key}
        combined = combine_dfs_with_t(dfs,time_column)
        plt.figure(figsize=(10, 6))
        sns.boxplot(x="T", y=time_column, data=combined, hue="T", palette=cmap, legend=False)
        plt.title(f'{method} Method Boxplot by Number of Threads')
        plt.xlabel('Number of Threads (T)')
        plt.ylabel(f'{time_column} (secs)')
        plt.savefig(os.path.join(output_folder, f'boxplot_{method}__{time_column}_{len(methods)}.pdf'))

    # Best T Comparison Boxplot
    combined_best_df = pd.DataFrame()
    for method in methods:
        dfs = {key: df for key, df in data_frames.items() if method in key}
        combined = combine_dfs_with_t(dfs,time_column)
        best_t, _ = find_best_t(combined)
        best_df = combined[combined['T'] == best_t]
        combined_best_df = pd.concat([combined_best_df, best_df.assign(Method=method)])
    combined_best_df = pd.concat([combined_best_df, ref_df.assign(Method="REF")])

    plt.figure(figsize=(12, 8))
    sns.boxplot(x="Method", y=time_column, data=combined_best_df, palette=cmap)
    plt.title('Comparison of Best T for Each Method Including REF')
    plt.xlabel('Method')
    plt.ylabel(f'{time_column} (secs)')
    plt.savefig(os.path.join(output_folder, f'best_t_comparison__{time_column}_{len(methods)}.pdf'))

# Rest of your script remains the same
def plot_mean_time_vs_t(data_frames, output_folder,time_column,methods):
    plt.figure(figsize=(12, 8))

    # Plotting mean times for each method
    for method in methods:
        dfs = {key: df for key, df in data_frames.items() if method in key}
        combined = combine_dfs_with_t(dfs,time_column)
        means = combined.groupby('T')[time_column].mean()
        sns.lineplot(x=means.index, y=means.values, label=method)

    # Plotting REF as a constant line
    ref_mean = data_frames['results_REF'][time_column].mean()
    plt.axhline(y=ref_mean, color='gray', linestyle='--', label='REF')

    plt.title(f'Mean {time_column} vs. Number of Threads (T) for Each Method')
    plt.xlabel('Number of Threads (T)')
    plt.ylabel(f'Mean {time_column} (secs)')
    plt.legend()
    plt.savefig(os.path.join(output_folder, f'mean_time_vs_t__{time_column}_{len(methods)}.pdf'),dpi=300)

def main(folder_path):
    data_frames = read_data(folder_path)
    output_folder = os.path.join(folder_path, 'plots')

    if not os.path.exists(output_folder):
        os.makedirs(output_folder)


            # Individual Boxplots for Each Method
    methods = ['COARSE', 'FINE','FINE2']

    for methods in [['COARSE', 'FINE','FINE2'],['COARSE', 'FINE','FINE2','PYTHON']]:
        for time_col in ['Total Time', 'Computation Time']:
            plot_boxplots(data_frames, output_folder, time_col,methods)
            plot_mean_time_vs_t(data_frames, output_folder, time_col,methods)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <folder_path>")
    else:
        main(sys.argv[1])