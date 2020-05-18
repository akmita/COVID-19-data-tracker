/*main.cpp*/

//
// header comment???
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <locale>
#include <iomanip>
#include <vector>
#include <stack>
#include <map>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

//
// struct stores the date of data, confirmed cases, deaths, recovered cases
//
struct countryData {
    string date;
    int numCases;
    int numDeaths;
    int numRecovered;
};


// ---------------------------------------------------------------------\\
// ------------------------ HELPER FUNCTIONS--------------------------- ||
// ---------------------------------------------------------------------//

//
// getFilesWithinFolder
//
// Given the path to a folder, e.g. "./daily_reports/", returns 
// a vector containing the full pathnames of all regular files
// in this folder.  If the folder is empty, the vector is empty.
//
vector<string> getFilesWithinFolder(string folderPath)
{
    vector<string> files;

    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            // this gives you just the filename:
            // files.push_back(entry.path().filename().string());

            // this gives you the full path + filename:
            files.push_back(entry.path().string());
        }
    }

    sort(files.begin(), files.end());

    return files;
}


//
// prints out each node in order first by country then by date newst to oldest
// 
void dump(map<string,stack<countryData>> data) {
    // loop through map
    for (auto it = data.begin(); it != data.end(); ++it) {
        cout << "\nCountry: " << it->first << endl;
        while (!it->second.empty()) {   // print and pop stack while not empty
            cout << "date: " << it->second.top().date << endl;
            cout << "numcases: " << it->second.top().numCases << endl;
            cout << "numDeaths: " << it->second.top().numDeaths << endl;
            cout << "numRecovered: " << it->second.top().numRecovered << endl << endl;
            it->second.pop();

        }
        cout << "--------------------------" << endl;
    }
    cout << "==============================================\n";
}


//
// formats date into mm-dd-yyyy format assuming year 2020
// ONLY formats if the date isn't already formatted
// returns curDate by reference, and takes in last_update
// which is the date we are currently working with
//
void formatDate(string &curDate, string last_update) {
    if (curDate == "") {
        string month, day;

        // removes the timestamp from string, leaving just date
        stringstream str(last_update);
        if (last_update.find('T') != last_update.npos) {    // if contains a 'T' between date and timestamp
            getline(str, curDate, 'T');
        }
        else {                                              // if contains a space between date and timestamp
            getline(str, curDate, ' ');
        }

        // puts the date in stringstream
        str.str(string());
        str << curDate;

        // CASE 1: date is mm/dd/yyyy  format
        if (curDate.find('/') != curDate.npos) {
            getline(str, month, '/');   // store in month variable
            getline(str, day, '/');     // store in month variable
        }

        // CASE 2: date is yyyy-mm-dd  format
        else if (curDate.find('-') != curDate.npos) {
            getline(str, month, '-');   // gets year, we're not using this  
            getline(str, month, '-');   // gets month, replaces year value which we're not using 
            getline(str, day);          // gets the day, which is the last thing in stream
        }
        else {
            cout << "ERROR PARSING DATE.   curDate: " << curDate << endl;
        }

        // fix one digit month and day
        if (day.length() == 1) {
            day = "0" + day;
        }
        if (month.length() == 1) {
            month = "0" + month;
        }

        // contruct string in mm-dd-yyyy format 
        curDate = "";
        curDate += month + "-" + day + "-" + "2020";
    }
}


//
// inserts data into the map, each map key is a country's name, and its data 
// is a stack of structs with the top containing the newest data
// takes in filename and map, and returns the updated map by reference
//
void insertData(string currFile, map<string, stack<countryData>> &data) {
    ifstream file;
    string province, country, last_update, confirmed, deaths, recovered;
    string curDate = "";
    string line;


    // cout << "Reading file: " << currFile << endl;

    file.open(currFile);            // OPEN FIRST FILE JUST FOR TESTING
    if (!file.is_open()) {
        cout << "Error reading file\n";
        exit(-1);
    }


    getline(file, line);                    // discard first line

    cout << "Reading file: " << currFile << ". Label:  " << line << endl;


    while (getline(file, line)) {

        // parse line appropriately, if province is actually a city/state
        if (line[0] == '"') {               
            line.erase(0, 1);
            size_t pos = line.find(',');
            line.erase(pos, 1);
            pos = line.find('"');
            line.erase(pos, 1);
        }

        stringstream stream(line);

        // add data into separate variables   -- can't go in this order because order on daily reports changed starting march 27th
        getline(stream, province, ',');
        getline(stream, country, ',');
        getline(stream, last_update, ',');
        getline(stream, confirmed, ',');
        getline(stream, deaths, ',');
        getline(stream, recovered, ',');

        // TO DO
        loop through line elements 
            >> operator to get one token at a time
            loop through and match each data element 
                province, country, last_update, confirmed, deaths, recovered
                ignore unmatched 
        


        // parse special cases
        if (confirmed == "") {
            confirmed = "0";
        }
        if (deaths == "") {
            deaths = "0";
        }
        if (recovered == "") {
            recovered = "0";
        }
        if (country == "Mainland China") {
            country = "China";
        }

        // Formats date once per function call 
        // (since each function call coresponds to one file, and one single date)
        formatDate(curDate, last_update);       

        // CASE 1: if country IS NOT in map    -OR- 
        //         if country IS in map and NOT matching date,
        if ( data.find(country) == data.end()  || data[country].top().date != curDate ) {
            countryData N;

            // add data to struct
            N.date = curDate;
            N.numCases = stoi(confirmed);
            N.numDeaths = stoi(deaths);
            N.numRecovered = stoi(recovered);

            // push struct into stack
            data[country].push(N); 
        }
        // CASE 2: if country is in map and matching date    
        else {
            // sum the old data and new data
            data[country].top().numCases += stoi(confirmed);
            data[country].top().numDeaths += stoi(deaths);
            data[country].top().numRecovered += stoi(recovered);
        }
    }   
}


//
// reads files from worldfacts and inserts into map
// (called separately for each file)
template <typename T>
void insertCountryData(map<string, T> &map, string filepath) {
    ifstream file(filepath);
    string line, country, numeric_value;

    if ( ! file.is_open()) {
        cout << "Error reading population file. Exiting.\n";
        exit(-1);
    }

    getline(file, line);                    // discard first line

    // add population data to map
    while (getline(file, line)) {
        stringstream stream(line);

        getline(stream, country, ',');      // discard first val
        getline(stream, country, ',');      // get country name
        getline(stream, numeric_value);     // get the country's data (either population or life expect.)

        if (numeric_value.find('.') == numeric_value.npos) {    // if value is NOT decimal
            map[country] = stoi(numeric_value);                     // parse to integer and insert
        }
        else {                                                  // if value is decimal 
            T num = stod(numeric_value);                            // parse to float
            map[country] = num;                                     // insert number
        }
        
    }

    file.close();
}


//
// gets the date of first case  
// parameter: stack of structs for that country
//
string getFirstCaseDate(stack<countryData> stack) {
    countryData prev;

    // pop stack until size is 1
    while (stack.size() > 0 && stack.top().numCases != 0) {
        prev = stack.top();
        stack.pop();
    }
    // return the date when stack size is 1
    return prev.date;
}


//
// gets the date of the first death
// parameter: stack of structs for that country
//
string getFirstDeathDate(stack<countryData> stack) {
    countryData prev;
    string prevDate = "none";

    // pop stack until size is 1
    while (stack.size() > 0 && stack.top().numDeaths != 0) {
        prev = stack.top();
        prevDate = prev.date;
        stack.pop();
    }
    // return the date when stack size is 1
    return prevDate;
}


//
// adds timeline data to output string
// 
void addTimelineData(stack<countryData> &stack, vector<int> &curDayData, char &userChoice) {
    // Add the appropriate data to output string
    switch (userChoice) {
    case 'c':
        curDayData.insert(curDayData.begin(), stack.top().numCases);
        break;
    case 'd':
        curDayData.insert(curDayData.begin(), stack.top().numDeaths);
        break;
    case 'r':
        curDayData.insert(curDayData.begin(), stack.top().numRecovered);
        break;
    }
}



// displays timelines of either confirmed cases, deaths, or recoveries of specific country
// parameters: stack of structs for that country, the option the user selects
void displayTimeline(stack<countryData> stack, char userChoice) {
    int counter = stack.size();
    vector<string> dates;       // vectors store the current date and data of that date
    vector<int> curDayData;
    int dayNum = 0;
    int start;

    // print appropriate header
    switch (userChoice) {
        case 'c':
            cout << "Confirmed:\n";
            break;
        case 'd':
            cout << "Deaths:\n";
            break;
        case 'r':
            cout << "Recovered:\n";
            break;
        case 'n':
            return;
        default:
            cout << "*Invalid choice\n";
    }

    // loop until stack empty   or no cases. that will be the start of the timeline
    while (!stack.empty() && stack.top().numCases > 0) {
        dates.insert(dates.begin(), stack.top().date);            // Add the date to dates vector

        addTimelineData(stack, curDayData, userChoice);           // adds data to output string, returns by reference
        stack.pop();
        counter--;
    }

    // find first non-zero in vector
    for (dayNum; dayNum < curDayData.size(); dayNum++) {
        if (curDayData[dayNum] != 0) {
            break;
        }
    }
    
    start = dayNum;
    
    for (dayNum; dayNum < dates.size(); dayNum++) {
        if (dayNum < start + 7 || dayNum >= dates.size() - 7) {         // display first and last 7                                                               
            cout << dates[dayNum] << " (day " << dayNum + 1 << "): " << curDayData[dayNum] << endl;
        }
        else if (dayNum < start + 8) {       // display a dot three times 
            cout << " .\n .\n .\n";
        }
        else {
            // do nothing
        }
    }
    
   
}


// ---------------------------------------------------------------------\\
// --------------------- USER COMMAND FUNCTIONS------------------------ ||
// ---------------------------------------------------------------------//

//
// displays help menus
//
void help() {
    cout << "Available commands:\n";
    cout << " <name>: enter a country name such as US or China\n";
    cout << " countries: list all countries and most recent report\n";
    cout << " top10: list of top 10 countries based on most recent # of confirmed cases\n";
    cout << " totals: world-wide totals of confirmed, deaths, and recovered\n\n";
}


//
// give info of country 
//
void displaySingleCountry(map<string, stack<countryData>> &data, map<string, int> &populations, 
                        map<string, float> &life_expect,  string country) {
    char userChoice;
    string userinput;
    
    // if country extists
    if (data.find(country) != data.end()) {
        // display population
        cout << "Population: " << populations[country] << endl;
        // display life exectancy
        cout << "Life Expectancy: " << life_expect[country] << " years" << endl;
        // display latest epidemic info
        cout << "Latest data: " << endl;
        cout << " confirmed: " << data[country].top().numCases << endl;
        cout << " deaths: " << data[country].top().numDeaths << endl;
        cout << " recovered: " << data[country].top().numRecovered << endl;
        cout << "First confirmed case: " << getFirstCaseDate(data[country]) << endl;
        cout << "First recorded death: " << getFirstDeathDate(data[country]) << endl;

        // prompt user for timeline 
        cout << "Do you want to see a timeline? Enter c/d/r/n> ";
        getline(cin, userinput);
        userChoice = userinput.at(0);

        displayTimeline(data[country], userChoice);
    }        
}



//
// lists all countries and most recent reports
//
void displayAllCountries(map<string, stack<countryData>> &data) {
    // get latest date 
    string latestDate = data["US"].top().date;  // latest US date is latest update, since US has updates every day

    // display info about country if date matches the latest date, else display all 0s for data
    for (auto it = data.begin(); it != data.end(); it++) {
        if (it->second.top().date == latestDate) {
            cout << it->first << ": "
                 << it->second.top().numCases << ", "
                 << it->second.top().numDeaths << ", "
                 << it->second.top().numRecovered << endl;
        }
        else {
            cout << it->first << ": "
                << 0 << ", "
                << 0 << ", "
                << 0 << endl;
        }
    }
}


//
// displays top 10 countries based on confirmed cases
// and show most recent reports
//
void displayTopTen(map<string, stack<countryData>> &data) {
    vector<string> top10;  
    string curMaxCountry;
    string latestDate = data["US"].top().date;  // latest US date is latest update, since US has updates every day
    int max = 0;
    
    for (int i = 1; i <= 10; i++) {         
        // finds country with max number of cases that hasn't been listed already
        for (auto it = data.begin(); it != data.end(); it++) {              // loop through data
            if (it->second.top().numCases >= max) {                             // if numCases value is more than current max
                if (find(top10.begin(), top10.end(), it->first) == top10.end()) {    // if country isn't already listed
                    if (it->second.top().date == latestDate) {                           // if up to date info
                        curMaxCountry = it->first;                                          // country is marked as max                                    
                        max = it->second.top().numCases;                                    // update max value
                    } 
                }               
            }
        }
                    
        top10.push_back(curMaxCountry);     // add country name with max cases to vector
        max = 0;                            // resets max value for next iteration     
    }

    // display top 10 countries and number of cases
    for (int i = 0; i < top10.size(); i++) {
        cout << i + 1 << ". " << top10[i] << ": " << data[top10[i]].top().numCases << endl;
    }
}   


//
// displays world-wide totals, summing most recent reports
//
void displayWorldTotals(map<string, stack<countryData>>& data) {
    int cases = 0; 
    int deaths = 0;
    int recoveries = 0;
    int count = 1; 
    // latest US date is latest update, since US has updates every day
    string latestDate = data["US"].top().date;       

    // sum data
    for (auto it = data.begin(); it != data.end(); it++) {
        if (it->second.top().date == latestDate) {
            cases += it->second.top().numCases;
            deaths += it->second.top().numDeaths;
            recoveries += it->second.top().numRecovered;
            count++;
        }
    }

    // display data
    cout << "As of " << latestDate << ", the world-wide totals are:\n";
    cout << " confirmed: "  << cases << endl;
    cout << " deaths: "     << deaths       << " (" << 100.0 * deaths / cases      << "%)" << endl;
    cout << " recovered: "  << recoveries   << " (" << 100.0 * recoveries / cases  << "%)" << endl;
}   


// 
// displays a world timeline along with percentage increases of virus cases from previous day.
//    This gives us idea of how fast the virus is spreading worldwide
//   
void displaySpreadRates(map<string, stack<countryData>> data) {
    string curDate;  
    vector<int> confirmedCases(200, 0);          // holds sum of world cases for corresponding date at same index in the dates vector   
    vector<string> dates(200, "");               // holds DATES in order 
    int iterations = 0;          
    int startingElement = 0, dayNum = 1;
   
    data["Republic of Korea"].top().numCases = 0;   // same as South Korea, throws off the total world count

    cout << "Displaying world timeline of virus and rates of change from previous day\n";

    while (!data["China"].empty()) {        // loop untill first day of virus data
        curDate = data["China"].top().date;     
        dates[iterations] = curDate;            // sets the current date we're working on in DATE vector
        // pops one element off each country with matching date, and sums confirmed cases in vector element
        for (auto it = data.begin(); it != data.end(); it++) {

            if (it->second.empty()) {                                   // ignore countries with no data
                continue;
            }
            confirmedCases[iterations] += it->second.top().numCases;    // sum the latest data, ragardless of matching date
            if (it->second.top().date == curDate) {                     // if current date matches, pop the stack
                it->second.pop();
            }
        }
        iterations++;
    }

    // find the first unitialized element in vector
    for (int i = 0; i < dates.size(); i++) {
        if (dates[i] == "") {
            startingElement = i - 1;
            break;
        }
    }

    // print the world timeline
    for (int i = startingElement; i >= 0; i--) {
        cout << setw(10) << dates[i];                                       // display dates 
        cout << setw(8) << " (Day: " << setw(2) << dayNum << ") ";          // display day number
        cout << setw(8) << confirmedCases[i] << " cases";                   // display confirmed cases                                
        if (i < startingElement) {                                          // display rate of change if possible

            cout << setw(9) << (1.0 * confirmedCases[i] / confirmedCases[i + 1]) * 100 - 100  << " %  change";
        } 
        cout << endl;
        dayNum++;
    }
}


//
// main:
//
int main()
{
    cout << "** COVID-19 Data Analysis **" << endl;
    cout << endl;
    cout << "Based on data made available by John Hopkins University" << endl;
    cout << "https://github.com/CSSEGISandData/COVID-19" << endl;
    cout << endl;

    //
    // setup cout to use thousands separator, and 2 decimal places:
    //
    cout.imbue(std::locale(""));
    cout << std::fixed;
    cout << std::setprecision(2);

                                                     // CHANGED VALUE, LOADING FULLY UPDATED FILE NOW
    vector<string> files = getFilesWithinFolder("./data-files/COVID-19/csse_covid_19_data/csse_covid_19_daily_reports/");     // stores all daily report filenames
    map<string, stack<countryData>> data;                                // stores all countries and info in a map of stacks           
    map<string, int> populations;                                        // map of populations of every country
    map<string, float> life_expect;                                      // map of life expectancies of every country
    string command;                                                      // user specified command
    int cycles = 0;

    // fills map with data on each country
    for (auto filename : files) {
        insertData(filename, data);
    }
    // load world facts from files
    insertCountryData(populations, "./worldfacts/populations.csv");
    insertCountryData(life_expect, "./worldfacts/life_expectancies.csv");
    // display info on files loaded
    cout << ">> Processed " << files.size() << " daily reports\n";
    cout << ">> Processed " << 2 << " files of world facts\n";
    cout << ">> Current data on " << data.size() << " countries\n";
    // main program loop
    while (1) {             
        if (cycles == 0) {
            cout << "\nEnter command (help for list, # to quit)> ";
        }
        else {
            cout << "\nEnter command> ";
        }
        getline(cin, command);

        if (command == "#") {
            exit(0);
        }
        else if (command == "help") {
            help();
        }
        else if (data.find(command) != data.end()) {     // if command is a country name
            displaySingleCountry(data, populations, life_expect, command);
        }
        else if (command == "countries") {
            displayAllCountries(data);
        }
        else if (command == "top10") {
            displayTopTen(data);
        }
        else if (command == "totals") {
            displayWorldTotals(data);
        }
        else if (command == "spread") {
            displaySpreadRates(data);
        }
        else {
            cout << "country or command not found...\n";
        }
       
        cycles++;
    }
    return 0;
}
