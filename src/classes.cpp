#include<iostream>
#include<string>
#include<vector>
#include<map>
#include <exception>
#include<fstream>
#include<thread>
#include<mutex>
#include "nlohmann/json.hpp"

#include "classes.h"

std::mutex docsAccess, freq_dictionaryAccess;

const char* fileMissingException::what() const noexcept
{
    return "Config file is missing";
};

const char* fileEmptyException::what() const noexcept
{
    return "Config file is empty";
};

std::vector<std::string> ConverterJSON::GetTextDocuments()
{
    
    std::vector<std::string> vec;
    
    std::ifstream file("config.json");
    if(file.is_open())
    {
        nlohmann::json dict;
        file >> dict;

        if(dict["config"].size()==0)
            throw fileEmptyException();
        for(auto j : dict["files"])
        {
            vec.push_back(j);
        };
    } else {
        throw fileMissingException();
    };
    file.close();

    return vec;
};

int ConverterJSON::GetResponsesLimit()
{
    int responsesLimit = 5;
    
    std::ifstream file("config.json");
    if(file.is_open())
    {
        nlohmann::json dict;
        file >> dict;

        if(dict["config"].size()==0)
            throw fileEmptyException();
        else 
            if(dict["config"].contains("max_responses"))
                responsesLimit = dict["config"]["max_responses"];       
    } else {
        throw fileMissingException();
    };
    file.close();
    return responsesLimit;
};

std::vector<std::string> ConverterJSON::GetRequests()
{
    std::vector<std::string> vec;
    
    std::ifstream file("requests.json");
    if(file.is_open())
    {
        nlohmann::json dict;
        file >> dict;

        if(dict["requests"].size()>0)
        {
            for(auto j : dict["requests"])
            {
                vec.push_back(j);
            };
        };
    }
    file.close();

    return vec;
};

void ConverterJSON::putAnswers(std::vector<std::vector<RelativeIndex>> answers)
{
    std::ofstream file("answers.json");
    file << "{" << std::endl << "\"answers\": {" << std::endl;

    int counter = 1;
    for(auto i : answers)
    {
        file << "\"request00" << counter << "\": {" << std::endl;

        file << "\"result\": " << ((i.size()==0)?("\"false\""):("\"true\",")) << std::endl;
        if(i.size() > 1)
            file << "\"relevance\": {" << std::endl;
        
        int counterJ = 1;
        for(auto j : i)
        {
            file << "\"docid\": " << j.doc_id << ", \"rank\": " << j.rank;
            if(counterJ<i.size()) file << ",";
            file << std::endl;
            counterJ++;
        };

        if(i.size() > 1)
            file << "}" << std::endl;

        file << ((counter<answers.size())? ("},") : ("}")) << std::endl;
        counter++;
    }

    file << "}" << std::endl << "}" << std::endl;
    file.close();
};

void fillDocs(int id, std::string input_doc, std::map<int,std::string>& mapDocs, std::map<std::string, std::vector<Entry>>& dictionary)
{
    std::ifstream file(input_doc);
    std::string doc = "";
     
    if(file.is_open())
    {
        std::string line;
        while(!file.eof())
        {
            //std::getline (file, line);
            file >> line;
            freq_dictionaryAccess.lock();
            auto it = dictionary.find(line);
            if(it == dictionary.end())
            {
                std::vector<Entry> emptyVec;
                Entry entry;
                entry.doc_id = id;
                entry.count = 1;
                emptyVec.push_back(entry);
                dictionary.insert(std::make_pair(line,emptyVec));
            } else {
                bool itStructFound = false;
                for(int i=0; i < it->second.size(); i++)
                {
                    if(it->second[i].doc_id == id)
                    {
                        it->second[i].count++;
                        itStructFound = true;
                        break;
                    };
                };
                if(!itStructFound)
                {
                    Entry entry;
                    entry.doc_id = id;
                    entry.count = 1;
                    it->second.push_back(entry);
                };
            };                
            freq_dictionaryAccess.unlock();
            doc += line;
            //doc += "\n";
            doc += " ";
        };
        doc += "\n";
    };
    docsAccess.lock();
    mapDocs.insert(std::make_pair(id,doc));
    docsAccess.unlock();
    file.close();
};

void InvertedIndex::UpdateDocumentBase(std::vector<std::string> input_docs)
{
    for(int i=0; i<input_docs.size(); i++)
    {
        std::thread call(fillDocs, i, input_docs[i], std::ref(docs), std::ref(freq_dictionary));
        call.join();
    };

    for(auto it=docs.begin(); it!=docs.end(); it++)
    {
        std::cout << "DocId " << it->first << ": " << std::endl;
        std::cout << it->second << std::endl;
    };

    for(auto it=freq_dictionary.begin(); it!=freq_dictionary.end(); it++)
    {
        std::cout << "Word: " << it->first << ": ";
        for(auto itStruct : it->second)
        {
            std::cout << "{" << itStruct.doc_id << "," << itStruct.count << "},";
        };
        std::cout << std::endl;
    };

};

std::vector<Entry> InvertedIndex::GetWordCount(const std::string& word)
{
    std::vector<Entry> vec;
    auto it = freq_dictionary.find(word);
    if(it != freq_dictionary.end())
    {
        return it->second;
    };    
    
    return vec;
};

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input)
{
    std::vector<std::vector<RelativeIndex>> vec;
    
    for(int i=0; i<queries_input.size(); i++)
    {
        std::string stdDoc = queries_input[i];
        std::vector<std::string> words;    
        const char* const delimeters = " ";
        char* token = std::strtok(stdDoc.data(), delimeters);        
        while (token != nullptr)
        {         
            words.push_back(token);            
            token = std::strtok(nullptr, delimeters);               
        };
        int totalCount = 0;
        std::vector<RelativeIndex> docRankVec;
        for(auto it : words)
        {
            std::cout << it << ": ";
            std::vector<Entry> entryVec = _index.GetWordCount(it);
            for(auto j : entryVec)
            {
                std::cout << "{" << j.doc_id << "," << j.count << "},";
                bool itDocRankFound = false;
                for(int i=0; i<docRankVec.size(); i++)
                {
                    if(docRankVec[i].doc_id == j.doc_id)
                    {
                        docRankVec[i].rank += j.count;
                        itDocRankFound = true;
                        break;
                    }
                };
                if(!itDocRankFound)
                {
                    RelativeIndex docRank;
                    docRank.doc_id = j.doc_id;
                    docRank.rank = j.count;
                    docRankVec.push_back(docRank);
                };

                totalCount += j.count;
            };
        };
        std::cout << std::endl << totalCount << std::endl;
        for(int i=0; i<docRankVec.size(); i++)
        {
            std::cout << "{" << docRankVec[i].doc_id << "," << docRankVec[i].rank << "} -";
            if(totalCount!=0)
                docRankVec[i].rank /= totalCount;
            std::cout << "{" << docRankVec[i].doc_id << "," << docRankVec[i].rank << "} ;;;;;;"; 
        };
        std::cout << std::endl;
        vec.push_back(docRankVec);
    };
    

    return vec;
    
};

