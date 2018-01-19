#include"ai.h"

#include <iostream>
#include <io.h>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <stdio.h>
#include <conio.h>

using namespace std;
using namespace cv;

areaInfo area[MAX_SIZE_AREA];

void aiInit(int classCount, vector<pair<Mat, int> > &train_set, vector<pair<vector<double>, int> > &preprocessTrain)
{
	preprocessTrain.clear();
	train_set.clear();

	for (int i = 0; i < MAX_SIZE_AREA; i++)
	{
		area[i].isWall = FALSE;
		area[i].pictureLen = 0;
		area[i].picture_png = (char*)calloc(MAX_SIZE_PICTURE, sizeof(char));
	}

	for (int i = 0; i < classCount; i++)
	{
		string path = to_string(i);
		if (!input(train_set, path, i))
			cout << "error!\n";
	}

	for (int i = 0; i < train_set.size(); i++)
	{
		auto tmp = train_set[i];
		preprocessTrain.push_back({ featureDescript(tmp.first), tmp.second });
	}
}

void sendResult(SOCKET sock, int playerNextPosition, int applePosition, int bombPosition)
{
	char sendString[12];

	*((int*)sendString) = playerNextPosition;
	*((int*)sendString + 1) = applePosition;
	*((int*)sendString + 2) = bombPosition;

	send(sock, sendString, 12, 0);
}

// 0 is wall, 1 is fine

int recvPicture(SOCKET sock, int flags, int idx)
{
	int recvLen, tmpLen, isFine;
	int iter = 0;
	char recvTmpChar[4];

	recv(sock, recvTmpChar, 4, flags);

	isFine = *((int*)recvTmpChar);

	recv(sock, recvTmpChar, 4, flags);

	recvLen = *((int*)recvTmpChar);

	area[idx].isWall = !isFine;
	area[idx].pictureLen = recvLen;

	if (!isFine)
		return -1;

	while (1)
	{
		if (iter >= recvLen)
			break;

		tmpLen = recv(sock, (area[idx].picture_png) + iter, recvLen - iter, flags);

		if (tmpLen == -1)
			continue;

		iter = iter + tmpLen;
	}

	(area[idx].picture_png)[recvLen] = 0;

	vector<uchar> trans;

	trans.assign(area[idx].picture_png, area[idx].picture_png + area[idx].pictureLen);
	area[idx].picture_rgba = imdecode(InputArray(trans), CV_8SC4);

	trans.clear();

	return recvLen;
}

void recvResult(SOCKET sock)
{
	for (int i = 0; i < MAX_SIZE_AREA; i++)
	{
		recvPicture(sock, 0, i);
		printf("%d / %d\n", i, MAX_SIZE_AREA);
	}
}

void AI(SOCKET sock)
{
	int playerNextPosition;
	int bombPosition;
	int applePosition;

	vector<pair<Mat, int> > train_set;
	vector<pair<vector<double>, int> > train_feature;

	aiInit(4, train_set, train_feature);

	while (1)
	{
		recvResult(sock);

		aiCode(playerNextPosition, applePosition, bombPosition, 4, train_feature);

		sendResult(sock, playerNextPosition, applePosition, bombPosition);
	}
}

int towardPoint(int idx)
{
	switch (idx)
	{
	case 0:
	case 6:
	case 7:
		return 0;
	case 1:
	case 8:
	case 9:
		return 1;
	case 2:
	case 11:
	case 10:
		return 2;
	case 3:
	case 12:
	case 13:
		return 3;
	case 4:
	case 14:
	case 15:
		return 4;
	case 5:
	case 16:
	case 17:
		return 5;

	}
}

int moveCharacter(vector<int> classNumber)
{
	int idx = 0;
	/*-----don't touch-----*/

	// sample code using idx and classNumber on idx
	for (auto tmp : classNumber)
	{
		if (tmp == 1)
		{
			towardPoint(idx);
			break;
		}

		idx++;
	}

	/*-----don't touch-----*/
	return idx;
}

void aiCode(int &playerNextPosition, int &applePosition, int &bombPosition, int nb_class, vector<pair<vector<double>, int> > train_feature)
{
	vector<int> result = predict(train_feature, nb_class);

	moveCharacter(result);

	applePosition = (rand() * 2) % 8;
	bombPosition = rand() % 8;
}

vector<double> featureDescript(Mat& m) {
	vector<double> ret;

	/*-----don't touch-----*/

	// example code
	Vec4b tmp = m.at<Vec4b>(m.rows / 2, m.cols / 2);

	ret.push_back((double)tmp[2]);
	ret.push_back((double)tmp[1]);
	ret.push_back((double)tmp[0]);


	/*-----don't touch-----*/

	return ret;
}

int classify(Mat example, vector<pair<vector<double>, int> > &training, int nb_class) {
	const int k = 10;
	int predict = -1;
	/*-----don't touch-----*/



	/*-----don't touch-----*/
	return predict;
}

vector<int> predict(vector<pair<vector<double>, int> > model, int nb_class) {
	vector<int> ret;

	for (int i = 0; i < MAX_SIZE_AREA; i++) {
		if (area[i].isWall)
			ret.push_back(-1);
		else
		{
			int pd_res = classify(area[i].picture_rgba, model, nb_class);
			ret.push_back(pd_res);
		}
	}

	return ret;
}

float model_evaluate(vector<pair<Mat, int> > training, int nb_class) {
	float error = 0.0;
	const int k = 11;
	vector<vector<int> > k_fold(k + 1);
	vector<bool> check(training.size(), false);
	int sz = training.size() / k;

	for (int i = 0; i < k; i++) {
		for (int j = 0; j < sz; j++) {
			int next = rand() % training.size();

			if (check[next]) {
				j--;
				continue;
			}
			k_fold[i].push_back(next);
			check[next] = true;
		}
	}

	for (int i = 0; i < k; i++) {
		vector<pair<Mat, int> > test;
		vector<pair<vector<double>, int> > train;
		for (int j = 0; j < training.size(); j++) {
			bool check = false;
			for (auto next : k_fold[i])
				if (next == j) {
					check = true;
					break;
				}

			if (check)
				test.push_back(training[j]);
			else
				train.push_back({ featureDescript(training[j].first), training[j].second });
		}

		int result = 0;
		for (auto here : test) {
			int res = classify(here.first, train, nb_class);
			if (here.second != res)
				result++;
		}

		error += (float)result / sz;
	}

	return (float)error / k;
}

bool input(vector<pair<Mat, int> > &list, string folder, int cs) {
	HANDLE hFind;
	WIN32_FIND_DATA FindData;
	char path[255];

	string make_path = ".\\" + folder + "\\*.*";
	string realPath = ".\\" + folder + "\\";

	strcpy(path, make_path.c_str());

	hFind = FindFirstFile((LPCSTR)path, &FindData);

	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do
	{
		if (!strcmp(FindData.cFileName, ".") || !strcmp(FindData.cFileName, ".."))
			continue;

		string name(FindData.cFileName);
		Mat file = imread(realPath + name, -1);

		list.push_back({ file, cs });

	} while (FindNextFile(hFind, &FindData));

	FindClose(hFind);

	return true;
}