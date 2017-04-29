//
//  tfidf.cpp
//  prahvi
//
//  Created by Yang Li on 4/29/17.
//  Copyright © 2017 Portable Reading Assistant Headset for the Visually Impaired. All rights reserved.
//
//  Description: module for genrating the TF-IDF score
//		Uses the boost/tokenizer to tokenize the string of text

#include <boost/tokenizer.hpp>
#include <cmath>
#include <iostream>
#include "tfidf.hpp"

//  Function: tfidf::tfidf
//  Description: constructor for tfidf
//		sets the total terms counter to 0
//		load idf score
tfidf::tfidf()
{
	_totalTerms=0;
	// TODO
	// load idf score
	
	// generate the average IDF
	_averageIDF = 0.0;
	int averageDocumentTerm = 0;
	
	for(auto const &it : _documentTerms) {
		averageDocumentTerm += it.second;
	}
	
	averageDocumentTerm /= _documentTerms.size();
	_averageIDF = log(_totalDocuments/(double) averageDocumentTerm);
}

//  Function: tfidf::get_tf
//  Description: get the term frequency score of the term
//		tf = number of times the term appears in the text / total numbers of terms in the text
double tfidf::getTF(std::string term)
{
	return _terms[term]/ (double) _totalTerms;
}

//  Function: tfidf::get_idf
//  Description: get the inverser document frequency score of the term
//		idf = ln(total number of documents / number of documents with the term in it)
//		if the idf of a term cannot be found, the average idf will be returned
double tfidf::getIDF(std::string term)
{
	std::map<std::string,int>::iterator it = _documentTerms.find(term);
	
	if(it != _documentTerms.end())
	{
		return log(_totalDocuments/(double) _documentTerms[term]);
	}
	
	return _averageIDF;
}

//	Function: tfidf::getTFIDF
//	Description: get the TFIDF score for the term
//		TFIDF = TF * IDF
double tfidf::getTFIDF(std::string term)
{
	return getTF(term)*getIDF(term);
}

//	Fucntion: tfidf::setTerms
//	Description: reset the current TF scores and terms
//		and generate new TF scores for the string of new terms
bool tfidf::setTerms(std::string newTerms)
{
	resetTerms();
	addTerm(newTerms);
	return true;
}

//	Function: tfidf::addTerms
//	Description: add the string of new terms to the tf score
bool tfidf::addTerms(std::string newTerms)
{
	boost::tokenizer<> token(newTerms);
	
	for(boost::tokenizer<>::iterator begin = token.begin(); begin != token.end(); begin++){
		addTerm(*begin);
	}
	
	return true;
}

//	Function: tfidf::addTerm
//	Description: add the new term to the tf score
//		rank the term with the others after the tfidf score is generated for the term
bool tfidf::addTerm(std::string newTerm)
{
	_totalTerms++;
	_terms[newTerm] = _terms[newTerm]+1;
	
	//	if the new term never seen before
	if(_terms[newTerm] == 1)
	{
		//	if this is the first term -> never did any ranking
		//	just push the term into the ranking
		if(_ranking.size() == 0)
		{
			_ranking.push_back(newTerm);
		}
		//	if there are already terms in the ranking
		//	start from the lowest rank
		//	move up if the current term has a higher score than the previous term
		else
		{
			int index = _ranking.size();
			
			while(this->getTFIDF(_ranking[index-1]) < getTFIDF(newTerm) && index != 0)
			{
				index--;
			}
			
			_ranking.insert(_ranking.begin()+index, newTerm);
		}
	}
	//	if the new term has seen before
	//	start from the current ranking of the term
	//	move up if the current term has a higher score than the previous term
	else
	{
		//	find the ranking location
		ptrdiff_t index = find(_ranking.begin(), _ranking.end(), newTerm) - _ranking.begin();
		
		//	remove the term from ranking
		_ranking.erase(_ranking.begin()+index);
		
		//	find new location
		while(this->getTFIDF(_ranking[index-1]) < getTFIDF(newTerm) && index != 0)
		{
			index--;
		}
		
		//	insert at new location
		_ranking.insert(_ranking.begin()+index, newTerm);
	}
	return true;
}

//	Function: tfidf::resetTerms
//	Description: remove all TF score and terms
bool tfidf::resetTerms()
{
	_totalTerms = 0;
	_terms.clear();
	_ranking.clear();
	return true;
}

//	Function: tfidf::getTerm
//	Description: get the term based on the rank received
//		Will return an empty string if rank received is greater than the number of terms
std::string tfidf::getTerm(int rank)
{
	
	//	if the rank value received is reater than the number of terms
	//	return an empty string
	if(rank > _ranking.size())
	{
		return "";
	}
	return _ranking[rank-1];
}

//	Function: tfidf::printRanking
//	Description: print the list of ranking in console
void tfidf::printRanking()
{
	for(int i=0; i< _ranking.size(); i++)
	{
		std::cout << i+1 <<": "
					<< _ranking[i] << "\t"
					<< _terms[_ranking[i]] << "\t"
					<< getTFIDF(_ranking[i])
					<< std::endl;
	}
}
