import subprocess
import csv
import re

def run_command(command):
    result = subprocess.run(command, stdout=subprocess.PIPE, text=True)
    output = result.stdout.strip()

    # Use a regular expression to find the elapsed time in the output
    match = re.search(r'Elapsed time: ([\d.]+) seconds', output)
    if match:
        elapsed_time = match.group(1)  # Extract the elapsed time
    else:
        elapsed_time = "Error"  # Or handle error appropriately

    return elapsed_time

def run_experiment(mode, x_val, n_repetitions, result_file):
    with open(result_file, 'w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Time'])  # Header

        for _ in range(n_repetitions):
            command = ["./matmulseq_file.o", str(x_val), "matrices_large.dat", mode, "0"]#"time"
            real_time = run_command(command)
            writer.writerow([real_time])

# Number of repetitions for each experiment
N_REPETITIONS = 5

# Run experiments for REF mode
run_experiment("REF", 1, N_REPETITIONS, "results/results_REF.csv")

# Run experiments for COARSE mode with varying X
for x in range(1, 33):
    result_file = f"results/results_COARSE_X{x}.csv"
    run_experiment("COARSE", x, N_REPETITIONS, result_file)

print("Experiments completed.")
