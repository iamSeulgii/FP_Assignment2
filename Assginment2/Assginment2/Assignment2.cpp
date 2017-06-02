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

//Funtion which gets inputData from file
int getInputData( Block*& blocks, string input_str) {



	//Get input data from .csv
	ifstream input_data(input_str.c_str());
	string buf;
	Tokenizer tokenizer; //include "Tokenize.h"
	int numOfRecords = blockSize / sizeof(Students) ;
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
	fwrite((char*)blocks, sizeof(Block), count/numOfRecords +1, DBFile);
}


int main() {

	Students *students;
	Block* blocks;

	
	int count = getInputData(blocks, "sampleData.csv");
	insertDB(blocks, count);

	int numOfRecords = blockSize / sizeof(Students);
	FILE *readDB = fopen("Students.DB", "rb");
	fseek(readDB, 0, SEEK_SET);


	Block* tmp = new Block[count / numOfRecords + 1];
	fread((void*)tmp, sizeof(Block), count / numOfRecords + 1, readDB);
	


	for (int j = 0; j < count / numOfRecords + 1; j++) {

		for (int i = 0; i < numOfRecords; i++) {
			if(strcmp(tmp[j].records[i].name, ""))
				cout << tmp[j].records[i].name <<tmp[j].records[i].studentID<< endl;
		}
	}
		
	return 0;

}