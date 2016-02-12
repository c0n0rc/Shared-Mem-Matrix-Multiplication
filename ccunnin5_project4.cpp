/*
Name: Conor Cunningham
Lab: Project 4
Compiling instructions: g++ -lpthreads ccunnin5_project4.cpp

NOTE: Can read matrices with elements that have more than 1 digit
Deviations from Design Document: 
Both parent and children must call shmat.
Output vector was replaced with output array. 
*/

#include <iostream>
#include <pthread.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

int multiply(int a, int b);

// Requests file name for program to use. Opens files and creates child processes.
int main(int argc, char const *argv[]) {

	ifstream fileInput;
	ofstream fileOutput;
	int ret;
	int multiplier   = 0;
	int multiplicand = 0;
	int ACol = 0;
	int ARow = 0;
	int BCol = 0;
	int BRow = 0;
	int output_size = 0;
	char input[255];							//buffers for scanf
	char output[255];							//buffers for scanf	
	vector< vector<int> > AMat;					//vector for first matrix
	vector< vector<int> > BMat;					//vector for second matrix

	printf("Enter the name of the input file (cannot exceed 255 chars): ");
	fgets(input, sizeof(input), stdin);
	size_t len = strlen(input) - 1; 			//removes newline from input
	if (input[len] == '\n') input[len] = '\0';

	printf("Enter the name of the output file (cannot exceed 255 chars): ");
	fgets(output, sizeof(output), stdin);
	len = strlen(output) - 1; 					//removes newline from input
	if (output[len] == '\n') output[len] = '\0';

	fileInput.open(input);
	fileOutput.open(output);

	if (!fileInput) {
		perror("Error opening input file for reading.");
		return 1;
	}

	if (!fileOutput) {
		perror("Error opening output file for reading.");
		return 1;
	}

	//read matrixes in from file
	string line;
	int new_matrix = 1;
	int element_count = 1;
	string temp_element;
	int element; 
	int skip = 0;
	stringstream linestream;
	while (getline(fileInput, line)) {
		vector<int> row;
    	linestream.str(line);
		while (linestream >> temp_element) {
			if (temp_element.at(0) == '*') {
				new_matrix = 0;
				skip = 1;
			}
			else {
				element = atoi(temp_element.c_str());
				row.push_back(element);
			}
		}
		if (!skip) {
			if (new_matrix) {
				AMat.push_back(row);
			} else {
				BMat.push_back(row);
			}
		}
		skip = 0;
		linestream.clear();	
	}

	ACol = AMat[0].size();
	ARow = AMat.size();
	BCol = BMat[0].size();
	BRow = BMat.size();

	//get output size
	for (int i = 0; i < ARow; i++) {
		for (int j = 0; j < BCol; j++) {
			for (int k = 0; k < BRow; k++) {
				output_size++;
			}
		}
	}

	int id = shmget(IPC_PRIVATE, output_size, 0666 | IPC_CREAT);
	if (id == -1) {
		cout << "Error in allocating memory with shmget()" << endl;
		fileInput.close();
		fileOutput.close();
		return 0;
	}

	int* OMat = (int*)shmat(id, NULL, 0);
	if (OMat == (int *)-1) {
		cout << "Error in attaching shared memory with shmat()" << endl;
		fileInput.close();
		fileOutput.close();
		return 0;
	}
	int num = 0;
	if (ACol == BRow) { 
		pid_t processes[ARow][BCol][BRow];
		int wait_status;
		for (int i = 0; i < ARow; i++) {
			for (int j = 0; j < BCol; j++) {
				for(int k = 0; k < BRow; k++){
		 			multiplier = AMat.at(i).at(k);
		 			multiplicand = BMat.at(k).at(j);
		 			processes[i][j][k] = fork();
		 			if (processes[i][j][k] == 0) {
						int* OMat = (int*)shmat(id, NULL, 0);
						if (OMat == (int*)-1) {
							cout << "Error in attaching shared memory with shmat()" << endl;
							fileInput.close();
							fileOutput.close();
							return 0;
						}
						OMat[num] = multiply(multiplier, multiplicand);
		 				exit(1);
		 			} else if (processes[i][j][k] < 0) {
						perror("fork");
						exit(1);
		 			}
					num++;
		 		}
		 	}
		 }
		int status;
		for (int i = 0; i < ARow; i++) {
			for (int j = 0; j < BCol; j++) {
				for(int k = 0; k < BRow; k++){
					pid_t pid = waitpid(processes[i][j][k], &status, WUNTRACED);
				 	// cout << "child with " << pid << "exited with status " << status << endl;
				}
			}
		}
	} else {
		cout << "Cannot perform the matrix multiplication. The columns in Matrix A are not equal to the rows in Matrix B." << endl;
		fileInput.close();
		fileOutput.close();
		return 0;
	}

	//create and initialize output matrix to all 0's
	int final_matrix[ARow * BCol];
	int index = 0;
	for (int i = 0; i < (ARow * BCol); i++) {
		final_matrix[i] = 0;
	}

	//add matrix multiplication elements
	for (int i = 0; i < output_size; i++) {
		final_matrix[index] = final_matrix[index] + OMat[i];
		if ((i+1) % ACol == 0) index++;
	}

	//print final output matrix
	for (int i = 0; i < (ARow * BCol); i++) {
		cout << final_matrix[i];
		fileOutput << final_matrix[i];
		if ((i+1) % BCol == 0 && i > 0) {
			cout << endl;
			fileOutput << endl;
		} 
		else {
			cout << " ";
			fileOutput << " ";
		}
	}

	fileInput.close();
	fileOutput.close();
	return 0;
}

int multiply(int a, int b) {
	return a * b;
}
