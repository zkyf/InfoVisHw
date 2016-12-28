#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
using namespace std;

#pragma region TypeDefinition
struct Entry
{
	string date;
	string time;
	string sip;
	string dip;
	string proto;
	int port;
	int uplen;
	int dlen;

	friend istream& operator >> (istream& stream, Entry& entry);
	friend ostream& operator << (ostream& stream, Entry& entry);
};

istream& operator >> (istream& stream, Entry& entry)
{
	if (stream.eof()) return stream;
	stream >> entry.date >> entry.time >> entry.sip >> entry.dip >> entry.port >> entry.proto >> entry.uplen >> entry.dlen;
	return stream;
}

ostream& operator << (ostream& stream, Entry& entry)
{
	stream << " " << entry.date << " " << entry.time << " " << entry.sip << " " << entry.dip << " " << entry.port << " " << entry.proto << " " << entry.uplen << " " << entry.dlen;
	return stream;
}

struct Node
{
	string ip;
	int num;
	int totalULen;
	int totalDLen;
	vector<int> upLinkList;
	vector<int> downLinkList;

	void AddLink(Entry entry, int entryNum)
	{
		if (entry.sip == ip)
		{
			totalULen += entry.uplen;
			totalDLen += entry.dlen;
			upLinkList.push_back(entryNum);
		}
		if (entry.dip == ip)
		{
			totalULen += entry.dlen;
			totalDLen += entry.dlen;
			downLinkList.push_back(entryNum);
		}
	}
};

typedef vector<Entry> EntryTable;

struct NodeTable
{
	map<string, int> numberTable;
	vector<Node> nodeList;
	int nodeCount = 0;
	int entryCount = 0;

	void AddEntry(Entry entry)
	{
		if (numberTable.find(entry.sip) == numberTable.end())
		{
			numberTable.insert(pair<string, int>(entry.sip, nodeCount));
			Node newNode;
			newNode.ip = entry.sip;
			newNode.num = nodeCount;
			newNode.totalULen = entry.uplen;
			newNode.totalDLen = entry.dlen;
			newNode.upLinkList.push_back(entryCount);
			nodeList.push_back(newNode);
			nodeCount++;
		}
		else
		{
			map<string, int>::const_iterator result = numberTable.find(entry.sip);
			int nodeNum = result->second;
			nodeList[nodeNum].totalULen += entry.uplen;
			nodeList[nodeNum].totalDLen += entry.dlen;
			nodeList[nodeNum].upLinkList.push_back(entryCount);
		}

		if (numberTable.find(entry.dip) == numberTable.end())
		{
			numberTable.insert(pair<string, int>(entry.dip, nodeCount));
			Node newNode;
			newNode.ip = entry.dip;
			newNode.num = nodeCount;
			newNode.totalULen = entry.dlen;
			newNode.totalDLen = entry.uplen;
			newNode.downLinkList.push_back(entryCount);
			nodeList.push_back(newNode);
			nodeCount++;
		}
		else
		{
			map<string, int>::const_iterator result = numberTable.find(entry.dip);
			int nodeNum = result->second;
			nodeList[nodeNum].totalULen += entry.dlen;
			nodeList[nodeNum].totalDLen += entry.uplen;
			nodeList[nodeNum].downLinkList.push_back(entryCount);
		}
		entryCount++;
	}
	void PrintAllInJson(ostream& stream)
	{
		stream << "{" << endl;
		stream << "  \"nodeCount\" : " << nodeCount << endl;
		stream << "  \"entryCount\" : " << entryCount << endl;
		stream << "  \"numberTable\" : " << endl;
		stream << "  [" << endl;
		for (int i = 0; i < nodeCount; i++)
		{
			stream << "    " << "\"" << nodeList[i].ip << "\"";
			if (i != nodeCount - 1) stream << ",";
			stream << endl;
		}
		stream << "  ]" << endl;
		stream << "  \"nodeList\" : " << endl;
		stream << "  [" << endl;
		for (int i = 0; i < nodeCount; i++)
		{
			stream << "    {" << endl;
			stream << "      \"ip\" : " << nodeList[i].ip << "," << endl;
			stream << "      \"num\" : " << nodeList[i].num << "," << endl;
			stream << "      \"totalULen\" : " << nodeList[i].totalULen << "," << endl;
			stream << "      \"totalDLen\" : " << nodeList[i].totalDLen << "," << endl;
			stream << "      \"upLinkList\" : " << endl;
			stream << "      [" << endl;
			for (int j = 0; j < nodeList[i].upLinkList.size(); j++)
			{
				stream << nodeList[i].upLinkList[j];
				if (j != nodeList[i].upLinkList.size() - 1) stream << ", ";
			}
			stream << endl;
			stream << "      ]" << endl;
			stream << "      \"downLinkList\" : " << endl;
			stream << "      [" << endl;
			for (int j = 0; j < nodeList[i].downLinkList.size(); j++)
			{
				stream << nodeList[i].downLinkList[j];
				if (j != nodeList[i].downLinkList.size() - 1) stream << ", ";
			}
			stream << endl;
			stream << "      ]" << endl;
			stream << "    }";
			if (i != nodeCount - 1) stream << ", ";
			stream << endl;
		}
		stream << "  ]" << endl;
		stream << "}" << endl;
	}

	void PrintAllInJson(string fileName)
	{
		fstream stream;
		stream.open(fileName, ios::out);
		if (stream.fail()) return;
		PrintAllInJson(stream);
		stream.close();
	}

	int GetIpNum(string ip)
	{
		map<string, int>::const_iterator result = numberTable.find(ip);
		if (result == numberTable.end())
		{
			return -1;
		}
		else
		{
			return result->second;
		}
	}
};
#pragma endregion

NodeTable nodeTable;

int ReadFile(int argc, char** argv)
{
	int ipCount = 0;
	int localIpCount = 0;
	fstream entryFile;
	entryFile.open("entries.json", ios::out);
	entryFile << "{[" << endl;
	FILE *cFile = NULL;
	for (int i = 1; i < argc; i++)
	{
		// using C style input;
		if (cFile) fclose(cFile);
		cFile = fopen(argv[i], "r");
		if (!cFile)
		{
			cout << "Failed to open file: " << argv[i] << endl;
			continue;
		}
		cout << "Reading file " << argv[i] << endl;
		int lineCount = 0;
		while (true)
		{
			if (feof(cFile)) break;
			Entry entry;
			char date[20], time[20], sip[20], dip[20], proto[20];
			fscanf(cFile, "%s%s%s%s%d%s%d%d", date, time, sip, dip, &entry.port, proto, &entry.uplen, &entry.dlen);
			entry.date = string(date);
			entry.time = string(time);
			entry.sip = string(sip);
			entry.dip = string(dip);
			entry.proto = string(proto);
			lineCount++;
			nodeTable.AddEntry(entry);
			entryFile << " {" << endl;
			entryFile << "  \"date\" : " << entry.date << "," << endl;
			entryFile << "  \"time\" : " << entry.time << "," << endl;
			entryFile << "  \"sipNum\" : " << nodeTable.GetIpNum(entry.sip) << "," << endl;
			entryFile << "  \"dipNum\" : " << nodeTable.GetIpNum(entry.dip) << "," << endl;
			entryFile << "  \"proto\" : " << entry.proto << "," << endl;
			entryFile << "  \"port\" : " << entry.port << "," << endl;
			entryFile << "  \"ulen\" : " << entry.uplen << "," << endl;
			entryFile << "  \"dlen\" : " << entry.dlen << endl;
			entryFile << " }," << endl;
			if (lineCount%10000 == 0)
			{
				//cout << "#" << lineCount << " " << entry << endl;
			}
		}
		cout << nodeTable.nodeCount - ipCount << " IPs" << endl;
		ipCount = nodeTable.nodeCount;
		localIpCount = 0;
	}
	entryFile << "\"eof\" : {} ]}" << endl;
	entryFile.close();
	cout << "In total " << nodeTable.nodeCount << "IPs found." << endl;
	nodeTable.PrintAllInJson("NodeData.json");
	return ipCount;
}

int main(int argc, char** argv)
{
	int ipCount = ReadFile(argc, argv);
	system("pause");
}