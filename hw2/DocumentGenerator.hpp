/**
 * CSE 100 PA2 SpeechBuilder
 * Author: Jonathan Margoliash
 */

#ifndef DOCUMENT_GENERATOR_HPP
#define DOCUMENT_GENERATOR_HPP

#include <string>
#include <dirent.h>
#include <string.h>
#include <ctime>
#include "AutocompleteUtils.hpp"
#include "MWT.hpp"

using namespace std;

class DocumentGenerator
{
public:

  
 /**
 * Builds a document generator from the documents in the given directory
 * This should open all files in the directory, reads them in, tokenize them into words,
 * and build the datastructure from that stream of words.
 *
 * To tokenize a document:
 * * remove all UNWANTED_CHARACTERS from the document
 * * split it into different tokens based on whitespace
 *  (treat all whitespace breaks equally - newlines and spaces should be treated the same way)
 * * additionally split each PUNCTUATION character into its own token (so a single period is a 'word')
 *   (equivalently - treat all punctuation as if it was surrounded on both sides by a space)
 * * characters that are neither UNWANTED_CHARACTERS nor PUNCTUATION should be treated normally, the same way as any alphabetic character
 * * from here on, we'll use the word "word" to refer to both strings like "hello" and punctuation strings like "." or ","
 *
 * So, for instance, the short sentence
 * "I like trains. Sally ju^mped over the moon, I think? I. I think."
 * Would be tokenized into ["I", "like", "trains", ".", "Sally",
 *   "jumped", "over", "the", "moon", ",", "I", "think", "?", "I", ".", "I" "think", "."]
 * and the frequencies of the words after "I" would be;
 *  like - 1
 *  think - 2
 *  . - 1
 *
 * You may use the utilities in AutocompleteUtils to help you with this
 * and you may rely on them working.
 * Note however that they do not deal with unwanted characters or split punctuation appropriately.
 * 
 * A few notes:
 * 1) you must treat words with different capitalizatoins differently
 * * (so "foo" and "Foo" are different words, and the frequencies of the words that follow (or precede)
 * * "foo" will be different than the frequencies of the words that follow (or precede) "Foo" )
 * 2) pretend that the first word in each document is preceeded by a periood (That way, it is considered when starting any new sentence)
 */
  DocumentGenerator(const string documentsDirectory) {
		tree = new MWT();			// tree for storing the document
		unsigned long pos;		// position in the document
		string first;					// a word in the document
		string second;				// the word to the next of the first
		//ifstream file;
		vector<string> content;				// content in a file
		DIR* dir;
		struct dirent* entry;
		string b;

		const char* path = &documentsDirectory[0];
		dir = opendir(path);

		srand(time(NULL));

		while((entry = readdir(dir))) {
			if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				string a = documentsDirectory + "/" + (string)entry->d_name;
				content = AutocompleteUtils::getWords(a);
			}

			b = "";
			for(auto a: content) {
				b += a + " "; 
			}

			pos = 0;
			first = getaword(&pos, &b);
			do {
				second = getaword(&pos, &b);
				tree->insert(first, second);
				first = second;
			} while(second != "");
		}
		closedir(dir);
  }

	string getaword(unsigned long* pos, const string* doc) {
		string result;				// the return value
		char character;				// a character in the document
		bool skip;						// skip this character?
		bool punc;						// a punctuation?

		// loop through the document char by char
		result = "";
		while(*pos < doc->length()) {
			character = doc->at(*pos);
			skip = false;
			punc = false;

			// check if the character is a unwanted char
			for(auto a: UNWANTED_CHARACTERS) {
				if(character == a) {
					skip = true;
					break;
				}
			}

			// skip if the character is a unwanted char
			if(skip) {
				(*pos)++;
				continue;
			}

			// check if the character is a puncuation
			for(auto a: PUNCTUATION) {
				if(character == a) {
					punc = true;
					break;
				}
			}

			// if the character is a punctuation return the result if there is already
			// something in there otherwise, return the punctuation
			if(punc) {
				if(result == "") {
					result += character;
					(*pos)++;
					return result;
				}

				return result;
			}

			// if the character is a space, return the result
			if(character == ' ') {
				(*pos)++;
				if(result != "") {
					return result;
				}
				continue;
			}

			// add the character to the word, go to the next character
			result += character;
			(*pos)++;
		}

		return result;
	}
  /**
 * Suppose you're in the middle of generating a document and that you've just added the word prevWord
 * to the document. generateNextWord(prevWord) should generate the next word at random for the document,
 * according to the frequencies of all the words that followed prevWord in the input documents
 *
 * So, for instance, if in all the input documents the word "foo" was followed by the word "bar" twice and "baz" 8 times,
 * then 80% of the time generateNextWord("foo") should return "baz" and 20% of the time you should return "bar".
 * 
 * This method should return punctuation words like "." and "," as if they were any other word.
 * Likewise, it should be able to take punctuation words as input (generateNextWord(","))
 * 
 * You can assume that prevWord is always a word that's present in one of the documents you've read in.
 */
  string generateNextWord(const string prevWord) {
		return tree->nextWord(prevWord);
  }

  /**
 * Generate a document with numWords words in it.
 *
 * See the docs on generateNextWord for a description of what it means to generate a word.
 * After generating all the words - concatenate them all into a single string representing the entire document, and return that string.
 *
 * Notes:
 * The first word you generate should be as if the previous word was '.'
 * Your document should not contain whitespace except for spaces.
 * Your should insert a single space in front of each word except:
 * * Don't insert spaces in front of punctuation marks
 * * Don't insert a space at the start of the document
 * Punctuation "words" count against this word total.
 *
 * The document will likely not end at the end of a sentence. That's okay.
 */
  string generateDocument(const int numWords) {
		string result = "";
		string preWord = ".";
		string nextWord;
		bool skip;
		result += generateNextWord(preWord);
		preWord = result;
		for(int i = 1; i < numWords; i++){
			skip = false;
			nextWord = generateNextWord(preWord);
			for(auto a : PUNCTUATION){
				if(nextWord[0] == a){
					skip = true;
				}
			}
			if(!skip){
				result += " ";
			}
			result += nextWord;
			preWord = nextWord;
		}
    return result;//TODO
  }

  ~DocumentGenerator() {
		delete tree;
    //TODO
  }
  
  //Feel free to put your own code here

private:
  const string PUNCTUATION = ".!,?";
  const string UNWANTED_CHARACTERS = ";:\"~()[]{}\\/^_<>*=&%@$+|`";

	MWT* tree;

  //Feel free to put your own code here
};

#endif //DOCUMENT_GENERATOR_HPP

