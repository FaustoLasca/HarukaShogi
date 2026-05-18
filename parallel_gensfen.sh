#!/bin/bash
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <num_processes> <root_dir> <start_index>"
    exit 1
fi

num_processes="$1"
root_dir="$2"
start_index="$3"

mkdir -p "$root_dir"

pids=()

cleanup() {
    echo "Terminating child processes..."
    kill "${pids[@]}" 2>/dev/null
    wait "${pids[@]}" 2>/dev/null
    exit 1
}

trap cleanup SIGINT SIGTERM SIGHUP EXIT

run_one() {
    local idx="$1"
    local dir="$root_dir/data_$idx"
    local log="$dir/gensfen.log"
    mkdir -p "$dir"

    stdbuf -oL -eL ./pre_nnue_gensfen "$dir" &> "$log"
    local rc=$?
    if [ "$rc" -ne 0 ]; then
        echo "[data_$idx] exited with code $rc. Last log lines:" >&2
        tail -n 20 "$log" >&2
    fi
}

for i in $(seq "$start_index" $((start_index + num_processes - 1))); do
    run_one "$i" &
    pids+=($!)
done

wait "${pids[@]}"
trap - EXIT
