
#pragma once

#include <string>
#include <vector>
#include <set>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <future>

#include "json.hpp"
//#include "event_poller.hpp"

class MallobIpasir {

public:
    //static EventPoller* _event_poller;
    enum Interface {SOCKET, FILESYSTEM};
    enum FormulaTransfer {FILE, NAMED_PIPE};

private:
    std::string _tmp_dir;

    Interface _interface;
    FormulaTransfer _formula_transfer;
    std::string _api_directory;
    int _solver_id;
    bool _incremental = true;

    std::vector<int> _formula;
    std::vector<int> _assumptions;
    int _num_vars = 0;
    int _num_cls = 0;
    int _revision = 0;

    bool _presubmitted = false;
    int _fd_formula = -1;

    //int _fd_inotify = -1;
    //int _fd_inotify_watcher = -1;
    //std::vector<char> _inotify_buffer;
    //EventPoller::PollState _poll_state;

    int (*_terminate_callback) (void*) = nullptr;
    void* _terminate_data;

    std::vector<int> _model;
    std::set<int> _failed_assumptions;

    std::vector<std::thread> _branched_threads;

    std::mutex _branch_mutex;
    std::condition_variable _branch_cond_var;

    int _fd_socket;

    std::thread _json_reader;
    std::optional<nlohmann::json> _result_json;
    bool _json_read {false};

    bool _consecutive_zero {false};
    bool _back_is_zero_before_write {true};
public:
    MallobIpasir(Interface interface, bool incremental);
    
    std::string getSignature() const;

    void addLiteral(int lit) {
        if (lit == 0 && (_back_is_zero_before_write || (_formula.back() == 0))) {
            _consecutive_zero = true;
            return;
        }
        _formula.push_back(lit);
        _back_is_zero_before_write = false;
        if (_presubmitted && _formula.size() >= 512) {
            if (_formula.back() == 0) _back_is_zero_before_write = true;
            completeWrite(_fd_formula, (char*)_formula.data(), _formula.size()*sizeof(int));
            _formula.clear();
        }
        if (lit == 0) {
            _num_cls++;
        } else {
            _num_vars = std::max(_num_vars, std::abs(lit));
        }
    }
    void addAssumption(int lit) {
        _assumptions.push_back(lit);
        _num_vars = std::max(_num_vars, std::abs(lit));
    }

    int solve();

    void branchedSolve(void * data, int (*terminate)(void* data), void (*callbackAtFinish)(int result, void* solver, void* data));

    void destruct();

    int getValue(int lit) {
        return (lit < 0 ? -1 : 1) * _model[std::abs(lit)];
    }
    bool isAssumptionFailed(int lit) {
        return _failed_assumptions.count(lit);
    }
    void setTerminateCallback(void * data, int (*terminate)(void * data)) {
        _terminate_callback = terminate;
        _terminate_data = data;
    }

    void submitJob();

private:

    std::string getJobName(int revision);
    std::string getFormulaName();
    std::string getResultJsonPath();

    std::string drawRandomApiPath();

    void writeJson(nlohmann::json& json, const std::string& file);
    std::optional<nlohmann::json> readJson(const std::string& file);
    
    void setupConnection();
    void sendJson(nlohmann::json& json);
    std::optional<nlohmann::json> receiveJson();

    void interruptResultJsonRead();

    void writeFormula(const std::string& file);
    void pipeFormula(const std::string& pipe);
    
    void completeWrite(int fd, const char* data, int numBytes);
    void completeRead(int fd, char* data, int numBytes);

    std::vector<int> tokenizeDimacsLines(const std::vector<std::string>& lines);

    bool isCompressedModel(const std::string& str);
    std::vector<int> decompressModel(const std::string& compressedModel);
};
