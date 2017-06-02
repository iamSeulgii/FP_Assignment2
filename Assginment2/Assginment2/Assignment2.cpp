#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <vector>
#include <set>
#include <map>
#include <conio.h>
#include <math.h>
#include "Tokenizer.h"


#define blockSize 4096

#define NODESIZE 5
#define EPS 0.0001

#define BUCKETSIZE 3
#define INIT_GLOB_DEPTH 0
#define SHOW_DUPLICATE_BUCKETS 0

using namespace std;



typedef struct _students {

	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;

}Students;


typedef struct _block {

	Students records[128];

}Block;


class Bucket {

private:
	int depth, size;
	map<int, int> values;

public:

	Bucket(int depth, int size) {
		this->depth = depth;
		this->size = size;
	}

	int insert(int key, int value) {
		map<int, int>::iterator it;
		it = values.find(key);
		if (it != values.end())
			return -1;
		if (isFull())
			return 0;
		values[key] = value;
		return 1;
	}

	bool search(int key) {
		map<int, int>::iterator it;
		it = values.find(key);
		if (it != values.end())
			return true;
		else    return false;
	}

	int isFull(void) {
		if (values.size() == size)
			return 1;
		else
			return 0;
	}

	int isEmpty(void) {
		if (values.size() == 0)
			return 1;
		else
			return 0;
	}

	int getDepth(void) {
		return depth;
	}

	int increaseDepth(void) {
		depth++;
		return depth;
	}

	int decreaseDepth(void) {
		depth--;
		return depth;
	}

	map<int, int> copy(void) {
		map<int, int> temp(values.begin(), values.end());
		return temp;
	}


	void clear(void) {
		values.clear();
	}


	void display(void) {
		map<int, int>::iterator it;
		for (it = values.begin(); it != values.end(); it++)
			cout << it->first << "(" << it->second << ") ";
		cout << endl;
	}


	int writeHashFile(FILE * fout) {
		if (fwrite(this, 4096, 1, fout) == -1)
			return -1;
		return 0;
	}

};


class Directory {
	int global_depth, bucket_size;
	vector<Bucket*> buckets;

	int hash(int n) {
		return n&((1 << global_depth) - 1);
	}

	int pairIndex(int bucket_no, int depth) {
		return bucket_no ^ (1 << (depth - 1));
	}


	void grow(void) {
		for (int i = 0; i < 1 << global_depth; i++)
			buckets.push_back(buckets[i]);
		global_depth++;
	}

	void shrink(void) {
		int flag = 1, i;
		for (i = 0; i < buckets.size(); i++) {
			if (buckets[i]->getDepth() == global_depth) {
				flag = 0;
				return;
			}
		}
		global_depth--;
		for (i = 0; i < 1 << global_depth; i++)
			buckets.pop_back();
	}

	void split(int bucket_no) {
		int local_depth, pair_index, index_diff, dir_size, i;
		map<int, int> temp;
		map<int, int>::iterator it;

		local_depth = buckets[bucket_no]->increaseDepth();
		if (local_depth>global_depth)
			grow();
		pair_index = pairIndex(bucket_no, local_depth);
		buckets[pair_index] = new Bucket(local_depth, bucket_size);
		temp = buckets[bucket_no]->copy();
		buckets[bucket_no]->clear();
		index_diff = 1 << local_depth;
		dir_size = 1 << global_depth;
		for (i = pair_index - index_diff; i >= 0; i -= index_diff)
			buckets[i] = buckets[pair_index];
		for (i = pair_index + index_diff; i<dir_size; i += index_diff)
			buckets[i] = buckets[pair_index];
		for (it = temp.begin(); it != temp.end(); it++)
			insert((*it).first, (*it).second, 1);
	}


	void merge(int bucket_no) {
		int local_depth, pair_index, index_diff, dir_size, i;

		local_depth = buckets[bucket_no]->getDepth();
		pair_index = pairIndex(bucket_no, local_depth);
		index_diff = 1 << local_depth;
		dir_size = 1 << global_depth;

		if (buckets[pair_index]->getDepth() == local_depth) {
			buckets[pair_index]->decreaseDepth();
			delete(buckets[bucket_no]);
			buckets[bucket_no] = buckets[pair_index];
			for (i = bucket_no - index_diff; i >= 0; i -= index_diff)
				buckets[i] = buckets[pair_index];
			for (i = bucket_no + index_diff; i<dir_size; i += index_diff)
				buckets[i] = buckets[pair_index];
		}
	}


	string bucket_id(int n) {
		int d;
		string s;
		d = buckets[n]->getDepth();
		s = "";
		while (n>0 && d>0) {
			s = (n % 2 == 0 ? "0" : "1") + s;
			n /= 2;
			d--;
		}
		while (d>0) {
			s = "0" + s;
			d--;
		}
		return s;
	}

public:

	Directory(int depth, int bucket_size) {
		this->global_depth = depth;
		this->bucket_size = bucket_size;
		for (int i = 0; i < 1 << depth; i++) {
			buckets.push_back(new Bucket(depth, bucket_size));
		}
	}

	void insert(int key, int value, bool reinserted) {
		int bucket_no = hash(key);
		int status = buckets[bucket_no]->insert(key, value);
		if (status == 0) {
			split(bucket_no);
			insert(key, value, reinserted);
		}
	}

	bool search(int key) {
		int bucket_no = hash(key);
		return buckets[bucket_no]->search(key);
	}


	void display(bool duplicates) {
		int i, j, d;
		string s;
		set<string> shown;
		for (i = 0; i<buckets.size(); i++) {
			d = buckets[i]->getDepth();
			s = bucket_id(i);
			if (duplicates || shown.find(s) == shown.end()) {
				shown.insert(s);
				for (j = d; j <= global_depth; j++)
					cout << " ";
				cout << s << " ~~~> ";
				buckets[i]->display();
			}
		}

	}

	int writeHashFile(FILE * fout) {
		int i, j, d;
		string s;
		set<string> shown;

		for (i = 0; i<buckets.size(); i++) {
			d = buckets[i]->getDepth();
			s = bucket_id(i);
			if (shown.find(s) == shown.end()) {
				shown.insert(s);

				if (fwrite(&s, 4096, 1, fout) == -1)
					return -1;
				if (buckets[i]->writeHashFile(fout) == -1)
					return -1;
			}
		}
		return 0;
	}


};



//Funtion which gets inputData from file
int getInputData(Block*& blocks, string input_str) {



	//Get input data from .csv
	ifstream input_data(input_str.c_str());
	string buf;
	Tokenizer tokenizer; //include "Tokenize.h"
	int numOfRecords = blockSize / sizeof(Students);
	tokenizer.setDelimiter(","); //parsing Delimiter = ","
	getline(input_data, buf);
	tokenizer.setString(buf);
	int count = atoi(tokenizer.next().c_str()); //the num of Students

	blocks = new Block[count / numOfRecords + 1];

	for (int j = 0; j < count / numOfRecords + 1; j++) {
		for (int i = 0; i < numOfRecords; i++) {
			strcpy(blocks[j].records[i].name, "");
			blocks[j].records[i].studentID = 0;
			blocks[j].records[i].score = 0;
			blocks[j].records[i].advisorID = 0;
		}
	}


	for (int j = 0; j < count / numOfRecords + 1; j++) {


		for (int i = 0; i < numOfRecords; i++) {

			string temp;
			getline(input_data, buf);
			tokenizer.setString(buf);
			if (input_data.eof()) {
				break;
			}


			strncpy(blocks[j].records[i].name, tokenizer.next().c_str(), sizeof(blocks[j].records[i].name));
			blocks[j].records[i].studentID = atoi(tokenizer.next().c_str());
			blocks[j].records[i].score = atof(tokenizer.next().c_str());
			blocks[j].records[i].advisorID = atoi(tokenizer.next().c_str());
			/*cout << blocks[j].records[i].name << " " << blocks[j].records[i].studentID << " " <<
			blocks[j].records[i].score << " " << blocks[j].records[i].advisorID << endl;*/
		}

	}



	return count;
}


void insertDB(Block*& blocks, int count) {
	int numOfRecords = blockSize / sizeof(Students);
	FILE *DBFile = fopen("Students.DB", "wb");
	fseek(DBFile, 0, SEEK_SET);
	fwrite((char*)blocks, sizeof(Block), count / numOfRecords + 1, DBFile);
}


int main() {

	Students *students;
	Block *readBlocks, *writeBlocks;


	int count = getInputData(writeBlocks, "sampleData.csv");
	insertDB(writeBlocks, count);


	int numOfRecords = blockSize / sizeof(Students);
	Directory directory(INIT_GLOB_DEPTH, BUCKETSIZE);


	FILE *readDB = fopen("Students.DB", "rb");
	fseek(readDB, 0, SEEK_SET);

	readBlocks = new Block[count / numOfRecords + 1];
	fread((void*)readBlocks, sizeof(Block), count / numOfRecords + 1, readDB);


	for (int j = 0; j < count / numOfRecords + 1; j++) {

		for (int i = 0; i < numOfRecords; i++) {

			directory.insert(readBlocks[j].records[i].studentID, j, 0);

		}
	}

	FILE * hashFile = fopen("Students.hash", "wb");
	// make .hash file
	if (directory.writeHashFile(hashFile) == -1)
		cout << ".hash file error." << endl;


	directory.display(SHOW_DUPLICATE_BUCKETS);

	return 0;
}
