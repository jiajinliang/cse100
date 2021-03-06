/**
 * File:	ActorGraph.cpp
 * Author:	Tianheng Liu, Jiajing Liang
 * Date:	Mar 15, 2018
 * Description:
 *	This file is meant to exist as a container for starter code that you can
 *	use to read the input file format defined in movie_casts.tsv. Feel free
 *	to modify any/all aspects as you wish.
 */


#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <queue>
#include <ctime>
#include <string>
#include <string.h>
#include <stdio.h>
#include "ActorGraph.hpp"
using namespace std;

static const char COMPUTE[] = "Computing path for (";
static const char IN[] = "--[";
static const char TO[] = ") -> (";
static const char WITH[] = "]-->(";
static const char FAILED_LOCATE[] = "Failed to locate '";
static const char OUTPUT_HEADER[] = "(actor)--[movie#@year]-->(actor)--...";
static const char NUM_OF_ACT[] = "#nodes: ";
static const char NUM_OF_MV[] = "#movies: ";
static const char NUM_OF_ED[] = "#edges: ";
static const char DONE[] = "done\n";

struct order {
	bool operator()(actor* act1, actor* act2) {
		return *act1 < *act2;
	}
};

struct order2 {
	bool operator()(Pair* pr1, Pair* pr2) {
		return *pr1 < *pr2;
	}
};

struct order3 {
	bool operator()(actor* act1, actor* act2) {
		return act1->fdnum < act2->fdnum;
	}
};

struct order4 {
	bool operator()(actor* a1,actor* a2) {
		if(strcmp(a1->actor_name.c_str(),a2->actor_name.c_str())<0){
			return true;
		}
		else return false;
	}
};

void output(string path, string content) {
	// append the content to the output file
	ofstream outfile(path, ios::app);
	outfile << content << endl;
	outfile.close();
}

ActorGraph::ActorGraph(void) {}

ActorGraph::~ActorGraph(void) {
	unordered_map<string, actor*>::iterator act_it;
	unordered_map<string, movie*>::iterator mv_it;
	
	act_it = actors.begin();
	for( ; act_it != actors.end(); act_it++) {
		delete act_it->second;
	}

	mv_it = movies.begin();
	for( ; mv_it != movies.end(); mv_it++) {
		delete mv_it->second;
	}
}

int ActorGraph::edgeCount() {
	unordered_map<string, movie*>::iterator mv_it;	// movie iterator
	int act_count;					// actor count
	int edge_count;					// edge count

	mv_it = movies.begin();
	edge_count = 0;
	for( ; mv_it != movies.end(); mv_it++) {
		act_count = mv_it->second->actors.size();
		edge_count += act_count * (act_count - 1);
	}

	return edge_count;
}

// build the graph
bool ActorGraph::loadFromFile(const char* in_filename, bool weighted) {
	// Initialize the file stream
	ifstream infile(in_filename);
	actor* this_actor;			// actor being inserted
	movie* this_movie;			// movie being inserted
	unordered_map<string, actor*>::iterator actor_it;	// act iterator
	unordered_map<string, movie*>::iterator movie_it;	// mv iterator
	int act_count = 0;			// number of actors
	int mv_count = 0;			// number of movies
	bool have_header = false;

	// keep reading lines until the end of file is reached
	while (infile) {
		string s;

		// get the next line
		if (!getline( infile, s )) break;

		if (!have_header) {
			// skip the header
			have_header = true;
			continue;
		}

		istringstream ss( s );
		vector <string> record;

		while (ss) {
			string next;

			// get the next string before hitting a tab character
			// and put it in 'next'
			if (!getline( ss, next, '\t' )) break;

			record.push_back( next );
		}

		if (record.size() != 3) {
			// we should have exactly 3 columns
			continue;
		}

		string actor_name(record[0]);
		string movie_title(record[1]);
		int movie_year = stoi(record[2]);
		string movie_name = movie_title + "#@" + record[2];


		// if actor not found, create a new actor node
		actor_it = actors.find(actor_name);
		if(actor_it == actors.end()) {
			this_actor = new actor(actor_name);
			actors.insert(make_pair(actor_name, this_actor));
			act_count++;

		// if acotr found, go to this actor
		} else {
			this_actor = actor_it->second;
		}

		// if movie not found, create a new movie node
		movie_it = movies.find(movie_name);
		if(movie_it == movies.end()) {
			this_movie = new movie(movie_name, movie_year,
				weighted);
			movies.insert(make_pair(movie_name, this_movie));
			mv_count++;

		// if movie found, go to this movie
		} else {
			this_movie = movie_it->second;
		}

		// add the movie to the actor node
		this_actor->addMovie(this_movie);

		// add the actor to the movie node
		this_movie->addActor(this_actor);

	}

	if (!infile.eof()) {
		cerr << "Failed to read " << in_filename << "!\n";
		return false;
	}
	infile.close();

	// stdout message
	cout << NUM_OF_ACT << act_count << endl;
	cout << NUM_OF_MV << mv_count << endl;
	cout << NUM_OF_ED << edgeCount() << endl;
	cout << DONE;

	return true;
}

actor* ActorGraph::firstActor(string name) {
	unordered_map<string, actor*>::iterator found;
						// iterator to the first actor

	found = actors.find(name);
	if(found == actors.end()) {
		return nullptr;
	}

	return found->second;
}

//`trace back
string ActorGraph::traceBack(actor* this_actor, string actor1) {
	string result;				// return path

	// trace back to actor1
	result = "";
	while(this_actor->actor_name != actor1) {
		// write down the path
		result = IN + this_actor->from_movie->movie_name + WITH +
			this_actor->actor_name + ")" + result;

		// go to the previous actor
		this_actor = this_actor->from_movie->from_actor;
	}

	// add the actor1 to the result
	result = "(" + actor1 + ")" + result;

	return result;
}

//`reset graph
void ActorGraph::reset() {
	unordered_map<string, actor*>::iterator act;	// actor iterator
	unordered_map<string, movie*>::iterator mv;	// movie iterator

	// reset actors
	act = actors.begin();
	for( ; act != actors.end(); act++) {
		act->second->visited = false;
		act->second->cost = 0;
		act->second->from_movie = nullptr;
		//act->second->in_queue = false;
	}

	// reset movies
	mv = movies.begin();
	for( ; mv != movies.end(); mv++) {
		mv->second->visited = false;
		mv->second->from_actor = nullptr;
		mv->second->cost = 0;
	}
	
}

//`unweighted path finder
bool ActorGraph::uFindPath(string actor1, string actor2,
		const char* output_path) {
	actor* this_actor;			// pointer to actor1
	queue<actor*> queue;			// queue of the actors

	// find the first actor, mark done, add to the queue
	if(!(this_actor = firstActor(actor1))) {
		cerr << FAILED_LOCATE << "'" << actor1 << "'" << endl;
		return false;
	}
	this_actor->visited = true;
	queue.push(this_actor);

	// find the shortest path to the second actor
	while(!queue.empty()) {
		// pop the actor from the queue
		this_actor = queue.front();
		queue.pop();

		// add the actors related to this_actor to the queue
		// loop through movie list of this_actor
		for(movie* mv : this_actor->movies) {
			// if this movie has been visited, go to next one
			if(mv->visited) {
				continue;
			}
			
			// mark done the movie, assign this_actor to the movie
			mv->visited = true;
			mv->from_actor = this_actor;

			// loop through the actor list of the movie
			unordered_map<string, actor*>::iterator act_it;
			act_it = mv->actors.begin();
			for( ; act_it != mv->actors.end(); act_it++) {
				// if the actor has been visited, go to next one
				if(act_it->second->visited) {
					continue;
				}

				// mark done the actor, compute the cost,
					// assign the movie to the actor
				act_it->second->visited = true;
				act_it->second->from_movie = mv;

				// if actor2 found
				if(act_it->second->actor_name == actor2) {
					// output to the file
					output(output_path,
						traceBack(act_it->second,
							actor1));
					
					// reset the graph
					reset();

					return true;
				}

				// add the actor to the queue
				queue.push(act_it->second);
			}
		}
	}

	return false;
}

//`weighted path finder
bool ActorGraph::wFindPath(string actor1, string actor2,
		const char* output_path) {
	actor* this_actor;			// pointer to actor1
	int temp_cost;				// temperary cost
	priority_queue<actor*, vector<actor*>, order> queue;
						// queue of the actors

	// find the first actor, add to the queue
	if(!(this_actor = firstActor(actor1))) {
		cerr << FAILED_LOCATE << "'" << actor1 << "'" << endl;
		return false;
	}
	queue.push(this_actor);

	// find the shortest path to the second actor
	while(!queue.empty()) {
		// pop the queue
		this_actor = queue.top();
		queue.pop();
		
		// if the actor has been visited, go to the next one
		if(this_actor->visited) {
			continue;
		}

		// mark done this_actor
		this_actor->visited = true;

		// if actor2 found
		if(this_actor->actor_name == actor2) {
			// output to the file
			output(output_path, traceBack(this_actor, actor1));
			
			// reset the graph
			reset();

			return true;
		}

		// add the actor related to this_actor to the queue
		for(movie* mv : this_actor->movies) {
			// the cost for this_actor to get to next actor
			temp_cost = this_actor->cost + mv->weight;

			// if the movie has been visited, compare the cost
			if(mv->cost && temp_cost > mv->cost) {
				continue;
			}
			
			// update the cost, assign the actor to the moive
			mv->cost = temp_cost;
			mv->from_actor = this_actor;

			// loop through the actor list of the movie
			unordered_map<string, actor*>::iterator act_it;
			act_it = mv->actors.begin();
			for( ; act_it != mv->actors.end(); act_it++) {
				// if the actor has been visited, go to next one
				if(act_it->second->visited) {
					continue;
				}

				// if the actor is in the queue
					// compare the cost
				if(act_it->second->cost && mv->cost
						> act_it->second->cost) {
					continue;
				}

				// update the cost
					// assign the movie to the actor
				act_it->second->cost = mv->cost;
				act_it->second->from_movie = mv;

				// add the actor to the queue
				queue.push(act_it->second);
			}
		}
	}

	return false;
}


//`path finder
bool ActorGraph::findPath(const char* input_path, const char* output_path,
		bool weighted) {
	string line;				// a line in the file
	bool status;				// return status
	ifstream infile(input_path);		// infile stream

	// add the header to the output file
	output(output_path, OUTPUT_HEADER);
	
	// skip the input file header
	if(!getline(infile, line)) {
		// TODO error message for empty input file
	}

	// read in a line and find the path between two actors
	while(infile) {
		if(!getline(infile, line)) {
			break;
		}

		istringstream ss(line);
		vector <string> record;

		// get the next string before hitting a tab character
		// and put it in 'next'
		while (ss) {
			string next;
			if (!getline( ss, next, '\t' )) {
				break;
			}

			record.push_back(next);
		}

		if(record.size() != 2) {
			continue;
		}

		// get the names of actor1 and actor2
		string actor1(record[0]);
		string actor2(record[1]);
		
		// stdout message
		cout << COMPUTE << actor1 << TO << actor2 << ")" << endl;

		// find the shortest path
		if(weighted) {
			status = wFindPath(actor1, actor2, output_path);
		} else {
			status = uFindPath(actor1, actor2, output_path);
		}

		if(!status) {
			return status;
		}
	}

	return true;
}

/*
bool ActorGraph::findFriend(actor* first_actor, actor* neighbour,
		const char* output_path1, const char* output_path2) {
	unordered_map<string, int> new_friends;
	unordered_map<string, int>::iterator nf_it;
	priority_queue<Pair*, vector<Pair*>, order2> queue;
	unordered_map<string, actor*>::iterator act_it, found;

	for(movie* mv : neighbour->movies) {
		act_it = mv->actors.begin();
		for( ; act_it != mv->actors.end(); act_it++) {
			if(act_it->second->visited == false) {
				act_it->second->visited = true;
				Pair(act_it->second->actor_name,0) a;
				for(movie* mo : act_it->second->movies) {
					found = actors.find(first_actor->actor_name);
					if(found != mv->actors.end()) {
						a.common ++;
					}
					else {
						nf_it =
						new_friends.find(act_it->second->actor_name);
						if(nf_it == new_friends.end()) {
							new_friends[act_it->second->actor_name]
							= 0;
						}
						else {
							new_friends[act_it->second->actor_name]
							++;
						}
					}
				}
			}
		}
	}
	sort(new_friends.begin(), new_friends.end(), 
}
*/

/*
bool ActorGraph::linkFinder(const char* input_path, const char* output_path1,
		const char* output_path2) {
	string actor_name;
	actor* first_actor;
	actor* this_actor;
	ifstream infile(input_path);
	unordered_map<string, actor*>::iterator act_it;

	getline(infile, actor_name);

	while(infile) {
		if(!getline(infile, actor_name)) {
			break;
		}

		cout << COMPUTE << actor << endl;
		
		first_actor = firstActor(actor_name);
		first_actor->visited = true;
		
		for(movie* mv : first_actor->movies) {
			act_it = mv->actors.begin();
			for( ; act_it != mv->actors.end(); act_it++) {
				findFriend(first_actor, act_it->second,
					output_path1, output_path2);
			}
		}
		reset();
	}

}
*/

bool ActorGraph::findkcore(int k, const char* output_path) {
	actor* cur;
	priority_queue<actor*, vector<actor*>, order3> queue;
	unordered_map<string,actor*>::iterator act_it = actors.begin();
	for(;act_it != actors.end();act_it++) {
		for(movie* mv : act_it->second->movies){
			act_it->second->fdnum += mv->actors.size();
		}
		queue.push(act_it->second);
	}
	while(queue.top()->fdnum <= k) {
		cur = queue.top();
		queue.pop();
		cur->going = false;
		for(movie* m : cur->movies) {
			act_it = m->actors.begin();
			for(; act_it != m->actors.end(); act_it++) {
				if(!act_it->second->visited) {
					act_it->second->visited = true;
					act_it->second->fdnum --;
					queue.push(act_it->second);
				}
			}
		}
	}
	priority_queue<actor*, vector<actor*>, order4> queue2;
	act_it = actors.begin();
	for(;act_it != actors.end(); act_it++) {
		if(act_it->second->going){
			queue2.push(act_it->second);
		}
	}
	while(!queue2.empty()){
		ofstream outfile(output_path, ios::app);
		outfile << queue2.top()->actor_name << endl;
		queue2.pop();
	}
}
