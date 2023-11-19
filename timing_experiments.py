import subprocess
import csv
import re
import os

file_path = os.path.realpath(__file__)

# FILE
DATFILE = 'matrices_large.dat'
# Number of repetitions for each experiment
N_REPETITIONS = 20

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
            if mode in ['REF','COARSE','FINE','FINEHUNGRY']:
                command = ["./matmulseq_file", str(x_val), DATFILE, mode, "0"]
            elif mode in ['PYTHON']:
                command = ["python3", "matmulseq_file.py", os.path.join(os.path.dirname(file_path),DATFILE), "--n_jobs", str(x_val)]
            else:
                raise "BAD MODE"
            real_time = run_command(command)
            writer.writerow([real_time])



# Run experiments for REF mode
print('REF')
run_experiment("REF", 1, N_REPETITIONS, "results/results_REF.csv")

# Run experiments for COARSE mode with varying X
for x in range(1, 33):
    print('COARSE',x)
    result_file = f"results/results_COARSE_X{x}.csv"
    run_experiment("COARSE", x, N_REPETITIONS, result_file)

# Run experiments for FINE mode with varying X
for x in range(1, 33):
    print('FINE',x)
    result_file = f"results/results_FINE_X{x}.csv"
    run_experiment("FINE", x, N_REPETITIONS, result_file)

# Run experiments for FINE mode with varying X
for x in range(1, 33):
    print('FINEHUNGRY',x)
    result_file = f"results/results_FINEHUNGRY_X{x}.csv"
    run_experiment("FINEHUNGRY", x, N_REPETITIONS, result_file)

# Run experiments for PYTHON mode with varying X
for x in range(1, 33):
    print('PYTHON',x)
    result_file = f"results/results_PYTHON_X{x}.csv"
    run_experiment("PYTHON", x, N_REPETITIONS, result_file)

print("Experiments completed.")


# Verify

for MODE in ['COARSE','FINE','FINEHUNGRY','PYTHON']:
    command=['./verify', DATFILE,'REF', MODE]
    result = subprocess.run(command, stdout=subprocess.PIPE, text=True)
    print("Checking ",MODE)
    print(result.stdout)