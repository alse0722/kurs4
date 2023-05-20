#include <iostream>
#include <fstream>
#include <istream>
#include <vector>
#include <regex>
#include <string>
#include <iterator>

using namespace std;

string configFile = "nginx.conf";

int generateDeafaultConfigFile()
{
    ofstream config_file("nginx.conf");

    if (!config_file.is_open())
    {
        cerr << "Failed to open config file for writing." << endl;
        return -1;
    }

    config_file << "worker_processes auto;" << endl;
    config_file << "pid /var/run/nginx.pid;" << endl;
    config_file << endl;
    config_file << "events {" << endl;
    config_file << "    worker_connections 1024;" << endl;
    config_file << "}" << endl;
    config_file << endl;
    config_file << "http {" << endl;
    config_file << "    include       mime.types;" << endl;
    config_file << "    default_type  application/octet-stream;" << endl;
    config_file << "    sendfile        on;" << endl;
    config_file << "    keepalive_timeout  65;" << endl;
    config_file << endl;
    config_file << "    error_log /var/log/nginx/error.log;" << endl;
    config_file << "    access_log /var/log/nginx/access.log;" << endl;
    config_file << endl;
    config_file << "    server {" << endl;
    //config_file << "\n        #listen" << endl;
    config_file << "        listen       80;" << endl;
    //config_file << "\n        #allow" << endl;
    //config_file << "\n        #deny" << endl;
    //config_file << "\n        #server_name" << endl;
    config_file << "        server_name  localhost;" << endl;
    config_file << "        location / {" << endl;
    config_file << "            root   /usr/share/nginx/html;" << endl;
    config_file << "            index  index.html index.htm;" << endl;
    config_file << "        }" << endl;
    config_file << "    }" << endl;
    config_file << "}" << endl;

    config_file.close();

    cout << "\nSuccessfully generated Nginx configuration file.\n" << endl;

    return 0;
}

void changeErrorLogLocation()
{

    string configFile = "nginx.conf";
    string newErrorLogLocation, newErrorLogName;

    cout << "Enter the new location and new log name for error_log: ";
    getline(cin, newErrorLogLocation);
    cout << "Enter the new log name for error_log [blank if default]: ";
    getline(cin, newErrorLogName);
    newErrorLogName = newErrorLogName == "" ? "error.log" : newErrorLogName;

    ifstream inputFile(configFile);
    string line;
    string configContent;

    // Read the entire configuration file
    while (getline(inputFile, line))
    {
        configContent += line + "\n";
    }

    inputFile.close();

    // Find and replace the error_log line
    size_t errorLogPos = configContent.find("error_log");
    if (errorLogPos != string::npos)
    {
        size_t endLinePos = configContent.find("\n", errorLogPos);
        configContent.replace(errorLogPos, endLinePos - errorLogPos, "error_log " + newErrorLogLocation + (newErrorLogLocation.back() == '/' ? "" : "/") + newErrorLogName + ";");
    }

    // Write the modified configuration back to the file
    ofstream outputFile(configFile);
    outputFile << configContent;
    outputFile.close();

    cout << "\nSuccessfully updated the error_log location in the configuration file.\n" << endl;
}

void changeAccessLogLocation()
{

    string configFile = "nginx.conf";
    string newAccessLogLocation, newAccessLogName;

    cout << "Enter the new location and new log name for access_log: ";
    getline(cin, newAccessLogLocation);
    cout << "Enter the new log name for access_log [blank if default]: ";
    getline(cin, newAccessLogName);
    newAccessLogName = newAccessLogName == "" ? "error.log" : newAccessLogName;

    ifstream inputFile(configFile);
    string line;
    string configContent;

    // Read the entire configuration file
    while (getline(inputFile, line))
    {
        configContent += line + "\n";
    }

    inputFile.close();

    // Find and replace the error_log line
    size_t accessLogPos = configContent.find("access_log");
    if (accessLogPos != string::npos)
    {
        size_t endLinePos = configContent.find("\n", accessLogPos);
        configContent.replace(accessLogPos, endLinePos - accessLogPos, "access_log " + newAccessLogLocation + (newAccessLogLocation.back() == '/' ? "" : "/") + newAccessLogName + ";");
    }

    // Write the modified configuration back to the file
    ofstream outputFile(configFile);
    outputFile << configContent;
    outputFile.close();

    cout << "\nSuccessfully updated the access_log location in the configuration file.\n" << endl;
}

void replaceListenPorts() {
    string configFile = "nginx.conf";
    ifstream inputFile(configFile);
    string line;
    string configContent;


    while (getline(inputFile, line)) {
        configContent += line + "\n";
    }

    inputFile.close();


    configContent = regex_replace(configContent, regex(R"(listen\s+[^;]+;)"), "");

    string newListenPorts;

    cout << "Enter the new listen ports (separated by space): ";
    getline(cin, newListenPorts);

    istringstream iss(newListenPorts);
    vector<string> listenPorts(istream_iterator<string>{iss}, istream_iterator<string>());

    
    string serverBlockEnd = "}";

    size_t insertPosition = configContent.find(serverBlockEnd);
    if (insertPosition != string::npos) {
        string listenDirectivesStr;
        for (const string& listenDirective : listenPorts) {
            listenDirectivesStr += "    listen " + listenDirective + ";\n";
        }

        configContent.insert(insertPosition, listenDirectivesStr);
    }

    
    ofstream outputFile(configFile);
    outputFile << configContent;
    outputFile.close();

    cout << "\nSuccessfully replaced listening ports in the configuration file.\n" << endl;
}

void replaceListenHosts() {
    ifstream inputFile(configFile);
    string line;
    string configContent;


    while (getline(inputFile, line)) {
        configContent += line + "\n";
    }

    inputFile.close();

    //ALLOW
    configContent = regex_replace(configContent, regex(R"(allow\s+[^;]+;)"), "");

    string newAllowHosts;

    cout << "Enter the new allowed hosts (separated by space): ";
    getline(cin, newAllowHosts);

    istringstream issa(newAllowHosts);
    vector<string> listenPortsAllow(istream_iterator<string>{issa}, istream_iterator<string>());

    string serverBlockEnd = "}";

    size_t insertPosition = configContent.find(serverBlockEnd);
    if (insertPosition != string::npos) {
        string listenAllowPortsStr;
        for (const string& listenDirective : listenPortsAllow) {
            listenAllowPortsStr += "    allow " + listenDirective + ";\n";
        }

        configContent.insert(insertPosition, listenAllowPortsStr);
    }

    //DENY
    configContent = regex_replace(configContent, regex(R"(deny\s+[^;]+;)"), "");

    string newDenyHosts;

    cout << "Enter the new deny hosts (separated by space): ";
    getline(cin, newDenyHosts);

    istringstream issd(newDenyHosts);
    vector<string> listenPortsDeny(istream_iterator<string>{issd}, istream_iterator<string>());

    insertPosition = configContent.find(serverBlockEnd);
    if (insertPosition != string::npos) {
        string listenDenyPortsStr;
        for (const string& listenDirective : listenPortsDeny) {
            listenDenyPortsStr += "    deny " + listenDirective + ";\n";
        }

        configContent.insert(insertPosition, listenDenyPortsStr);
    }

    
    ofstream outputFile(configFile);
    outputFile << configContent;
    outputFile.close();

    cout << "\nSuccessfully updated listen/allow/deny in the configuration file.\n" << endl;
}

void updateServerBlock() {
    std::ifstream inputFile(configFile);
    std::string line;
    std::string configContent;

    // Read the entire configuration file
    while (std::getline(inputFile, line)) {
        configContent += line + "\n";
    }

    inputFile.close();

    std::string newListenDirectives;
    std::string newAllowDirectives;
    std::string newDenyDirectives;

    // Get the new directives from user input
    std::cout << "Enter the new listen directives (separated by space): ";
    std::getline(std::cin, newListenDirectives);

    std::cout << "Enter the new allow directives (separated by space): ";
    std::getline(std::cin, newAllowDirectives);

    std::cout << "Enter the new deny directives (separated by space): ";
    std::getline(std::cin, newDenyDirectives);

    // Remove existing listen, allow, and deny directives
    size_t listenPos = configContent.find("listen");
    while (listenPos != std::string::npos) {
        size_t endLinePos = configContent.find("\n", listenPos);
        configContent.erase(listenPos, endLinePos - listenPos + 1);
        listenPos = configContent.find("listen");
    }

    size_t allowPos = configContent.find("allow");
    while (allowPos != std::string::npos) {
        size_t endLinePos = configContent.find("\n", allowPos);
        configContent.erase(allowPos, endLinePos - allowPos + 1);
        allowPos = configContent.find("allow");
    }

    size_t denyPos = configContent.find("deny");
    while (denyPos != std::string::npos) {
        size_t endLinePos = configContent.find("\n", denyPos);
        configContent.erase(denyPos, endLinePos - denyPos + 1);
        denyPos = configContent.find("deny");
    }

    // Find the server block
    std::string serverBlockStart = "server {";
    std::string serverBlockEnd = "}";

    size_t startPos = configContent.find(serverBlockStart);
    size_t endPos = configContent.find(serverBlockEnd, startPos);

    if (startPos != std::string::npos && endPos != std::string::npos) {
        std::string newDirectives = "\n";

        // Construct the new directives
        if (!newListenDirectives.empty()) {
            newDirectives += "\t\tlisten " + newListenDirectives + ";\n";
        }
        if (!newAllowDirectives.empty()) {
            newDirectives += "\t\tallow " + newAllowDirectives + ";\n";
        }
        if (!newDenyDirectives.empty()) {
            newDirectives += "\t\tdeny " + newDenyDirectives + ";\n";
        }

        // Insert the new directives inside the server block
        configContent.insert(endPos, newDirectives);
    }

    // Write the modified configuration back to the file
    std::ofstream outputFile(configFile);
    outputFile << configContent;
    outputFile.close();
}




int main()
{
    generateDeafaultConfigFile();
    changeErrorLogLocation();
    changeAccessLogLocation();
    replaceListenPorts();
    replaceListenHosts();
    updateServerBlock();
    //modifyServerSettings();
    return 0;
}
