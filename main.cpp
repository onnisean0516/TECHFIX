#include <iostream>
#include <vector>
#include <string>
#include <fstream>
// Requires downloading httplib.h (https://github.com/yhirose/cpp-httplib)
#include "httplib.h" 

using namespace std;

// --- OOP Design ---

class SolutionStep {
private:
    int stepNumber;
    string instruction;
public:
    SolutionStep(int num, string inst) : stepNumber(num), instruction(inst) {}
    string getInstruction() const { return instruction; }
};

class Problem {
private:
    string title;
    string category;
    string targetOS;
    vector<SolutionStep> steps;
public:
    Problem(string t, string cat, string os) : title(t), category(cat), targetOS(os) {}
    void addStep(const SolutionStep& step) { steps.push_back(step); }
    
    string getTitle() const { return title; }
    string getOS() const { return targetOS; }
    vector<SolutionStep> getSteps() const { return steps; }
};

class FileManager {
public:
    static void saveHistory(const string& problemTitle) {
        ofstream file("history.txt", ios::app); // Append mode
        if (file.is_open()) {
            file << "Solved: " << problemTitle << "\n";
            file.close();
        }
    }
};

// --- System Initialization & API ---

int main() {
    // 1. Initialize Mock Database (In reality, load from problems.txt)
    vector<Problem> database;
    
    Problem p1("App keeps crashing", "Software", "Android");
    p1.addStep(SolutionStep(1, "Clear app cache"));
    p1.addStep(SolutionStep(2, "Restart device"));
    p1.addStep(SolutionStep(3, "Update app"));
    database.push_back(p1);

    Problem p2("No Internet", "Network", "Windows");
    p2.addStep(SolutionStep(1, "Check WiFi switch"));
    p2.addStep(SolutionStep(2, "Restart router"));
    p2.addStep(SolutionStep(3, "Run network troubleshooter"));
    database.push_back(p2);

    // 2. Setup HTTP Server
    httplib::Server svr;

    // Handle Search Requests (GET)
    svr.Get("/api/problems", [&](const httplib::Request& req, httplib::Response& res) {
        // Enable CORS so the HTML file can talk to the server
        res.set_header("Access-Control-Allow-Origin", "*"); 

        string searchOS = req.has_param("os") ? req.get_param_value("os") : "";
        string searchQuery = req.has_param("q") ? req.get_param_value("q") : "";

        string jsonResponse = "{}"; // Default empty response

        for (const auto& prob : database) {
            // Basic filtering logic
            if (prob.getOS() == searchOS && prob.getTitle().find(searchQuery) != string::npos) {
                // Construct JSON manually (or use a library like nlohmann/json)
                jsonResponse = "{\"title\": \"" + prob.getTitle() + "\", \"steps\": [";
                for (size_t i = 0; i < prob.getSteps().size(); ++i) {
                    jsonResponse += "\"" + prob.getSteps()[i].getInstruction() + "\"";
                    if (i < prob.getSteps().size() - 1) jsonResponse += ", ";
                }
                jsonResponse += "]}";
                break; // Return the first match
            }
        }

        res.set_content(jsonResponse, "application/json");
    });

    // Handle History Saving (POST)
    svr.Post("/api/history", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        
        // In a real app, parse the JSON body. Here we just log a generic success for demonstration.
        FileManager::saveHistory("Problem marked as solved via web UI");
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // Handle CORS preflight requests
    svr.Options("/api/history", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    cout << "TechFix C++ Backend running on http://localhost:8080..." << endl;
    svr.listen("localhost", 8080);

    return 0;
}
