#include<iostream>
#include<string>
#include<fstream>
#include<exception>
#include<filesystem>
#include "nlohmann/json.hpp"

#include"classes.h"

int main() {

    std::cout << "\nSearch engine\n\n";

    std::vector<std::string> paths, requests;
    int responsesLimits = 5;

    ConverterJSON newConverter;
    try
    {
        paths = newConverter.GetTextDocuments();
        responsesLimits = newConverter.GetResponsesLimit();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    };

    
    for(auto i : paths)
    {
        std::cout << i << " " << std::filesystem::exists(i) << ". Responses limits: " << responsesLimits << std::endl;
    };

    requests = newConverter.GetRequests();

    std::vector<std::vector<RelativeIndex>> answers;
    int countI = 0;
    for(auto i : requests)
    {
        std::cout << i << std::endl;

        std::vector<RelativeIndex> answer;
        int countJ = 0;
        for(auto j : paths)
        {
            RelativeIndex relativeIndex;
            if(countI==0)
            {
                if(countJ==0)
                {
                    relativeIndex.rank = 0.989;
                } else if(countJ==1)
                {
                    relativeIndex.rank = 0.856;
                } else if(countJ==2)
                {
                    relativeIndex.rank = 0.750;
                } else if(countJ==3)
                {
                    relativeIndex.rank = 0.630;
                };
            } else if(countI==1)
            {
                if(countJ==1)
                {
                    relativeIndex.rank = 0.769;
                };
            } else if(countI==2)
            {
                //
            };
            relativeIndex.doc_id = countJ;
            answer.push_back(relativeIndex);
            countJ++;
        };
        countI++;
        answers.push_back(answer);
    };
    
    newConverter.putAnswers(answers);

    InvertedIndex newBase;
    newBase.UpdateDocumentBase(paths);

    SearchServer searchServer(newBase);

    answers = searchServer.search(requests);
    newConverter.putAnswers(answers);

}