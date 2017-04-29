//
//  tfidf.hpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//  Description: header file for TF-IDF class
//		allow other program to get the tfidf score of a text
//		and to get the ranked terms

#ifndef tfidf_hpp
#define tfidf_hpp

#include <map>

class tfidf
{
public:
	tfidf();
	double getTF(std::string term);
	double getIDF(std::string term);
	double getTFIDF(std::string term);
	bool setTerms(std::string newTerms);
	bool addTerms(std::string newTerms);
	bool resetTerms();
	bool addTerm(std::string newTerm);
	void printRanking();
	std::string getTerm(int rank = 1);
private:
	int _totalTerms;
	int _totalDocuments;
	std::map<std::string, int> _documentTerms;
	double _averageIDF;
	std::map<std::string, int> _terms;
	std::vector<std::string> _ranking;
};


#endif /* tfidf_hpp */
