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
            if mode in ['REF','COARSE','FINE']:
                command = ["./matmulseq_file.o", str(x_val), "matrices_large.dat", mode, "0"]#"time"
            else: #mode=PYTHON
                command = ["/usr/bin/python3", "/home/user/code/parallelMatrixMultiplication/matmulseq_file.py", "/home/user/code/parallelMatrixMultiplication/matrices_dev.dat", "--n_jobs", str(x_val), "--verbose" if mode == "verbose" else ""]

            real_time = run_command(command)
            writer.writerow([real_time])

# Number of repetitions for each experiment
N_REPETITIONS = 1

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


# Run experiments for PYTHON mode with varying X
for x in range(1, 33):
    print('PYTHON',x)
    result_file = f"results/results_PYTHON_X{x}.csv"
    run_experiment("PYTHON", x, N_REPETITIONS, result_file)

print("Experiments completed.")
