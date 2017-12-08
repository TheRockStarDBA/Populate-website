// Populate website.cpp : Defines the entry point for the console application.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "stdafx.h"	
#include "main.h"

using namespace std;

int main()
{
	ios::sync_with_stdio(false);
	cout << "Running\n";
	unordered_map<string, int> geneid;
	unordered_map<pair<int, int>, int> record;
	readFile("C:\\OneDrive\\projects\\Web\\BIOGRID.txt", geneid, record);
	populateDB(geneid, record);
	cout << "Done.";
	getchar();
}

void readFile(char* path, unordered_map<string, int>& geneid, unordered_map<pair<int, int>, int>& record)
{
	FILE* fp;
	char line[2048];
	fp = fopen(path, "r");
	if (!fp)
	{
		cout << "Cannot open file" << path << endl;
		return;
	}
	int genecount = 0;
	const char ht = (char)9;
	char gene1[128], gene2[128], species1[128], species2[128];
	int pg1 = 0, pg2 = 0, ps1 = 0, ps2 = 0;
	while (fgets(line, 2048, fp) != NULL)
	{
		memset(gene1, 0, sizeof(gene1));
		memset(gene2, 0, sizeof(gene2));
		memset(species1, 0, sizeof(species1));
		memset(species2, 0, sizeof(species2));
		pg1 = 0, pg2 = 0, ps1 = 0, ps2 = 0;
		if (line[0] != '#')
		{
			int i = 0;
			int TabCount = 0;
			while (line[i] && TabCount < 7)
			{
				if (line[i] == ht)
					TabCount++;
				i++;
			}
			while (line[i] && line[i] != ht && pg1 < sizeof(gene1))
			{
				gene1[pg1++] = line[i];
				i++;
			}
			i++;
			while (line[i] && line[i] != ht && pg2 < sizeof(gene2))
			{
				gene2[pg2++] = line[i];
				i++;
			}
			if (gene1[0] != '\0' && gene1[0] != '-' && gene2[0] != '\0' && gene2[0] != '-')
			{
				TabCount = 8;
				while (line[i] && TabCount < 15)
				{
					if (line[i] == ht)
						TabCount++;
					i++;
				}
				while (line[i] && line[i] != ht && ps1 < sizeof(species1))
				{
					species1[ps1++] = line[i];
					i++;
				}
				i++;
				while (line[i] && line[i] != ht && ps2 < sizeof(species2))
				{
					species2[ps2++] = line[i];
					i++;
				}
				if (!strcmp(species1, "9606") && !strcmp(species2, "9606"))
				{
					string g1(gene1);
					string g2(gene2);
					int id1, id2;
					if (geneid.find(g1) == geneid.end())
					{
						geneid[g1] = genecount;
						id1 = genecount;
						genecount++;
					}
					else
						id1 = geneid[g1];
					if (geneid.find(g2) == geneid.end())
					{
						geneid[g2] = genecount;
						id2 = genecount;
						genecount++;
					}
					else
						id2 = geneid[g2];
					auto genepair = make_pair(id1, id2);
					if (record.find(genepair) == record.end())
						record[genepair] = 1;
					else
						record[genepair]++;
					if (record[genepair] == 1)
						cout << gene1 << '\t' << gene2 << '\n';
				}
			}
		}
	}
}

void populateDB(unordered_map<string, int>& geneid, unordered_map<pair<int, int>, int>& record)
{
	HRESULT hr = S_OK;
	_ConnectionPtr pConn = NULL;
	_CommandPtr pCom;
	const _bstr_t strCon("Provider=SQLNCLI11; Data Source='(localdb)\\MSSQLLocalDB'; Initial Catalog='ReactomeDB'; Integrated Security=SSPI;");
	_RecordsetPtr pDB = NULL;
	CoInitialize(NULL);
	try {
		hr = pConn.CreateInstance((__uuidof(Connection)));
		if (FAILED(hr)) {
			cout << "Error instantiating Connection object\n";
			CoUninitialize();
			return;
		}
		hr = pConn->Open(strCon, "", "", 0);
		if (FAILED(hr)) {
			printf("Error Opening Database objectn");
			CoUninitialize();
			return;
		}
		pCom.CreateInstance(__uuidof(Command));
		pCom->ActiveConnection = pConn;
		pCom->CommandType = adCmdText;
		ctpl::thread_pool thrPool(thread::hardware_concurrency());
		for (auto& key : geneid)
			thrPool.push(insertID,key, pConn);
		for (auto& key : record)
			thrPool.push(insertRecord, key, pConn);	  
		thrPool.~thread_pool();
		pConn->Close();
	}
	catch (_com_error &ce) {
		cout << "Error:" << ce.Description() << '\n';
	}
	CoUninitialize();
}

void insertID(int tid, pair<string, int> const& gene, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = "INSERT INTO GeneID VALUES(" + _bstr_t(gene.second) + ", '" + _bstr_t(gene.first.c_str()) + "');";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
	cout << cmdStr << '\n';
}

void insertRecord(int tid, pair<pair<int, int>, int> const& item, _ConnectionPtr& pConn)
{
	_bstr_t cmdStr = "INSERT INTO Reactomes VALUES(" + _bstr_t(item.first.first) + ", " + _bstr_t(item.first.second) + ", " + _bstr_t(item.second) + ");";
	pConn->Execute(cmdStr, NULL, adExecuteNoRecords);
}

