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

	Students records[blockSize / sizeof(Students)];

}Block;


typedef struct _hashMap {

	int key;
	int tableNum;

}HashMap;


class Bucket {

private:
	int hashPrefix, size;
	map<int, int> hashTable;

public:

	Bucket(int hashPrefix, int size) {
		this->hashPrefix = hashPrefix;
		this->size = size;
	}

	int insert(int _key, int _blockNum) {
		map<int, int>::iterator it;
		it = hashTable.find(_key);
		if (it != hashTable.end()) return -1;
		else if (isFull()) return 0;
		else {
			hashTable[_key] = _blockNum;
			return 1;
		}
	}

	bool search(int key) {
		map<int, int>::iterator it;
		it = hashTable.find(key);
		if (it != hashTable.end())
			return true;
		else    return false;
	}

	int isFull(void) {
		if (hashTable.size() == size)
			return 1;
		else
			return 0;
	}

	int isEmpty(void) {
		if (hashTable.size() == 0)
			return 1;
		else
			return 0;
	}

	int gethashTableize() {

		return hashTable.size();
	}

	int getHashPrefix(void) {
		return hashPrefix;
	}

	int increaseHashPrefix(void) {
		hashPrefix++;
		return hashPrefix;
	}

	int decreaseHashPrefix(void) {
		hashPrefix--;
		return hashPrefix;
	}

	map<int, int> copy(void) {
		map<int, int> temp(hashTable.begin(), hashTable.end());
		return temp;
	}


	void clear(void) {
		hashTable.clear();
	}

	//insert values into Student.hash as binary format

	int writeHashFile(FILE *& fout) {

		map<int, int>::iterator it;
		HashMap* hashMap = new HashMap[hashTable.size()];
		int i = 0;
		for (it = hashTable.begin(); it != hashTable.end(); it++) {
	
			hashMap[i].key = it->first;
			hashMap[i].tableNum = it->second;

			i++;
		}

		fwrite((void*)hashMap, sizeof(HashMap), hashTable.size(), fout);
		return 0;
	}

};


class Directory {

	int hashPrefix;
	vector<Bucket*> buckets;


	//증가된 hashPrefix맞게 버킷넘버 추가
	int pairIndex(int bucketNum, int hashPrefix) {
		return bucketNum ^ (1 << (hashPrefix - 1));
	}


	void grow(void) {
		for (int i = 0; i < 1 << hashPrefix; i++)
			buckets.push_back(buckets[i]);
		hashPrefix++;
	}

	void shrink(void) {
		int flag = 1, i;
		for (i = 0; i < buckets.size(); i++) {
			if (buckets[i]->getHashPrefix() == hashPrefix) {
				flag = 0;
				return;
			}
		}
		hashPrefix--;
		for (i = 0; i < 1 << hashPrefix; i++)
			buckets.pop_back();
	}


	void split(int bucketNum) {
		int local_depth, pair_index, index_diff, dir_size, i;
		map<int, int> temp;
		map<int, int>::iterator it;

		local_depth = buckets[bucketNum]->increaseHashPrefix();
		if (local_depth>hashPrefix)
			grow();
		pair_index = pairIndex(bucketNum, local_depth);
		buckets[pair_index] = new Bucket(local_depth, blockSize);
		temp = buckets[bucketNum]->copy();
		buckets[bucketNum]->clear();
		index_diff = 1 << local_depth;
		dir_size = 1 << hashPrefix;
		for (i = pair_index - index_diff; i >= 0; i -= index_diff)
			buckets[i] = buckets[pair_index];
		for (i = pair_index + index_diff; i<dir_size; i += index_diff)
			buckets[i] = buckets[pair_index];
		for (it = temp.begin(); it != temp.end(); it++)
			insert((*it).first, (*it).second, 1);
	}


	void merge(int bucketNum) {
		int local_hashPrefix, extendedBucketNum, index_diff, dir_size, i;

		local_hashPrefix = buckets[bucketNum]->getHashPrefix();
		extendedBucketNum = pairIndex(bucketNum, local_hashPrefix);
		index_diff = 1 << local_hashPrefix;
		dir_size = 1 << hashPrefix;

		if (buckets[extendedBucketNum]->getHashPrefix() == local_hashPrefix) {

			buckets[extendedBucketNum]->decreaseHashPrefix();
			delete(buckets[bucketNum]);
			buckets[bucketNum] = buckets[extendedBucketNum];
			for (i = bucketNum - index_diff; i >= 0; i -= index_diff)
				buckets[i] = buckets[extendedBucketNum];
			for (i = bucketNum + index_diff; i<dir_size; i += index_diff)
				buckets[i] = buckets[extendedBucketNum];
		}
	}


	string bucket_id(int n) {
		int d;
		string s;
		d = buckets[n]->getHashPrefix();
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

	Directory() {
		this->hashPrefix = 0;
		for (int i = 0; i < 1 << this->hashPrefix; i++) {
			buckets.push_back(new Bucket(this->hashPrefix, blockSize/sizeof(Students)));
		}
	}


	int hash(int n) {
		return n&((1 << hashPrefix) - 1);
	}

	void insert(int key, int bucketNum, bool reinserted) {

		int cmpIdx = 0;

		if (hashPrefix == 0) {

		}
		else {
			
			for (int i = 0; i < hashPrefix; i++)
				cmpIdx += pow(2, i);
				
		}
		int status = buckets[(key & cmpIdx)]->insert(key, bucketNum);

		if (status == 0) {
			split(bucketNum);
			insert(key, bucketNum, reinserted);
		}
	}

	bool search(int key) {
		int bucketNum = hash(key);
		return buckets[bucketNum]->search(key);
	}


	void display(bool duplicates) {


	}



	int writeHashFile(FILE *& fout, bool duplicates) {
		fseek(fout, 0, SEEK_SET);
		for (int i = 0; i<buckets.size(); i++)
			buckets[i]->writeHashFile(fout);
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


	//initialize blocks 
	for (int j = 0; j < count / numOfRecords + 1; j++) {
		for (int i = 0; i < numOfRecords; i++) {
			strcpy(blocks[j].records[i].name, "");
			blocks[j].records[i].studentID = 0;
			blocks[j].records[i].score = 0;
			blocks[j].records[i].advisorID = 0;
		}
	}



	//read inputFile and then put value into blocks
	for (int j = 0; j < count / numOfRecords + 1; j++) {
		for (int i = 0; i < numOfRecords; i++) {

			string temp;
			getline(input_data, buf);
			tokenizer.setString(buf);
			if (input_data.eof()) break;

			strncpy(blocks[j].records[i].name, tokenizer.next().c_str(), sizeof(blocks[j].records[i].name));
			blocks[j].records[i].studentID = atoi(tokenizer.next().c_str());
			blocks[j].records[i].score = atof(tokenizer.next().c_str());
			blocks[j].records[i].advisorID = atoi(tokenizer.next().c_str());
		}
	}

	return count;
}


void insertDB(Block*& blocks, int count) {
	//insert hashTable into DB as binary format
	int numOfRecords = blockSize / sizeof(Students);
	FILE *DBFile = fopen("Students.DB", "wb");
	fseek(DBFile, 0, SEEK_SET);
	fwrite((char*)blocks, sizeof(Block), count / numOfRecords + 1, DBFile);
}


void readHashFile(HashMap*& readHashMap, int count) {

	FILE *readHash = fopen("Students.hash", "rb");
	fseek(readHash, 0, SEEK_SET);
	fread((void*)readHashMap, sizeof(HashMap), count, readHash);

	//print out readHashMap
	for (int j = 0; j < count; j++) {
		cout << readHashMap[j].key << " " << readHashMap[j].tableNum << endl;
	}

}



int main() {

	int num;
	Students *students;
	Block *readBlocks, *writeBlocks;
	int count = getInputData(writeBlocks, "sampleData.csv");
	insertDB(writeBlocks, count);
	int numOfRecords = blockSize / sizeof(Students);

	Directory directory; //hash directory initialization

						 //read values from Students.DB
	FILE *readDB = fopen("Students.DB", "rb");
	fseek(readDB, 0, SEEK_SET);
	readBlocks = new Block[count / numOfRecords + 1];
	fread((void*)readBlocks, sizeof(Block), count / numOfRecords + 1, readDB);
	//studentID is key of Hash
	//insert key value into hash table
	for (int j = 0; j < count / numOfRecords + 1; j++) {
		for (int i = 0; i < numOfRecords; i++) {
			directory.insert(readBlocks[j].records[i].studentID, directory.hash(readBlocks[j].records[i].studentID), 0);
		}
	}

	//make Students.hash
	FILE *hashFile = fopen("Students.hash", "wb");
	if (directory.writeHashFile(hashFile, SHOW_DUPLICATE_BUCKETS) == -1)
		cout << ".hash file error." << endl;


	HashMap* readHashMap = new HashMap[count];

	while (1) {
		cout << "Select your operation\n 1.show Students.hash\n 2.show kth leaf of Students_score.idx\n 3.Show your DB\n 4.exit..\n>>>>>>";
		cin >> num;
		switch (num) {

		case 1:
			//read values from Students.hash
			readHashFile(readHashMap, count);
			break;
		case 2:
			cout << "...";

			break;
		case 3:
			for (int j = 0; j < count / numOfRecords + 1; j++) {
				for (int i = 0; i < numOfRecords; i++) {
					if (strcmp(readBlocks[j].records[i].name, ""))
						cout << readBlocks[j].records[i].name << " " << readBlocks[j].records[i].studentID << " "
						<< readBlocks[j].records[i].score << " " << readBlocks[j].records[i].advisorID << endl;
				}
			}
			break;
		case 4:
			return 0;
			break;
		default:
			cout << "Not valid operation number!\n";
		}
		cout << endl;

	}


	return 0;
}
