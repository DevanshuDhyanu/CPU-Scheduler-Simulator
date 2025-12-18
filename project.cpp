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
};

struct Gantt {
    int pid, start, end;
};

// -------------------- Utility --------------------
void printLine() {
    cout << string(70, '-') << "\n";
}

void printResult(vector<Process> &p, vector<Gantt> &gantt) {
    double totalWT = 0, totalTAT = 0;

    cout << "\nGantt Chart:\n";
    for (auto &g : gantt)
        cout << "| P" << g.pid << " " << g.start << "-" << g.end << " ";
    cout << "|\n";

    printLine();
    cout << left << setw(6) << "PID"
         << setw(10) << "AT"
         << setw(10) << "BT"
         << setw(12) << "CT"
         << setw(12) << "WT"
         << setw(12) << "TAT" << "\n";
    printLine();

    for (auto &x : p) {
        cout << left << setw(6) << x.pid
             << setw(10) << x.arrival
             << setw(10) << x.burst
             << setw(12) << x.completion
             << setw(12) << x.waiting
             << setw(12) << x.turnaround << "\n";

        totalWT += x.waiting;
        totalTAT += x.turnaround;
    }

    printLine();
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time   : " << totalWT / p.size() << "\n";
    cout << "Average Turnaround Time: " << totalTAT / p.size() << "\n";
}

// -------------------- FCFS --------------------
void FCFS(vector<Process> p) {
    sort(p.begin(), p.end(), [](auto &a, auto &b) {
        return a.arrival < b.arrival;
    });

    vector<Gantt> gantt;
    int time = 0;

    for (auto &x : p) {
        if (time < x.arrival) time = x.arrival;

        gantt.push_back({x.pid, time, time + x.burst});
        time += x.burst;

        x.completion = time;
        x.turnaround = x.completion - x.arrival;
        x.waiting = x.turnaround - x.burst;
    }

    printResult(p, gantt);
}

// -------------------- SJF Non-Preemptive --------------------
void SJF(vector<Process> p) {
    int n = p.size(), time = 0, done = 0;
    vector<bool> finished(n, false);
    vector<Gantt> gantt;

    while (done < n) {
        int idx = -1, minBT = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (!finished[i] && p[i].arrival <= time && p[i].burst < minBT) {
                minBT = p[i].burst;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        gantt.push_back({p[idx].pid, time, time + p[idx].burst});
        time += p[idx].burst;

        p[idx].completion = time;
        p[idx].turnaround = time - p[idx].arrival;
        p[idx].waiting = p[idx].turnaround - p[idx].burst;

        finished[idx] = true;
        done++;
    }

    printResult(p, gantt);
}

// -------------------- SRTF --------------------
void SRTF(vector<Process> p) {
    int n = p.size(), time = 0, done = 0;
    vector<Gantt> gantt;

    while (done < n) {
        int idx = -1, minRem = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && p[i].remaining > 0 &&
                p[i].remaining < minRem) {
                minRem = p[i].remaining;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        if (gantt.empty() || gantt.back().pid != p[idx].pid)
            gantt.push_back({p[idx].pid, time, time + 1});
        else
            gantt.back().end++;

        p[idx].remaining--;
        time++;

        if (p[idx].remaining == 0) {
            done++;
            p[idx].completion = time;
            p[idx].turnaround = time - p[idx].arrival;
            p[idx].waiting = p[idx].turnaround - p[idx].burst;
        }
    }

    printResult(p, gantt);
}

// -------------------- Round Robin --------------------
void RoundRobin(vector<Process> p, int quantum) {
    int n = p.size(), time = 0, done = 0;
    queue<int> q;
    vector<bool> visited(n, false);
    vector<Gantt> gantt;

    while (done < n) {
        for (int i = 0; i < n; i++) {
            if (!visited[i] && p[i].arrival <= time) {
                q.push(i);
                visited[i] = true;
            }
        }

        if (q.empty()) {
            time++;
            continue;
        }

        int idx = q.front();
        q.pop();

        int exec = min(quantum, p[idx].remaining);
        gantt.push_back({p[idx].pid, time, time + exec});

        time += exec;
        p[idx].remaining -= exec;

        if (p[idx].remaining > 0)
            q.push(idx);
        else {
            done++;
            p[idx].completion = time;
            p[idx].turnaround = time - p[idx].arrival;
            p[idx].waiting = p[idx].turnaround - p[idx].burst;
        }
    }

    printResult(p, gantt);
}

// -------------------- Priority --------------------
void Priority(vector<Process> p, bool preemptive) {
    int n = p.size(), time = 0, done = 0;
    vector<Gantt> gantt;

    while (done < n) {
        int idx = -1, best = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival <= time && p[i].remaining > 0 &&
                p[i].priority < best) {
                best = p[i].priority;
                idx = i;
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        if (preemptive) {
            if (gantt.empty() || gantt.back().pid != p[idx].pid)
                gantt.push_back({p[idx].pid, time, time + 1});
            else
                gantt.back().end++;

            p[idx].remaining--;
            time++;

            if (p[idx].remaining == 0) {
                done++;
                p[idx].completion = time;
                p[idx].turnaround = time - p[idx].arrival;
                p[idx].waiting = p[idx].turnaround - p[idx].burst;
            }
        } else {
            gantt.push_back({p[idx].pid, time, time + p[idx].burst});
            time += p[idx].burst;

            p[idx].remaining = 0;
            done++;
            p[idx].completion = time;
            p[idx].turnaround = time - p[idx].arrival;
            p[idx].waiting = p[idx].turnaround - p[idx].burst;
        }
    }

    printResult(p, gantt);
}

// -------------------- MAIN --------------------
int main() {
    int n;
    cout << "Enter number of processes: ";
    cin >> n;

    vector<Process> p(n);
    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        cout << "\nProcess P" << i + 1 << ":\n";
        cout << "Arrival Time: ";
        cin >> p[i].arrival;
        cout << "Burst Time: ";
        cin >> p[i].burst;
        cout << "Priority: ";
        cin >> p[i].priority;

        p[i].remaining = p[i].burst;
    }

    int choice, q;
    do {
        cout << "\n1.FCFS  2.SJF  3.SRTF  4.RoundRobin  5.Priority  6.Priority(P)  0.Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: FCFS(p); break;
            case 2: SJF(p); break;
            case 3: SRTF(p); break;
            case 4:
                cout << "Enter Time Quantum: ";
                cin >> q;
                RoundRobin(p, q);
                break;
            case 5: Priority(p, false); break;
            case 6: Priority(p, true); break;
        }
    } while (choice != 0);

    return 0;
}
