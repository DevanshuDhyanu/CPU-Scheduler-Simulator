// cpu_scheduler.cpp
// Intelligent CPU Scheduler Simulator (Menu-Based)
// C++17 (compile: g++ -std=c++17 cpu_scheduler.cpp -o scheduler)

#include <bits/stdc++.h>
using namespace std;

struct Process {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int priority;
    int completion;
    int waiting;
    int turnaround;
    Process(int id=0,int a=0,int b=0,int p=0){
        pid = id; arrival = a; burst = b; remaining = b;
        priority = p; completion = 0; waiting = 0; turnaround = 0;
    }
};

struct GanttEvent {
    int pid;
    int start;
    int end;
    GanttEvent(int p,int s,int e): pid(p), start(s), end(e) {}
};

// Utility: print a separator
void print_sep() {
    cout << string(70, '-') << "\n";
}

// Print results table and averages
void print_results(const vector<Process>& procs, const vector<GanttEvent>& gantt) {
    int n = procs.size();
    double total_wait = 0;
    double total_tat = 0;
    cout << "\nGantt Chart (pid:start-end):\n";
    for (auto &g : gantt) {
        cout << " | P" << g.pid << ":" << g.start << "-" << g.end;
    }
    cout << " |\n";
    print_sep();
    cout << left << setw(6) << "PID" 
         << setw(10) << "Arrival"
         << setw(8) << "Burst"
         << setw(12) << "Completion"
         << setw(10) << "Waiting"
         << setw(12) << "Turnaround" << "\n";
    print_sep();
    for (const auto &p : procs) {
        cout << left << setw(6) << ("P"+to_string(p.pid))
             << setw(10) << p.arrival
             << setw(8) << p.burst
             << setw(12) << p.completion
             << setw(10) << p.waiting
             << setw(12) << p.turnaround << "\n";
        total_wait += p.waiting;
        total_tat += p.turnaround;
    }
    print_sep();
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time   : " << (total_wait / n) << "\n";
    cout << "Average Turnaround Time: " << (total_tat / n) << "\n";
    // CPU Utilization (approx): (total busy time / last completion time) * 100
    int last = 0;
    for (auto &g : gantt) if (g.end > last) last = g.end;
    int busy = 0;
    for (auto &g : gantt) busy += (g.end - g.start);
    double util = (last == 0 ? 0.0 : (100.0 * busy / last));
    cout << "CPU Utilization (approx): " << util << " %\n";
    print_sep();
}

// Helper: deep copy processes
vector<Process> copy_processes(const vector<Process>& src) {
    vector<Process> dest;
    for (auto &p : src) dest.push_back(p);
    return dest;
}

//first come first serve
void scheduler_FCFS(vector<Process> procs) {
    sort(procs.begin(), procs.end(), [](const Process &a, const Process &b){
        if (a.arrival != b.arrival) return a.arrival < b.arrival;
        return a.pid < b.pid;
    });
    vector<GanttEvent> gantt;
    int time = 0;
    for (auto &p: procs) p.remaining = p.burst;
    for (auto &p: procs) {
        if (time < p.arrival) {
            time = p.arrival; // CPU idle jump
        }
        int start = time;
        time += p.burst;
        int end = time;
        p.completion = end;
        p.turnaround = p.completion - p.arrival;
        p.waiting = p.turnaround - p.burst;
        gantt.emplace_back(p.pid, start, end);
    }
    print_results(procs, gantt);
}


  // SJF Non-preemptive (Shortest Job First)
   
void scheduler_SJF_nonpreemptive(vector<Process> procs) {
    int n = procs.size();
    for (auto &p : procs) p.remaining = p.burst;
    vector<GanttEvent> gantt;
    int completed = 0;
    int time = 0;
    vector<bool> done(n, false);

    while (completed < n) {
        // find available process with smallest burst among arrived and not done
        int idx = -1;
        int minBurst = INT_MAX;
        for (int i = 0; i < n; ++i) {
            if (!done[i] && procs[i].arrival <= time) {
                if (procs[i].burst < minBurst) {
                    minBurst = procs[i].burst;
                    idx = i;
                } else if (procs[i].burst == minBurst) {
                    if (procs[i].arrival < procs[idx].arrival) idx = i;
                }
            }
        }
        if (idx == -1) {
            // no process ready; jump time
            int nextArrival = INT_MAX;
            for (int i=0;i<n;i++) if(!done[i]) nextArrival = min(nextArrival, procs[i].arrival);
            time = max(time, nextArrival);
            continue;
        }
        int start = time;
        time += procs[idx].burst;
        int end = time;
        procs[idx].completion = end;
        procs[idx].turnaround = procs[idx].completion - procs[idx].arrival;
        procs[idx].waiting = procs[idx].turnaround - procs[idx].burst;
        done[idx] = true;
        completed++;
        gantt.emplace_back(procs[idx].pid, start, end);
    }
    print_results(procs, gantt);
}

/* ----------------------------------------------------
   SRTF (Shortest Remaining Time First) - Preemptive SJF
   ---------------------------------------------------- */
void scheduler_SRTF(vector<Process> procs) {
    int n = procs.size();
    for (auto &p : procs) p.remaining = p.burst;
    vector<GanttEvent> gantt;
    int time = 0;
    int completed = 0;
    int last_pid = -1;
    while (completed < n) {
        // find process with smallest remaining >0 and arrival <= time
        int idx = -1; int minRem = INT_MAX;
        for (int i=0;i<n;i++){
            if (procs[i].remaining > 0 && procs[i].arrival <= time) {
                if (procs[i].remaining < minRem) {
                    minRem = procs[i].remaining; idx = i;
                } else if (procs[i].remaining == minRem) {
                    // tie-breaker: earlier arrival
                    if (procs[i].arrival < procs[idx].arrival) idx = i;
                }
            }
        }
        if (idx == -1) {
            // idle: advance to next arrival
            int nextArrival = INT_MAX;
            for (int i=0;i<n;i++) if (procs[i].remaining > 0) nextArrival = min(nextArrival, procs[i].arrival);
            time = max(time+1, nextArrival);
            continue;
        }
        // execute for 1 unit (simulation step)
        int start = time;
        // if continuing same pid, extend last event; otherwise push new event
        if (!gantt.empty() && gantt.back().pid == procs[idx].pid && gantt.back().end == start) {
            // continue
        } else {
            // new segment
            gantt.emplace_back(procs[idx].pid, start, start+1);
        }
        // ensure the last event end increments
        if (gantt.back().end == start) gantt.back().end = start + 1;
        else gantt.back().end = start + 1;
        procs[idx].remaining -= 1;
        time += 1;
        if (procs[idx].remaining == 0) {
            procs[idx].completion = time;
            procs[idx].turnaround = procs[idx].completion - procs[idx].arrival;
            procs[idx].waiting = procs[idx].turnaround - procs[idx].burst;
            completed++;
        }
    }
    // Merge adjacent events for same pid (already contiguous by construction)
    print_results(procs, gantt);
}

/* -------------------------
   Round Robin Scheduling
   ------------------------- */
void scheduler_RR(vector<Process> procs, int quantum) {
    int n = procs.size();
    for (auto &p : procs) p.remaining = p.burst;
    vector<GanttEvent> gantt;
    queue<int> q; // indices
    int time = 0;
    vector<bool> inQueue(n, false);
    // push processes that arrive at time 0
    int added = 0;
    while (added < n && procs[added].arrival == 0) {
        q.push(added); inQueue[added] = true; added++;
    }
    // but arrival order might not be sorted; so sort by arrival first
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a,int b){
        if (procs[a].arrival != procs[b].arrival) return procs[a].arrival < procs[b].arrival;
        return procs[a].pid < procs[b].pid;
    });
    // rebuild procs in arrival order for simpler logic:
    vector<Process> pcopy;
    for (int idx : order) pcopy.push_back(procs[idx]);
    // Reassign pcopy indices start from 0..n-1 with same pid values
    procs = pcopy;
    n = procs.size();
    for (auto &p : procs) p.remaining = p.burst;
    time = 0;
    int completed = 0;
    int i = 0;
    // enqueue first arriving ones
    while (i < n && procs[i].arrival <= time) { q.push(i); inQueue[i]=true; i++; }
    if (q.empty() && i < n) { time = procs[i].arrival; q.push(i); inQueue[i]=true; i++; }
    while (completed < n) {
        if (q.empty()) {
            if (i < n) { time = max(time, procs[i].arrival); q.push(i); inQueue[i]=true; i++; }
            continue;
        }
        int idx = q.front(); q.pop(); inQueue[idx] = false;
        int start = time;
        int exec = min(quantum, procs[idx].remaining);
        // record event
        gantt.emplace_back(procs[idx].pid, start, start + exec);
        procs[idx].remaining -= exec;
        time += exec;
        // enqueue newly arrived processes during this time slice
        while (i < n && procs[i].arrival <= time) {
            if (!inQueue[i]) { q.push(i); inQueue[i] = true; }
            i++;
        }
        if (procs[idx].remaining > 0) {
            q.push(idx);
            inQueue[idx] = true;
        } else {
            procs[idx].completion = time;
            procs[idx].turnaround = procs[idx].completion - procs[idx].arrival;
            procs[idx].waiting = procs[idx].turnaround - procs[idx].burst;
            completed++;
        }
    }
    print_results(procs, gantt);
}

/* ------------------------------------------------
   Priority Scheduling (non-preemptive & preemptive)
   Note: smaller priority value = higher priority
   ------------------------------------------------ */
void scheduler_Priority(vector<Process> procs, bool preemptive) {
    int n = procs.size();
    for (auto &p : procs) p.remaining = p.burst;
    vector<GanttEvent> gantt;
    int time = 0;
    int completed = 0;
    vector<bool> done(n,false);

    if (!preemptive) {
        while (completed < n) {
            int idx = -1;
            int bestPr = INT_MAX;
            for (int i=0;i<n;i++){
                if (!done[i] && procs[i].arrival <= time) {
                    if (procs[i].priority < bestPr) { bestPr = procs[i].priority; idx = i; }
                    else if (procs[i].priority == bestPr) {
                        if (procs[i].arrival < procs[idx].arrival) idx = i;
                    }
                }
            }
            if (idx == -1) {
                int nextArrival = INT_MAX;
                for (int i=0;i<n;i++) if(!done[i]) nextArrival = min(nextArrival, procs[i].arrival);
                time = max(time, nextArrival);
                continue;
            }
            int start = time;
            time += procs[idx].burst;
            int end = time;
            procs[idx].completion = end;
            procs[idx].turnaround = procs[idx].completion - procs[idx].arrival;
            procs[idx].waiting = procs[idx].turnaround - procs[idx].burst;
            done[idx] = true;
            completed++;
            gantt.emplace_back(procs[idx].pid, start, end);
        }
    } else {
        // preemptive
        while (completed < n) {
            int idx = -1;
            int bestPr = INT_MAX;
            for (int i=0;i<n;i++){
                if (procs[i].remaining > 0 && procs[i].arrival <= time) {
                    if (procs[i].priority < bestPr) { bestPr = procs[i].priority; idx = i; }
                }
            }
            if (idx == -1) {
                int nextArrival = INT_MAX;
                for (int i=0;i<n;i++) if (procs[i].remaining > 0) nextArrival = min(nextArrival, procs[i].arrival);
                time = max(time+1, nextArrival);
                continue;
            }
            // execute 1 time unit
            int start = time;
            if (!gantt.empty() && gantt.back().pid == procs[idx].pid && gantt.back().end == start) {
                // extend
            } else {
                gantt.emplace_back(procs[idx].pid, start, start+1);
            }
            gantt.back().end = start+1;
            procs[idx].remaining -= 1;
            time += 1;
            if (procs[idx].remaining == 0) {
                procs[idx].completion = time;
                procs[idx].turnaround = procs[idx].completion - procs[idx].arrival;
                procs[idx].waiting = procs[idx].turnaround - procs[idx].burst;
                completed++;
            }
        }
    }
    print_results(procs, gantt);
}

/* -------------------------
   Helper: Read processes
   ------------------------- */
vector<Process> read_processes_interactive() {
    int n;
    cout << "Enter number of processes: ";
    while (!(cin >> n) || n <= 0) {
        cout << "Invalid. Enter a positive integer: ";
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    vector<Process> procs;
    for (int i=0;i<n;i++){
        int at, bt, pr;
        cout << "\nProcess P" << (i+1) << ":\n";
        cout << "Arrival Time (>=0): ";
        while (!(cin >> at) || at < 0) {
            cout << "Invalid. Enter non-negative integer: ";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cout << "Burst Time (>0): ";
        while (!(cin >> bt) || bt <= 0) {
            cout << "Invalid. Enter positive integer: ";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cout << "Priority (integer, lower = higher priority): ";
        while (!(cin >> pr)) {
            cout << "Invalid. Enter integer: ";
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        procs.emplace_back(i+1, at, bt, pr);
    }
    return procs;
}

/* -------------------------
   Main menu & driver
   ------------------------- */
int main() {
    cout << "==== Intelligent CPU Scheduler Simulator (C++ - Menu Based) ====\n";
    vector<Process> procs;
    bool has_input = false;
    while (true) {
        cout << "\nMain Menu:\n";
        cout << "1. Enter processes (interactive)\n";
        cout << "2. Load sample processes (demo)\n";
        cout << "3. FCFS\n";
        cout << "4. SJF (Non-preemptive)\n";
        cout << "5. SRTF (Preemptive SJF)\n";
        cout << "6. Round Robin\n";
        cout << "7. Priority Scheduling (Non-preemptive)\n";
        cout << "8. Priority Scheduling (Preemptive)\n";
        cout << "9. Show current process list\n";
        cout << "0. Exit\n";
        cout << "Choose option: ";
        int opt; if (!(cin >> opt)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
        if (opt == 0) { cout << "Exiting. Good luck!\n"; break; }
        if (opt == 1) {
            procs = read_processes_interactive();
            has_input = true;
            cout << "\nProcesses recorded. Use a scheduling option to simulate.\n";
        } else if (opt == 2) {
            // sample data (demonstration)
            procs.clear();
            procs.emplace_back(1, 0, 8, 2);
            procs.emplace_back(2, 1, 4, 1);//sample data
            procs.emplace_back(3, 2, 9, 3);
            procs.emplace_back(4, 3, 5, 2);
            has_input = true;
            cout << "Loaded sample processes (4 processes).\n";
        } else if (!has_input) {
            cout << "No processes loaded. Choose option 1 or 2 first.\n";
        } else if (opt == 3) {
            cout << "\n== FCFS ==\n";
            scheduler_FCFS(copy_processes(procs));
        } else if (opt == 4) {
            cout << "\n== SJF (Non-preemptive) ==\n";
            scheduler_SJF_nonpreemptive(copy_processes(procs));
        } else if (opt == 5) {
            cout << "\n== SRTF (Preemptive SJF) ==\n";
            scheduler_SRTF(copy_processes(procs));
        } else if (opt == 6) {
            cout << "\n== Round Robin ==\n";
            int q; cout << "Enter quantum (>0): ";
            while (!(cin >> q) || q <= 0) { cout << "Invalid. Enter positive integer: "; cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
            scheduler_RR(copy_processes(procs), q);
        } else if (opt == 7) {
            cout << "\n== Priority (Non-preemptive) ==\n";
            scheduler_Priority(copy_processes(procs), false);
        } else if (opt == 8) {
            cout << "\n== Priority (Preemptive) ==\n";
            scheduler_Priority(copy_processes(procs), true);
        } else if (opt == 9) {
            cout << "\nCurrent Processes:\n";
            cout << left << setw(6) << "PID" << setw(10) << "Arrival" << setw(8) << "Burst" << setw(10) << "Priority" << "\n";
            print_sep();
            for (auto &p : procs) {
                cout << left << setw(6) << ("P"+to_string(p.pid)) << setw(10) << p.arrival << setw(8) << p.burst << setw(10) << p.priority << "\n";
            }
            print_sep();
        } else {
            cout << "Invalid option.\n";
        }
    }
    return 0;
}



