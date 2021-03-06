// ZhinkinaA_ind_task.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

#include <stdio.h>
#include "omp.h"
#include "assert.h"
#include <iostream>

using namespace std;

const int n = 512;
double *A, *B, **C, **T;
int blockSize = 8;
double **AA, **BB;

int sumProgression(int N) {
	return ((N + 1) * N) / 2;
}

void initRand() {
	A = new double[sumProgression(n)];
	B = new double[sumProgression(n)];
	C = new double *[n];
	T = new double *[n];
	for (int i = 0; i < sumProgression(n); ++i) {
		A[i] = rand() % 100;
		B[i] = rand() % 100;
	}

	for (int i = 0; i < n; ++i) {
		C[i] = new double[n];
		T[i] = new double[n];
	}

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j) {
			C[i][j] = 0;
			T[i][j] = 0;
		}
}

int Aij(int ii, int jj) {

	int k = 0; 
	int *index = new int[n]; //массив индексов
	for (int i = 0; i < n; ++i)
		index[i] = sumProgression(i);
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j)
			if (i > j) { //нижний треугольник
				index[j] += k;
				if (i == ii && j == jj) {
					return A[index[j]];
				}
			}
			else {// верхний треугольник
				if (i == 0) {
					if (i == ii && j == jj) {
						return A[index[j]];
					}
				}
				else {
					index[j] += 1;
					if (i == ii && j == jj) {
						return A[index[j]];
					}
				}
			}
		k = i + 1;
	}
}

int Bij(int ii, int jj) {

	int k = 0;
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j)
			if (i <= j) { // верхний треугольн
				if (i == ii && j == jj)
					return B[k];
				++k;
			}
			else // нижний треугольник
				if (i == ii && j == jj)
					return 0;
	}
}

void print() {
	cout << "A:" << endl;
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			printf("%4d", Aij(i, j));
		}
		printf("\n");
	}

	cout << "B:" << endl;
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			printf("%4d", Bij(i, j));
		}
		printf("\n");
	}

	cout << endl << "C:" << endl;
	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j)
			cout << C[i][j] << " ";
		printf("\n");
	}
}

void initMatr() {
	cout << "Initialization matrix A ..." << endl;
	AA = new double *[n];
	for (int i = 0; i < n; ++i)
		AA[i] = new double[n];

	cout << "Initialization matrix B ..." << endl;
	BB = new double *[n];
	for (int i = 0; i < n; ++i)
		BB[i] = new double[n];

	for (int i = 0; i < n; ++i) {
		for (int j = 0; j < n; ++j) {
			AA[i][j] = Aij(i, j);
			BB[i][j] = Bij(i, j);
		}
	}	
}

void multSimpleMatr(int blockSize) {
	int N = n / blockSize;
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j)
			//block - движение по блочным строке/столбцу внутри блока
			for (int block = 0; block < N; ++block)
				for (int ii = i * blockSize; ii < (i + 1)*blockSize; ++ii)
					for (int jj = j * blockSize; jj < (j + 1)*blockSize; ++jj)
						//k - движение по строке/столбцу внутри блока
						for (int k = block * blockSize; k < (block + 1)*blockSize; ++k) {
							if (k <= jj) {
								C[ii][jj] += AA[ii][k] * BB[k][jj];
							}
							else {
								C[ii][jj] += 0;
							}
						}
	}

}

void multThreadMatr(int blockSize) {
	int N = n / blockSize;
	omp_set_num_threads(N);
#pragma omp parallel 
	{
		for (int j = 0; j < N; ++j)
			//block - движение по блочным строке/столбцу внутри блока
			for (int block = 0; block < N; ++block)
				for (int ii = omp_get_thread_num()*blockSize; ii < (omp_get_thread_num() + 1)*blockSize; ++ii)
					for (int jj = j * blockSize; jj < (j + 1)*blockSize; ++jj)
						//k - движение по строке/столбцу внутри блока
						/*for (int k = block * blockSize; k < (block + 1)*blockSize; ++k) {
							C[ii][jj] += AA[ii][k] * BB[k][jj];
						}*/
						for (int k = block * blockSize; k < (block + 1)*blockSize; ++k) {
							if (k <= jj) {
								C[ii][jj] += AA[ii][k] * BB[k][jj];
							}
							else {
								C[ii][jj] += 0;
							}
						}
	}

}

void multThreadMatrBlock(int blockSize) {
	int N = n / blockSize;
	omp_set_num_threads(N);
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			for (int block = 0; block < N; ++block) {
#pragma omp parallel
				{
#pragma omp for
					for (int ii = i * blockSize; ii < (i + 1)*blockSize; ++ii) {
						for (int jj = j * blockSize; jj < (j + 1)*blockSize; ++jj){
							//for (int k = block * blockSize; k < (block + 1)*blockSize; ++k){
								//C[ii][jj] += AA[ii][k] * BB[k][jj];
							//}
							for (int k = block * blockSize; k < (block + 1)*blockSize; ++k) {
								if (k <= jj) {
									C[ii][jj] += AA[ii][k] * BB[k][jj];
								}
								else {
									C[ii][jj] += 0;
								}
							}
						}
					}
				}
			}
		}
	}
}

double start_time, end_time;

int main()
{

	if (blockSize > n || n % blockSize != 0 || n % 2 != 0)
		cout << "Ошибка! Размер блока больше размера матрицы или невозможно разделить матрицу на блоки!" << endl;
	else {


		initRand();
		initMatr();
		start_time = omp_get_wtime();
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				for (int k = 0; k < n; k++) {
					T[i][j] += AA[i][k] * BB[k][j];
				}
			}
		}
		end_time = omp_get_wtime();
		cout << "\nTime: " << end_time - start_time << endl;

		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				C[i][j] = 0;

		start_time = omp_get_wtime();
		multSimpleMatr(blockSize);
		end_time = omp_get_wtime();
		//print();
		//Проверка
		bool flag = true;
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++) {
				if (T[i][j] != C[i][j]) {
					flag = false;
				}
			}
		printf("\n");
		if (flag) printf("TEST: TRUE");
		else printf("TEST: FALSE");
		printf("\n");
		cout << "\nTime without threads: " << end_time - start_time << endl;
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				C[i][j] = 0;

		start_time = omp_get_wtime();
		multThreadMatr(blockSize);
		end_time = omp_get_wtime();
		//print();		
		//Проверка
		flag = true;
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++) {
				if (T[i][j] != C[i][j]) {
					flag = false;
				}
			}
		printf("\n");
		if (flag) printf("TEST: TRUE");
		else printf("TEST: FALSE");
		printf("\n");
		cout << "\nTime with threads: " << end_time - start_time << endl;

		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j)
				C[i][j] = 0;

		start_time = omp_get_wtime();
		multThreadMatrBlock(blockSize);
		end_time = omp_get_wtime();
		//print();
		//Проверка
		flag = true;
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++) {
				if (T[i][j] != C[i][j]) {
					flag = false;
				}
			}
		printf("\n");
		if (flag) printf("TEST: TRUE");
		else printf("TEST: FALSE");
		printf("\n");
		cout << "\nTime with threads (blocks): " << end_time - start_time << endl;
	}

	delete[] A;
	delete[] B;
	for (int i = 0; i < n; i++) {
		delete[] C[i];
	}
	delete[] C;
	return 0;
}