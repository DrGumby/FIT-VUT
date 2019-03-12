#include <iostream>
#include <fstream>
#include <string>
#include <queue>

using namespace std;



void printQueue (queue<string> lines, int length)
{
    for (int i = 0; i < length; i++) {
        cout << lines.front() << '\n';
        lines.pop();
    }

}

void tail(int lineCount, istream& source)
{
    int i = 0;
    string lineOfInput;
    queue<string> lines;
    while (getline(source, lineOfInput))
    {
        if (i<lineCount)
        {
            lines.push(lineOfInput);
        }
        else
        {
            lines.pop();
            lines.push(lineOfInput);
        }
        if(i<lineCount)
            i++;
    }
    printQueue(lines, i);
}

int argCheck(string arga, string argb)
{
    if (arga == "-n")
    {
        int lineCount = std::stoi(argb, nullptr, 0);
        return lineCount;
    }
    else
    {
        cerr << "Invalid parameters, exiting.\n";
        return -1;
    }
}


int main(int argc, char  *argv[])
{
    std::ios::sync_with_stdio(false);
    switch (argc) {
        case 1:
            {
                int lineCount = 10;
                tail(lineCount, cin);
                break;
            }
        case 2:
            {
                int lineCount = 10;
                ifstream fp;
                fp.open(argv[1]);
                if (fp.is_open()) {
                    tail(lineCount, fp);
                }
                else{
                    cerr << "Error, file cannot be opened\n";
                    return 1;
                }
                fp.close();
                break;
            }
        case 3:
            {
                int lineCount = (int)argCheck(argv[1], argv[2]);
                if (lineCount > 0) {
                    tail(lineCount, cin);
                }
                else{
                    cerr << "Error, invalid arguments.\n";
                    return 1;
                }

                break;
            }
        case 4:
            {
                int lineCount = (int)argCheck(argv[1], argv[2]);
                if (lineCount > 0) {
                    ifstream fp;
                    fp.open(argv[3]);
                    if (fp.is_open()) {
                        tail(lineCount, fp);
                    }
                    else{
                        cerr << "Error, file cannot be opened.\n";
                        return 1;
                    }
                    fp.close();
                }
                else{
                    cerr << "Error, invalid argument\n";
                    return 1;
                }
                break;
            }
    }
    return 0;
}
