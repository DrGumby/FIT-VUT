#include <string> 
#include <iostream> 
#include <unordered_map> 
int main() 
{ 
	using namespace std; 
	unordered_map<string,int> m; // asociativní pole 
	string word; 

	while (cin >> word) // čtení slova 
		m[word]++; // počítání výskytů slova 

	for (auto &mi: m) // pro všechny prvky kontejneru m 
		cout << mi.first << "\t" << mi.second << "\n"; 
}
