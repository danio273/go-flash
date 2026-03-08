#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
using namespace std;

void write_prompt(const string& prompt) {
    ofstream data("DATA.TXT");
    data << prompt;
    data.close();
}

void trigger_file() {
    ofstream go("GO.TXT");
    go.close();
}

string get_text() {
    ifstream file("DATA.TXT", ios::binary);
    if (!file.is_open()) {
        return "";
    }

    file.seekg(0, ios::end);
    size_t size = file.tellg();
    if (size == 0) {
        return "";
    }
    
    string buffer(size, '\0');
    file.seekg(0);
    file.read(&buffer[0], size);
    
    return buffer;
}

void clear_screen() {
    #ifdef _WIN32 
        system("cls");
    #else 
        system("clear");
    #endif
}

int main() {
    string input = "";
    string previous_text = "";
    
    cout << "=== GoFlash Prompt Interface ===" << "\n";
    cout << "Enter your prompts below. Type 'STOP' to exit." << "\n\n";
    
    while (true) {
        cout << "> Prompt: ";
        getline(cin, input);
        
        if (input == "STOP" || input == "stop") {
            cout << "Exiting..." << "\n";
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        // Write prompt to DATA.TXT
        write_prompt(input);
        // Trigger
        trigger_file();
        
        cout << "Waiting for AI response";
        cout.flush();
        
        // Wait for response
        previous_text = input;
        string current_text = get_text();
        
        while (current_text == previous_text || current_text.empty()) {
            cout << ".";
            cout.flush();
            this_thread::sleep_for(chrono::milliseconds(500));
            current_text = get_text();
        }
        
        cout << "\n\n=== Response ===" << "\n";
        cout << current_text << "\n";
        cout << "================\n\n";
    }
    
    return 0;
}