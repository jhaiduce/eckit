/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/cmd/PsCmd.h"
#include "eckit/runtime/Monitor.h"
#include "eckit/log/Colour.h"

//-----------------------------------------------------------------------------

namespace eckit {

//-----------------------------------------------------------------------------

static PsCmd ps;

//-----------------------------------------------------------------------------

PsCmd::PsCmd() : CmdResource("ps") {
}

//-----------------------------------------------------------------------------

PsCmd::~PsCmd() {
}

//-----------------------------------------------------------------------------

void PsCmd::display(std::ostream& out, TaskInfo& info, long tasknb, const std::string& grep) const {
    char st = info.state();

    if (info.max() != info.min()) {
        out << Colour::green;
    }

    if (st == 'T' || st == 'V' || st == 'P' || st == 'X') {
        out << Colour::cyan;
    }

    if (st == 'Z' || st == 'L') {
        out << Colour::magenta;
    }

    if (std::string(info.status()).find("queued") != std::string::npos) {
        out << Colour::yellow;
    }

    if (grep.length()) {
        if (std::string(info.status()).find(grep) != std::string::npos) {
            out << Colour::red << Colour::bold;
        }
    }

    for (int i = 0; i < info.depth(); i++) out << "   ";

    // name column
    std::string app = info.name();
    out << app;

    int n = info.depth() * 3 + app.length();
    for (int i = 0; i < 16 - n; i++) out << ' ';

    // Idle column
    long age = ::time(0) - info.last();

    long s = age % 60;
    age /= 60;
    long m = age % 60;
    age /= 60;
    long h = age % 24;
    age /= 24;
    long d = age;

    if (d)
        out << std::setw(d > 1 ? 2 : 3) << d << "day" << (d > 1 ? "s" : "");
    else if (h || m)
        out << std::setw(3) << h << ":" << std::setw(2) << std::setfill('0') << m << std::setfill(' ');
    else
        out << std::setw(6) << s;

    // State flag
    out << ' ' << info.state();
    // Pid column
    out << std::setw(9) << info.pid();
    // Task column
    out << "   " << std::setfill('0') << std::setw(3) << tasknb << std::setfill(' ');
    // ID column
    out << std::setw(11) << info.taskID();
    // Info column
    out << std::setw(14) << info.message();
    // Request column
    out << "  " << info.status();

    // Progress indicator
    if (info.max() != info.min()) {
        double x = (info.val() - info.min()) * 100.0 / (info.max() - info.min());
        out << ' ' << x << '%';
    }

    out << std::endl;

    out << Colour::reset;
}

//-----------------------------------------------------------------------------

static void get(int n, std::vector<int>& v) {
    Monitor::TaskArray& info = Monitor::instance().tasks();
    if (n != -1) {
        get(info[n].parent(), v);
        v.push_back(n);
    }
}

//-----------------------------------------------------------------------------

static std::string get(int n) {

    Monitor::TaskArray& info = Monitor::instance().tasks();

    if (info[n].parent() != -1) return get(info[n].parent());

    return info[n].name();
}

//-----------------------------------------------------------------------------

static bool sortTasks(int n1, int n2) {

    std::string s1 = get(n1);
    std::string s2 = get(n2);

    if (s1 != s2) return s1 < s2;

    std::vector<int> v1;
    std::vector<int> v2;

    get(n1, v1);
    get(n2, v2);

    return v1 < v2;
}

//-----------------------------------------------------------------------------

static bool isChild(Monitor::TaskArray& info, const std::string& name, int task) {
    int parent = info[task].parent();
    if ((parent == -1) && (name == info[task].application())) return true;

    if (parent != -1) return isChild(info, name, parent);

    return false;
}

//-----------------------------------------------------------------------------

static bool isParent(Monitor::TaskArray& info, int taskid, int task) {
    int parenttask = info[task].parent();
    int parentid = info[taskid].parent();
    if (task == taskid) return true;

    return ((parenttask != -1) && isParent(info, taskid, parenttask)) ||
           ((parentid != -1) && isParent(info, parentid, task));
}

//-----------------------------------------------------------------------------

void PsCmd::execute(std::istream& in, std::ostream& out, CmdArg& args) {
    std::string grep;

    std::vector<int> t;
    Monitor::TaskArray& info = Monitor::instance().tasks();

    std::vector<std::string> tasks;
    std::vector<int> taskids;
    std::vector<int> pids;
    bool all = true;

    if (args.exists("grep")) {
        grep = std::string(args["grep"]);
    }

    for (size_t i = 1; i < args.size(); ++i)
        if (args.exists(i)) {
            all = false;
            if (args[i].isNumber()) {
                int id = args[i];
                if (info.size() >= static_cast<unsigned long>(id))
                    taskids.push_back(id);
                else
                    pids.push_back(id);
            } else
                tasks.push_back(args[i]);
        }

    for (size_t j = 0; j < info.size(); j++)
        if (info[j].busy(true) && info[j].show()) t.push_back(j);

    std::sort(t.begin(), t.end(), sortTasks);

    out << Colour::bold;
    out << "name              Idle        Pid   Task       ID   Info      Request" << std::endl;
    out << "---------------------------------------------------------------------" << std::endl;
    out << Colour::reset;

    if (all)
        for (size_t j = 0; j < t.size(); j++) display(out, info[t[j]], t[j], grep);
    else
        for (size_t j = 0; j < t.size(); j++) {
            for (Ordinal i = 0; i < tasks.size(); ++i)
                if (isChild(info, tasks[i], t[j])) {
                    display(out, info[t[j]], t[j], grep);
                    break;
                }
            for (Ordinal k = 0; k < taskids.size(); ++k)
                if (isParent(info, taskids[k], t[j])) {
                    display(out, info[t[j]], t[j], grep);
                    break;
                }
            for (Ordinal l = 0; l < pids.size(); ++l)
                if (pids[l] == info[t[j]].pid()) {
                    display(out, info[t[j]], t[j], grep);
                    break;
                }
        }
}

//-----------------------------------------------------------------------------

void PsCmd::help(std::ostream& out) const {
    out << "as the UNIX counterpart";
}

//-----------------------------------------------------------------------------

Arg PsCmd::usage(const std::string& cmd) const {
    return ~Arg("-grep", Arg::text) + Arg("<name> ...", Arg::text);
}

//-----------------------------------------------------------------------------

} // namespace eckit
