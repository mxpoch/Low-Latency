#include<iostream>
#include<vector>
#include<fstream>
#include<filesystem>
#include<string>
#include<chrono>
#include<map>
#include<cassert>
#include<stdio.h>
#include<cmath>

namespace fs = std::filesystem;

void print(std::vector<std::string> &valids);
std::size_t linecount(std::ifstream &filehandle);
std::vector<std::string> extract_cik();
std::vector<std::string> sub_scan(std::string &company);
bool cik_match(std::string company_cik, std::string &subentry);
std::string findsubdir(std::string &sub);
std::string get_quarter(std::string &month);
std::string find_cik(std::string &line);

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(NULL);

    // extracting ciks from flattened json
    std::vector<std::string> ciks = extract_cik();

    std::cout<<"building subindex...."<<std::endl;
    // creating subindex map structure 
    std::map<std::string, std::string> subindex;
    for( auto& entry : fs::directory_iterator("F:\\LPS\\SEC_DB"))
    {
        bool start{true};
        std::ifstream infile;
        fs::current_path(entry);
        std::cout<<"dir: "<<entry<<std::endl;
        infile.open("sub.tsv");
        std::string line; 
        while(std::getline(infile, line)) // can be optmized with fgets
        {
            if(start) {start=false; continue;}
            std::string cik{find_cik(line)};
            std::string adsh{line.substr(0,20)};

            subindex.insert(std::make_pair(adsh, cik));
        }
        infile.close();
    }
    std::cout<<"created subindex"<<std::endl;

    // creating new directories
    std::cout<<"building directories..."<<std::endl;
    fs::current_path("F:\\LPS\\CORP_DB");
    std::size_t count{0};
    for(auto& ck : ciks) 
    { 
        std::cout<<count<<" of "<<ciks.size()<<std::endl;
        ++count;
        fs::create_directory(ck);
    }

    std::cout<<"cloning reindexed database..."<<std::endl;
    // writing to actual files
    std::vector<std::string> filetypes = {"sub", "pre", "num", "txt", "cal"};
    for( auto& entry : fs::directory_iterator("F:\\LPS\\SEC_DB"))
    {
        std::cout<<"dir: "<<entry<<std::endl;
        fs::current_path(entry);
        for(auto &filet : filetypes)
        {
            std::cout<<"file: "<<filet<<std::endl;
            std::ifstream infile;
            infile.open(filet+".tsv");
            std::string line;
            std::size_t cline{0};
            auto start = std::chrono::high_resolution_clock::now();
            while(std::getline(infile, line))
            {
                std::string adsh{line.substr(0,20)};
                std::string cik{subindex[adsh]};
                if(cik.length() == 0) {continue;}
                std::string dir{"F:\\LPS\\CORP_DB\\"+cik+"\\"};
                // updating individual files
                std::ifstream rnfile;
                std::ofstream onfile;
                rnfile.open(dir+filet+".tsv");
                onfile.open(dir+filet+".tsv", std::ios::app);
                onfile<<line<<"\n";
                rnfile.close();
                onfile.close();
                ++cline;
            }
            infile.close();
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop-start);
            std::cout<<"Copied "<<cline<<" lines in "<<duration.count()<<" ms"<<std::endl;
        }
    }
    
}

std::string find_cik(std::string &line)
{
    std::size_t end{0};

    while(!isspace(line[21+end])) {++end;}
    return line.substr(21, end);
}

bool compare(char *line1, std::string &line2)
{
    for(std::size_t i{0}; i < 20; ++i)
    {
        if(line1[i] != line2[i]) { return false; }
    }
    return true;
}

void clear(char *line)
{
    for(std::size_t i{0}; i < 2000; ++i) { line[i] = *" "; }
}

void print(char *line)
{
    for(std::size_t i{0}; i < 100; ++i) { std::cout<<line[i]; }
    std::cout<<std::endl;
}

std::string get_quarter(std::string &month)
{
    std::size_t monthnum = std::stoi(month);
    if( monthnum < 4) { return "Q1"; }
    else if (monthnum < 7) { return "Q2";}
    else if (monthnum < 10) { return "Q3"; }
    else if (monthnum < 12) { return "Q4";}
    else { return "Q0"; };
}

std::string findsubdir(std::string &sub)
{
    std::size_t tabs{0};
    std::size_t cha{0};
    for(cha; tabs < 29; ++cha)
    {
        if( sub[cha] == *"\t") { ++tabs; }
    }
    std::string date{sub.substr(cha, 6)};
    std::string filedate;

    std::string mon{date.substr(4,2)};
    if(std::stoi(date) >= 202010)
    {
        filedate = date.substr(0, 4) + "-" + mon;
        return filedate;
    }
    filedate = date.substr(0,4) + "-" + get_quarter(mon);
    return filedate;
}

bool cik_match(std::string company_cik, std::string &subentry)
{
    std::size_t cha{21};
    for(cha; cha < subentry.length(); ++cha)
    {
        if(subentry[cha] == *"\t") { break; }
    }
    if(company_cik == subentry.substr(21, cha-21)) { return true; }
    else { return false; };
}

std::vector<std::string> sub_scan(std::string &company_cik)
{
    // traversing all the directories
    std::vector<std::string> submissions;
    bool header{true};
    for(auto &entry: fs::directory_iterator("F:\\LPS\\SEC_DB"))
    {
        std::ifstream subfile;
        fs::current_path(entry);
        subfile.open("sub.tsv");
        
        std::size_t c{0};
        std::string line;
        while(std::getline(subfile, line))
        {
            if(cik_match(company_cik, line) || header)
            {
                submissions.push_back(line);
            }
            if(header) { header = false; }
        }
        subfile.close();
    }
    return submissions; 
}

std::vector<std::string> extract_cik()
{
    std::ifstream cik_file;
    cik_file.open("F:\\LPS\\flattened_ticker.csv"); 
    
    std::string line;
    std::vector<std::string> ciks;
    while(std::getline(cik_file, line))
    {
        for(std::size_t i{0}; i < line.length(); ++i)
        {
            if(line[i] == ',')
            {
                ciks.push_back(line.substr(0, i));
                break;
            }
        }
    }
    cik_file.close();
    return ciks;
}

std::size_t linecount(std::ifstream &filehandle)
{
    std::size_t lines{0};
    std::string line;
    for(lines; std::getline(filehandle, line); ++lines);
    filehandle.clear();
    filehandle.seekg(0, filehandle.beg);
    return lines;
}

void print(std::vector<std::string> &valids)
{
    for(auto &stri : valids)
    {
        std::cout<<stri<<std::endl;
    }
}